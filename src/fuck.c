
#if defined(_M_X64) || !defined(WIN32)
#error "Can be comiled only to 32-bit Windows program!"
#endif

#include "fuck.h"
#include <stdlib.h>
#include <stdarg.h>
#include <Windows.h>


static HANDLE hExeHeap = NULL;

void InitExeHeap(){
    hExeHeap=HeapCreate(HEAP_CREATE_ENABLE_EXECUTE,0,0);
}

typedef struct tagGROWING{
    byte* buf;
    int size;
    int cap;
}GROWING;
static void InitGrowing(GROWING* p){
    const int initCap = 1;
    if(hExeHeap==NULL)InitExeHeap();
    p->buf = (byte*)HeapAlloc(hExeHeap,0,initCap);
    p->size = 0;
    p->cap = initCap;
}
static void Grow(GROWING* p,int size,...){
    va_list vars;
    va_start(vars,size);
    if(p->size+size>p->cap){
        p->cap = (p->size+size)*2;
        p->buf = (byte*)HeapReAlloc(hExeHeap,0,p->buf,p->cap);
    }
    while(size-->0){
        p->buf[p->size++] = (byte)(va_arg(vars,int));
    }
    va_end(vars);
}

typedef enum tagOperator{
    oPLUS,oMINUS,oFORWARD,oBACKWARD,oINPUT,oOUTPUT,oSTART,oEND
}Operator;
static void ReadNextCode(const char* source,Operator* op,int* count){
    char first;
    *count = 0;
    switch(first=*source){
    case '+':
        *op = oPLUS; break;
    case '-':
        *op = oMINUS; break;
    case '>':
        *op = oFORWARD; break;
    case '<':
        *op = oBACKWARD; break;
    case '.':
        *op = oOUTPUT; break;
    case ',':
        *op = oINPUT; break;
    case '[':
        *op = oSTART; break;
    case ']':
        *op = oEND; break;
    default:
        return;
    }
    while(*(source++)==first)++*count;
}

Fucker CompileFucker(const char* source,CompileFuckerResult* result){
    GROWING dest;
    InitGrowing(&dest);

    int loopStack[100],*loopStackHead = loopStack;

#define PARAM_UNIT 4

    //Function Header
    Grow(&dest,1,0x55);//push ebp
    Grow(&dest,2,0x8B,0xEC);//mov ebp,esp
    Grow(&dest,3,0x8B,0x45,PARAM_UNIT*6);//mov eax,dword ptr [workSpace=*(ebp+18h)] 

    Operator op;
    int opCount;
    int i;
    while(1){
        ReadNextCode(source,&op,&opCount);
        if(opCount==0){
            if(result)result->endingChar = *source;
            break;
        }
        source += opCount;

        switch(op){
        case oPLUS:case oMINUS:
            opCount %= 256;
            Grow(&dest,3,0x0F,0xB6,0x08);//movzx ecx,byte ptr [eax]
            Grow(&dest,3,0x83,op==oPLUS?0xC1:0xE9,opCount);//add|sub ecx,opCount
            Grow(&dest,2,0x88,0x08);//mov byte ptr [eax],cl  
            break;
        case oFORWARD:case oBACKWARD:
        {
            byte* r = (byte*)&opCount;
            Grow(&dest,5,op==oFORWARD?0x05:0x2D,
                r[0],r[1],r[2],r[3]);//add|sub eax,opCount
            break;
        }
        case oINPUT:
            Grow(&dest,1,0x50);//push eax
            for(i = 0; i<opCount; ++i){
                Grow(&dest,3,0x8B,0x45,PARAM_UNIT*3);//mov eax,dword ptr [readerParam]
                Grow(&dest,1,0x50);//push eax
                Grow(&dest,3,0xFF,0x55,PARAM_UNIT*2);//call dword ptr [reader]
                Grow(&dest,3,0x83,0xC4,PARAM_UNIT);//add esp,4
            }
            Grow(&dest,1,0x59);//pop ecx
            Grow(&dest,2,0x88,0x01);//mov byte ptr [ecx],al
            Grow(&dest,2,0x8B,0xC1);//mov eax,ecx;
     
            break;
        case oOUTPUT:
            for(i = 0; i<opCount; ++i){
                Grow(&dest,3,0x0F,0xB6,0x08);//movzx ecx,byte ptr[eax]
                Grow(&dest,1,0x50);//push eax
                Grow(&dest,1,0x51);//push ecx
                Grow(&dest,3,0x8B,0x55,PARAM_UNIT*5);//mov edx,dword ptr[writerParam]
                Grow(&dest,1,0x52);//push edx
                Grow(&dest,3,0xFF,0x55,PARAM_UNIT*4);//call dword ptr[writer]
                Grow(&dest,3,0x83,0xC4,PARAM_UNIT*2);//add esp,8
                Grow(&dest,1,0x58);//pop eax
            }
            break;
        case oSTART:
            for(i = 0; i<opCount; ++i){
                Grow(&dest,3,0x0F,0xB6,0x08);//movzx ecx,byte ptr[eax]
                Grow(&dest,2,0x85,0xC9);//test ecx,ecx
                Grow(&dest,6,0x0F,0x84,0,0,0,0);//je +??
                *(loopStackHead++) = dest.size;
            }
            break;
        case oEND:
        {
            for(i = 0; i<opCount; ++i){
                int bodyBegin = *(--loopStackHead);
                int bodySize = dest.size-bodyBegin;
                int foreOff = bodySize+5;
                byte* foreOffR = (byte*)&foreOff;
                dest.buf[bodyBegin-4] = foreOffR[0];
                dest.buf[bodyBegin-3] = foreOffR[1];
                dest.buf[bodyBegin-2] = foreOffR[2];
                dest.buf[bodyBegin-1] = foreOffR[3];//je +?? -> je +(bodySize+5)
                int backOff = -(bodySize+16);
                byte* backOffR = (byte*)&backOff;
                Grow(&dest,5,0xE9,
                    backOffR[0],
                    backOffR[1],
                    backOffR[2],
                    backOffR[3]);//jmp -(bodySize+16)
            }
        }
            break;
        }
    }

    //Function Footer
    Grow(&dest,2,0x8B,0xE5);//mov esp,ebp
    Grow(&dest,1,0x5D);//pop ebp
    Grow(&dest,1,0xC3);//ret

    if(result){
        result->size = dest.size;
    }


    return (Fucker)(dest.buf);
}
void FreeFucker(Fucker fucker){
    HeapFree(hExeHeap,0,fucker);
}
