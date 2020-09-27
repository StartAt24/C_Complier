#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

int token;              // current token
char *src, *old_src;    // pointer to source code string;
int poolsize;           // default size of text/data/stack
int line;               // line number

void next(){
    token = *src++;
    return;
}

void expression(int level){
    // to do
}

// 语法分析的入口,分析整个C语言程序
void program(){
    next();
    while( token > 0 ){
        printf("token is: %c\n", token);
        next();
    }
}

int eval() {
    return 0;    
}

int main(int argc, char** argv){
    int i, fd;

    // get correct parameter.
    argc--;
    argv++;

    poolsize = 256*1024; //arbitrary size
    line = 1;

    if( (fd = open(*argv, 0)) < 0){
        printf("counld not open(%s)\n", *argv);
        return -1;
    }

    if(!(src = old_src = malloc(poolsize))){
        printf("counld not malloc(%d) for source area\n", poolsize);
        return -1;
    }

    // read source file
    if ((i = read(fd, src, poolsize-1)) <= 0){
        printf("read() returned %d\n", i);
        return -1;
    }
    src[i] = 0; //add EOF character
    close(fd);

    program();
    return eval();
}