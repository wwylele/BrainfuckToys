#include <array>
#include <iostream>

struct Empty {};

template <typename T, T h, T... t>
struct Array {
    static constexpr T head = h;
    using tail = Array<T, t...>;
    static constexpr unsigned length = 1 + tail::length;
};

template <typename T, T h>
struct Array<T, h> {
    static constexpr T head = h;
    using tail = Empty;
    static constexpr unsigned length = 1;
};

template <typename ArrayT, unsigned skip_length>
struct Skip {
    using result = typename Skip<typename ArrayT::tail, skip_length - 1>::result;
};

template <typename ArrayT>
struct Skip<ArrayT, 0> {
    using result = ArrayT;
};

template <typename T, typename ArrayProvider, unsigned len, T... element>
struct ArrayBuilder {
    using result = typename ArrayBuilder<T, ArrayProvider, len - 1, ArrayProvider::data()[len - 1],
                                         element...>::result;
};

template <typename T, typename ArrayProvider, T... element>
struct ArrayBuilder<T, ArrayProvider, 0, element...> {
    using result = Array<T, element...>;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

constexpr unsigned StringLength(char const* str, unsigned count = 0) {
    return ('\0' == str[0]) ? count : StringLength(str + 1, count + 1);
}

template <typename StringProvider>
using String =
    typename ArrayBuilder<char, StringProvider, StringLength(StringProvider::data())>::result;

#define DEFINE_STRING(name__, content__)                                                           \
    struct name__##_TAG_ {                                                                         \
        static constexpr const char* data() {                                                      \
            return content__;                                                                      \
        }                                                                                          \
    };                                                                                             \
    using name__ = String<name__##_TAG_>;

//////////////////////////////////////////////////////////////////////////////////////////////////
namespace BrainfuckDetail {

template <typename Code>
struct Block;

// generic template for charactors that are ignored
template <char head, typename CodeTail>
struct Command {
    using Next = Block<CodeTail>;
    static constexpr unsigned length = Next::length + 1;

    template <typename In, typename Out, typename WorkspacePtr>
    static void execute(In in, Out out, WorkspacePtr& ptr) {
        Next::execute(in, out, ptr);
    }
};

// template specialization for commands that are executed sequentially
#define DEFINE_BRAINFUCK_COMMAND(command__, operation__)                                           \
    template <typename CodeTail>                                                                   \
    struct Command<command__, CodeTail> {                                                          \
        using Next = Block<CodeTail>;                                                              \
        static constexpr unsigned length = Next::length + 1;                                       \
                                                                                                   \
        template <typename In, typename Out, typename WorkspacePtr>                                \
        static void execute(In in, Out out, WorkspacePtr& ptr) {                                   \
            operation__;                                                                           \
            Next::execute(in, out, ptr);                                                           \
        }                                                                                          \
    };

DEFINE_BRAINFUCK_COMMAND('>', ++ptr)
DEFINE_BRAINFUCK_COMMAND('<', --ptr)
DEFINE_BRAINFUCK_COMMAND('+', ++*ptr)
DEFINE_BRAINFUCK_COMMAND('-', --*ptr)
DEFINE_BRAINFUCK_COMMAND('.', out(*ptr))
DEFINE_BRAINFUCK_COMMAND(',', *ptr = in())

#undef DEFINE_BRAINFUCK_COMMAND

// template specialization for block beginning command '['
template <typename CodeTail>
struct Command<'[', CodeTail> {
    using Inner = Block<CodeTail>;
    static_assert(Inner::length < CodeTail::length, "parentheses mismatch!");
    using Next = Block<typename Skip<CodeTail, Inner::length + 1>::result>;
    static constexpr unsigned length = Inner::length + Next::length + 2;

    template <typename In, typename Out, typename WorkspacePtr>
    static void execute(In in, Out out, WorkspacePtr& ptr) {
        while (*ptr)
            Inner::execute(in, out, ptr);

        Next::execute(in, out, ptr);
    }
};

// template specialization for block end command ']'
template <typename CodeTail>
struct Command<']', CodeTail> {
    static constexpr unsigned length = 0;

    template <typename In, typename Out, typename WorkspacePtr>
    static void execute(In in, Out out, WorkspacePtr& ptr) {}
};

// generic template for non-Empty block
template <typename Code>
struct Block {
    using Current = Command<Code::head, typename Code::tail>;
    static constexpr unsigned length = Current::length;

    template <typename In, typename Out, typename WorkspacePtr>
    static void execute(In in, Out out, WorkspacePtr& ptr) {
        Current::execute(in, out, ptr);
    }
};

// template specialization for Empty block
template <>
struct Block<Empty> {
    static constexpr unsigned length = 0;

    template <typename In, typename Out, typename WorkspacePtr>
    static void execute(In in, Out out, WorkspacePtr& ptr) {}
};

} // namespace BrainfuckDetail

// final template
/*
Executes a brainfuck program.
(template) parameters:
    Code
        the code of the brainfuck program. Should be a String<StringProvider> type.
    In in
        the input function, in the form of (void)->InputType.
    Out out
        the output function, in the form of (OutputType)->void.
    WorkspacePtr ptr
        the workspace pointer object. This object should support increment(++), decrement(--) and
        dereference(*) operators. The dereference operator should return a reference to CellType.

notes:
    among the types mentioned above, InputType should be convertable to CellType, and CellType
    should be convertable to OutputType. CellType should support increment(++) and decrement(--)
    operator.
*/
template <typename Code, typename In, typename Out, typename WorkspacePtr>
void Brainfuck(In in, Out out, WorkspacePtr& ptr) {
    using Block = BrainfuckDetail::Block<Code>;
    static_assert(Block::length == Code::length, "parentheses mismatch!");
    Block::execute(in, out, ptr);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

template <typename CellType, unsigned size>
class Workspace {
    unsigned pos = 0;
    std::array<CellType, size> memory{};

public:
    void operator++() {
        ++pos;
    }

    void operator--() {
        --pos;
    }

    CellType& operator*() {
        return memory[pos];
    }
};

//////////////////////////////////////////////////////////////////////////////////////////////////

int main() {
    using byte = unsigned char;
    Workspace<byte, 100> workspace;
    DEFINE_STRING(hello, "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>."
                         "<-.<.+++.------.--------.>>+.>++.")
    Brainfuck<hello>([]() -> byte { return 0; }, [](byte b) { std::cout << (char)b; }, workspace);
    std::cout << std::endl;
}
