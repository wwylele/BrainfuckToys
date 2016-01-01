#ifndef _FUCK_COMPILER_FUCK_H_
#define _FUCK_COMPILER_FUCK_H_
typedef unsigned char byte;
typedef byte(__cdecl* ByteReader)(void* param);
typedef void(__cdecl* ByteWriter)(void* param,byte data);
typedef void(__cdecl* Fucker)(
    ByteReader reader,void* readerParam,
    ByteWriter writer,void* writerParam,
    byte* workSpace);
typedef struct tagCompileFuckerResult{
    char endingChar;
    int compiledSrcSize;
    int size;
    int wrongLoop;
}CompileFuckerResult;
Fucker CompileFucker(const char* source,CompileFuckerResult* result);
void FreeFucker(Fucker fucker);


#endif
