#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#define int long long

int token;              // current token
char *src, *old_src;    // pointer to source code string;
int poolsize;           // default size of text/data/stack
int line;               // line number

int *text,              // text segment
    *old_text,          // for dump text segment
    *stack;             // stack
char *data;             // data segment

// 寄存器
int *pc, *bp, *sp, ax, cycle; //virtual machine registers

// instructions
enum { LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,
       OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,
       OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT };

// tokens and classes (operators last and in precedence order)
enum {
    Num = 128, Fun, Sys, Glo, Loc, Id,
    Char, Else, Enum, If, Int, Return, Sizeof, While,
    Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};

struct identifier{
    int token; 
    int hash;       // 标识符的hash值
    char* name;     // 标识符本身的字符串
    int class;
    int type;
    int value;
    int Bclass;
    int Btype;
    int Bvalue;
};

int token_val;      // value of current token (mainly for number)
int *current_id,    // current parsed ID
    *symbols;       // symbol table

// fields of identifier
enum {Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize};


void next(){
    char *last_pos;
    int hash;
    while (token = *src){
        ++src;
        // parse token here

        if(token == '\n'){
            ++line;
        }else if(token == '#'){
            while(*src !=0 && *src != '\n'){
                src++;
            }
        }else if( (token >= 'a'&& token <= 'z') || (token>'A'&& token<'Z')|| (token == '_')){
            // parse identifier
            last_pos = src-1;
            hash = token;

            while((*src>'a' && *src <= 'z') || (*src>='A' && *src<='Z') || (*src>='0' && *src <='9') || (*src == '_')){
                hash = hash*147 + *src;
                src++;
            }

            // look for existing identifier, linear search
            current_id = symbols;
            while( current_id[Token]){
                if (current_id[Hash] == hash && !memcpy((char*)current_id[Name], last_pos, src-last_pos)){
                    // found one, return
                    token = current_id[Token];
                    return;
                }
                current_id = current_id + IdSize;
            }

            // store new ID
            current_id[Name] = (int)last_pos;
            current_id[Hash] = hash;
            token = current_id[Token] = Id;
            return;
        }
    }
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
    int op, *tmp;
    while(1){
        op = *pc++;

        if( op == IMM)      {ax = *pc++;}
        else if(op == LC)   {ax = *(char *)ax;}
        else if(op == LI)   {ax = *(int *)ax;}
        else if(op == SC)   {ax = *(char *)*sp++ = ax;}
        else if(op == SI)   {*(int*)*sp++ = ax;}
        else if(op == PUSH) {*--sp = ax;}
        else if(op == JMP)  {pc = (int*)*pc;}
        else if(op == JZ)   {pc = ax? pc+1:(int*)*pc;}
        else if(op == JNZ)  {pc = ax? (int*)*pc:pc +1;}
        else if(op == CALL) {*--sp = (int)(pc+1); pc=(int*)*pc;}
        // else if(op == RET) {pc = (int*)*sp++;}
        else if(op == ENT)  {*--sp = (int)bp; bp = sp; sp = sp - *pc++;}
        else if(op == ADJ)  {sp = sp + *pc++;}
        else if(op == LEV)  {sp = bp; bp = (int *)*sp++; pc = (int *)*sp++;}  // restore call frame and PC
        else if(op == LEA)  {ax = (int)(bp + *pc++);}

        // operators
        else if (op == OR)  ax = *sp++ | ax;
        else if (op == XOR) ax = *sp++ ^ ax;
        else if (op == AND) ax = *sp++ & ax;
        else if (op == EQ)  ax = *sp++ == ax;
        else if (op == NE)  ax = *sp++ != ax;
        else if (op == LT)  ax = *sp++ < ax;
        else if (op == LE)  ax = *sp++ <= ax;
        else if (op == GT)  ax = *sp++ >  ax;
        else if (op == GE)  ax = *sp++ >= ax;
        else if (op == SHL) ax = *sp++ << ax;
        else if (op == SHR) ax = *sp++ >> ax;
        else if (op == ADD) ax = *sp++ + ax;
        else if (op == SUB) ax = *sp++ - ax;
        else if (op == MUL) ax = *sp++ * ax;
        else if (op == DIV) ax = *sp++ / ax;
        else if (op == MOD) ax = *sp++ % ax;


        else if (op == EXIT) { printf("exit(%d)", *sp); return *sp;}
        else if (op == OPEN) { ax = open((char *)sp[1], sp[0]); }
        else if (op == CLOS) { ax = close(*sp);}
        else if (op == READ) { ax = read(sp[2], (char *)sp[1], *sp); }
        else if (op == PRTF) { tmp = sp + pc[1]; ax = printf((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]); }
        else if (op == MALC) { ax = (int)malloc(*sp);}
        else if (op == MSET) { ax = (int)memset((char *)sp[2], sp[1], *sp);}
        else if (op == MCMP) { ax = memcmp((char *)sp[2], (char *)sp[1], *sp);}
        else{
            printf("unknown instruction: %d\n", op);
            return -1;
        }
    }
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


    // allocate memory for virtual machine
    if( !(text = old_text = malloc(poolsize))){
        printf("could not malloc (%d) for text area\n", poolsize);
        return -1;
    }

    if( !(data  = malloc(poolsize))){
        printf("could not malloc (%d) for data area\n", poolsize);
        return -1;
    }

    if( !(stack = malloc(poolsize))){
        printf("could not malloc (%d) for stack area\n", poolsize);
        return -1;
    }

    memset(text, 0, poolsize);
    memset(data, 0, poolsize);
    memset(stack, 0, poolsize);

    bp = sp = (int*)((int)stack + poolsize);
    ax = 0;

    i = 0;
    text[i++] = IMM;
    text[i++] = 10;
    text[i++] = PUSH;
    text[i++] = IMM;
    text[i++] = 20;
    text[i++] = ADD;
    text[i++] = PUSH;
    text[i++] = EXIT;
    pc = text;

    program();
    return eval();
}