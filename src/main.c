#include "fuck.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

byte ReadCharFromStd(void* desc){
    byte data = getchar();
    printf("%s:%c\n",(const char*)desc,data);
    return data;
}
byte ReadNumberFromStd(void* desc){
    printf("%s:",(const char*)desc);
    int number;
    scanf("%d",&number);
    return number;
}
void WriteCharToStd(void* desc,byte data){
    printf("%s:%c\n",(const char*)desc,data);
}
void WriteNumberToStd(void* desc,byte data){
    printf("%s:%d\n",(const char*)desc,data);
}
void WriteCharToStd_Zen(void* no_use,byte data){
    printf("%c",data);
}
int main(){
   
    byte workSpace[100];

    Fucker fuckerA = CompileFucker(
        "++++++++[>++++[>++>+++>+++>+<<<<-"
        "]>+>+>->>+[<]<-]>>.>---.+++++++..+"
        "++.>>.<-.<.+++.------.--------.>>+.>++.",0);
    puts("\nFucker A begin:");
    memset(workSpace,0,100);
    fuckerA(0,0,WriteCharToStd_Zen,0,workSpace);

    Fucker fuckerB = CompileFucker(",>,<[->+<]>.",0);
    puts("\nFucker B begins:");
    memset(workSpace,0,100);
    fuckerB(ReadNumberFromStd,"B want a number(within 0~100)",
        WriteNumberToStd,"B says the sum of them is",
        workSpace);

    //These two lines are just to clean the stdin buffer
    scanf("%*[^\n]");
    getchar();

    Fucker fuckerC = CompileFucker("+[,-.---------]",0);
    puts("\nFucker C begin:");
    puts("Please input some chars");
    memset(workSpace,0,100);
    fuckerC(ReadCharFromStd,"C gets a char",
        WriteCharToStd,"C says the previous char in ASCII is",
        workSpace);
    
    system("PAUSE");
}
