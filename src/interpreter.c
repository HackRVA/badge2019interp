#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include <plib.h>
#include "badge.h"
#include "ir.h"
#include "assets.h"
#include "buttons.h"
#include "S6B33.h"
#include "timer1_int.h"
#include "tinyalloc.h"
#include "fb.h"

static jmp_buf error_exit;

void interpreter_alloc(int textpct, int datapct, int stackpct, int symbolpct);

/* for compiling and testing under linux */
#ifdef MAINAPP

/*

   cc -m32 -I../include -o inter -DMAINAPP interpreter.c 

*/

void backlight(char b)
{
}

union IRpacket_u G_hack;

void echoUSB(char *str) {
}


void flareled(unsigned char r, unsigned char g, unsigned char b) {
}

void led(unsigned char r, unsigned char g, unsigned char b) {
}

void FbWriteLine(char *str) {
}

void FbPushBuffer() {
}

void FbMove(unsigned char x, unsigned char y) {
}
#else


#endif
/* **************** */



int debug;    // print the executed instructions

int token; // current token

/*
 the instructions need to sync with the functions in init_interpreter
*/

enum { LEA, IMM, JMP, CALL, JZ, JNZ, ENT, ADJ, LEV, LI, LC, SI, SC, PUSH,
       OR, XOR, AND, EQ, NE, LT, GT, LE, GE, SHL, SHR, ADD, SUB, MUL, DIV, MOD, 
       PRT, PRTD, PRTX, MALC, MSET, MCMP,
       FLARELED, LED, FBMOVE, FBWRITE, BACKLIGHT,
       IRRECEIVE, IRSEND, SETNOTE, GETBUTTON, GETDPAD, CONTRAST, IRSTATS,
       SETTIME, GETTIME, FBLINE, FBCLEAR, FLASHW, FLASHR,IALLOC,
       EXIT
};

/* these map to above
    src = "char else enum if int return sizeof while "
      .
      .
      .
	  "exit void main";
*/

// tokens and classes (operators last and in precedence order)
// copied from c4
enum {
  Num = 128, Fun, Sys, Glo, Loc, Id,
  Char, Else, Enum, If, Int, Return, Sizeof, While,
  Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};

// fields of identifier
enum {Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize};


// types of variable/function
enum { CHARCAST, INTCAST, PTR };

// type of declaration.
enum {Global, Local};

int *text=0, *textbase, // text segment
    *stack=0, *stackbase, *stacklow;// stack
char *data=0, *database; // data segment
int *idmain;

char *src=0;  // pointer to source code string;

int *pc, *bp, *sp, ax, cycle; // virtual machine registers

int *current_id, // current parsed ID
    *symbols=0, *symbolbase,    // symbol table
    line,        // line number of source code
    token_val;   // value of current token (mainly for number)

int basetype;    // the type of a declaration, make it global for convenience
int expr_type;   // the type of an expression

/*
   howto extend interpreter

   add INST to /enum { LEA ,IMM/
   add keyword to /"PRT,MALC/
   add keyword to /src = "char else/
   add opcode check eg: /else if (op == MCMP)/
   add function call below
*/

void contrast(unsigned char con)
{
    S6B33_send_command(CONTRAST_CONTROL1); 
    S6B33_send_command(con);
}

void IRstats()
{
    char dbuffer[9];

    decDump(IR_inpkts, dbuffer);
    echoUSB("I "); echoUSB(dbuffer); echoUSB("\r\n");

    decDump(IR_outpkts, dbuffer);
    echoUSB("O "); echoUSB(dbuffer); echoUSB("\r\n");

    decDump(pinged, dbuffer);
    echoUSB("P "); echoUSB(dbuffer); echoUSB("\r\n");
}

void IRsend(int p)
{
    union IRpacket_u pkt;

    pkt.p.command = IR_WRITE;
    pkt.p.address = IR_PING;
    pkt.p.badgeId = 0x0; 
    pkt.p.data    = 0xABCD;

    IRqueueSend(pkt);
}

unsigned int IRreceive()
{
   return (unsigned int)(pinged);
}

extern struct wallclock_t wclock;
void setTime(unsigned char hour, unsigned char min, unsigned char sec)
{
    wclock.last = _CP0_GET_COUNT();
    wclock.now = _CP0_GET_COUNT();
    wclock.delta = wclock.now - wclock.last;
    wclock.accum = wclock.delta;

    wclock.hour = hour;
    wclock.min = min;
    wclock.sec = sec;
}

static char time[9];
char *getTime()
{
   time[0] = wclock.hour / 10 + 48;
   time[1] = wclock.hour % 10 + 48;
   time[2] = ':';
   time[3] = wclock.min / 10 + 48;
   time[4] = wclock.min % 10 + 48;
   time[5] = ':';
   time[6] = wclock.sec / 10 + 48;
   time[7] = wclock.sec % 10 + 48;
   time[8] = 0;

   return time;
}

/* 
  interpreter flash area
*/
const unsigned char Iflash[2048] = {0x00};

unsigned int *IflashAddr=0;
unsigned int *IflashAddrPtr=0;
unsigned int findex=0;


void IflashInit()
{
    /* 1k align */
    IflashAddr = (unsigned int *)(((unsigned int)(Iflash)+1024) & 0b11111111111111111111110000000000); // 1k flash boundary
    IflashAddrPtr = IflashAddr;

    if (*IflashAddr != (unsigned int)0xFACEBEEF) {
        findex=0;
	NVMErasePage(IflashAddr);
	NVMWriteWord(IflashAddr, (unsigned int)0xFACEBEEF);
        IflashAddrPtr++;
	findex++;
    }

}

unsigned int IflashWrite(unsigned int data) {
    if (IflashAddrPtr == 0) IflashInit();

    NVMWriteWord(IflashAddrPtr, data);
    IflashAddrPtr++;

    return (findex++);
}

unsigned int IflashRead(unsigned int index) {
    unsigned int *r_addr;

    if (IflashAddrPtr == 0) IflashInit();

    r_addr = IflashAddr + index;

    return(*r_addr);
}

// function frame
//
// 0: arg 1
// 1: arg 2
// 2: arg 3
// 3: return address
// 4: old bp pointer  <- index_of_bp
// 5: local var 1
// 6: local var 2
int index_of_bp; // index of bp pointer on stack

void next() {
    char *last_pos;
    int hash;

    //while (token = *src) {
    while (1) {
	token = *src;

        if (*src == 0) {
	   break;
	}

        ++src;

        if (token == '\n') {
            ++line;
        }
        else if (token == '#') {
            // skip macro, because we will not support it
            while (*src != 0 && *src != '\n') {
                src++;
            }
        }
        else if ((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || (token == '_')) {

            // parse identifier
            last_pos = src - 1;
            hash = token;

            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_')) {
                hash = hash * 147 + *src;
                src++;
            }

            // look for existing identifier, linear search
            current_id = symbols;
            while (current_id[Token]) {
                if (current_id[Hash] == hash && !memcmp((char *)current_id[Name], last_pos, src - last_pos)) {
                    //found one, return
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
        else if (token >= '0' && token <= '9') {
            // parse number, three kinds: dec(123) hex(0x123) oct(017)
            token_val = token - '0';
            if (token_val > 0) {
                // dec, starts with [1-9]
                while (*src >= '0' && *src <= '9') {
                    token_val = token_val*10 + *src++ - '0';
                }
            } else {
                // starts with number 0
                if (*src == 'x' || *src == 'X') {
                    //hex
                    token = *++src;
                    while ((token >= '0' && token <= '9') || (token >= 'a' && token <= 'f') || (token >= 'A' && token <= 'F')) {
                        token_val = token_val * 16 + (token & 15) + (token >= 'A' ? 9 : 0);
                        token = *++src;
                    }
                } else {
                    // oct
                    while (*src >= '0' && *src <= '7') {
                        token_val = token_val*8 + *src++ - '0';
                    }
                }
            }

            token = Num;
            return;
        }
        else if (token == '/') {
            if (*src == '/') {
                // skip comments
                while (*src != 0 && *src != '\n') {
                    ++src;
                }
            } else {
                // divide operator
                token = Div;
                return;
            }
        }
        else if (token == '"' || token == '\'') {
            // parse string literal, currently, the only supported escape
            // character is '\n', store the string literal into data.
            last_pos = data;
            while (*src != 0 && *src != token) {
                token_val = *src++;
                if (token_val == '\\') {
                    // escape character
                    token_val = *src++;
                    if (token_val == 'n') {
                        token_val = '\n';
                    }
                }

                if (token == '"') {
                    *data++ = token_val;
                }
            }

            src++;
            // if it is a single character, return Num token
            if (token == '"') {
                token_val = (int)last_pos;
            } else {
                token = Num;
            }

            return;
        }
        else if (token == '=') {
            // parse '==' and '='
            if (*src == '=') {
                src ++;
                token = Eq;
            } else {
                token = Assign;
            }
            return;
        }
        else if (token == '+') {
            // parse '+' and '++'
            if (*src == '+') {
                src ++;
                token = Inc;
            } else {
                token = Add;
            }
            return;
        }
        else if (token == '-') {
            // parse '-' and '--'
            if (*src == '-') {
                src ++;
                token = Dec;
            } else {
                token = Sub;
            }
            return;
        }
        else if (token == '!') {
            // parse '!='
            if (*src == '=') {
                src++;
                token = Ne;
            }
            return;
        }
        else if (token == '<') {
            // parse '<=', '<<' or '<'
            if (*src == '=') {
                src ++;
                token = Le;
            } else if (*src == '<') {
                src ++;
                token = Shl;
            } else {
                token = Lt;
            }
            return;
        }
        else if (token == '>') {
            // parse '>=', '>>' or '>'
            if (*src == '=') {
                src ++;
                token = Ge;
            } else if (*src == '>') {
                src ++;
                token = Shr;
            } else {
                token = Gt;
            }
            return;
        }
        else if (token == '|') {
            // parse '|' or '||'
            if (*src == '|') {
                src ++;
                token = Lor;
            } else {
                token = Or;
            }
            return;
        }
        else if (token == '&') {
            // parse '&' and '&&'
            if (*src == '&') {
                src ++;
                token = Lan;
            } else {
                token = And;
            }
            return;
        }
        else if (token == '^') {
            token = Xor;
            return;
        }
        else if (token == '%') {
            token = Mod;
            return;
        }
        else if (token == '*') {
            token = Mul;
            return;
        }
        else if (token == '[') {
            token = Brak;
            return;
        }
        else if (token == '?') {
            token = Cond;
            return;
        }
        else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' || token == ']' || token == ',' || token == ':') {
            // directly return the character as token;
            return;
        }
    }
}

void match(int tk) {
    if (token == tk) {
        next();
    } else {
        echoUSB("expected token: line, token\n");
	longjmp(error_exit, line);
    }
}


void expression(int level) {
    // expressions have various format.
    // but majorly can be divided into two parts: unit and operator
    // for example `(char) *a[10] = (int *) func(b > 0 ? 10 : 20);
    // `a[10]` is an unit while `*` is an operator.
    // `func(...)` in total is an unit.
    // so we should first parse those unit and unary operators
    // and then the binary ones
    //
    // also the expression can be in the following types:
    //
    // 1. unit_unary ::= unit | unit unary_op | unary_op unit
    // 2. expr ::= unit_unary (bin_op unit_unary ...)

    // unit_unary()
    int *id;
    int tmp;
    int *addr;
    {
        if (!token) {
            echoUSB("unexpected token EOF of expression\n");
	    longjmp(error_exit, line);
        }
        if (token == Num) {
            match(Num);

            // emit code
            *++text = IMM;
            *++text = token_val;
            expr_type = INTCAST;
        }
        else if (token == '"') {
            // continous string "abc" "abc"


            // emit code
            *++text = IMM;
            *++text = token_val;

            match('"');
            // store the rest strings
            while (token == '"') {
                match('"');
            }

            // append the end of string character '\0', all the data are default
            // to 0, so just move data one position forward.
            data = (char *)(((int)data + sizeof(int)) & (-sizeof(int)));
            expr_type = PTR;
        }
        else if (token == Sizeof) {
            // sizeof is actually an unary operator
            // now only `sizeof(int)`, `sizeof(char)` and `sizeof(*...)` are
            // supported.
            match(Sizeof);
            match('(');
            expr_type = INTCAST;

            if (token == Int) {
                match(Int);
            } else if (token == Char) {
                match(Char);
                expr_type = CHARCAST;
            }

            while (token == Mul) {
                match(Mul);
                expr_type = expr_type + PTR;
            }

            match(')');

            // emit code
            *++text = IMM;
            *++text = (expr_type == CHARCAST) ? sizeof(char) : sizeof(int);

            expr_type = INTCAST;
        }
        else if (token == Id) {
            // there are several type when occurs to Id
            // but this is unit, so it can only be
            // 1. function call
            // 2. Enum variable
            // 3. global/local variable
            match(Id);

            id = current_id;

            if (token == '(') {
                // function call
                match('(');

                // pass in arguments
                tmp = 0; // number of arguments
                while (token != ')') {
                    expression(Assign);
                    *++text = PUSH;
                    tmp ++;

                    if (token == ',') {
                        match(',');
                    }

                }
                match(')');

                // emit code
                if (id[Class] == Sys) {
                    // system functions
                    *++text = id[Value];
                }
                else if (id[Class] == Fun) {
                    // function call
                    *++text = CALL;
                    *++text = id[Value];
                }
                else {
                    echoUSB("bad function call\n");
		    longjmp(error_exit, line);
                }

                // clean the stack for arguments
                if (tmp > 0) {
                    *++text = ADJ;
                    *++text = tmp;
                }
                expr_type = id[Type];
            }
            else if (id[Class] == Num) {
                // enum variable
                *++text = IMM;
                *++text = id[Value];
                expr_type = INTCAST;
            }
            else {
                // variable
                if (id[Class] == Loc) {
                    *++text = LEA;
                    *++text = index_of_bp - id[Value];
                }
                else if (id[Class] == Glo) {
                    *++text = IMM;
                    *++text = id[Value];
                }
                else {
                    echoUSB("undefined variable\n");
		    longjmp(error_exit, line);
                }

                // emit code, default behaviour is to load the value of the
                // address which is stored in `ax`
                expr_type = id[Type];
                *++text = (expr_type == Char) ? LC : LI;
            }
        }
        else if (token == '(') {
            // cast or parenthesis
            match('(');
            if (token == Int || token == Char) {
                tmp = (token == Char) ? CHARCAST : INTCAST; // cast type
                match(token);
                while (token == Mul) {
                    match(Mul);
                    tmp = tmp + PTR;
                }

                match(')');

                expression(Inc); // cast has precedence as Inc(++)

                expr_type  = tmp;
            } else {
                // normal parenthesis
                expression(Assign);
                match(')');
            }
        }
        else if (token == Mul) {
            // dereference *<addr>
            match(Mul);
            expression(Inc); // dereference has the same precedence as Inc(++)

            if (expr_type >= PTR) {
                expr_type = expr_type - PTR;
            } else {
                echoUSB("bad dereference\n");
		longjmp(error_exit, line);
            }

            *++text = (expr_type == CHARCAST) ? LC : LI;
        }
        else if (token == And) {
            // get the address of
            match(And);
            expression(Inc); // get the address of
            if (*text == LC || *text == LI) {
                text --;
            } else {
                echoUSB("bad address of\n");
		longjmp(error_exit, line);
            }

            expr_type = expr_type + PTR;
        }
        else if (token == '!') {
            // not
            match('!');
            expression(Inc);

            // emit code, use <expr> == 0
            *++text = PUSH;
            *++text = IMM;
            *++text = 0;
            *++text = EQ;

            expr_type = INTCAST;
        }
        else if (token == '~') {
            // bitwise not
            match('~');
            expression(Inc);

            // emit code, use <expr> XOR -1
            *++text = PUSH;
            *++text = IMM;
            *++text = -1;
            *++text = XOR;

            expr_type = INTCAST;
        }
        else if (token == Add) {
            // +var, do nothing
            match(Add);
            expression(Inc);

            expr_type = INTCAST;
        }
        else if (token == Sub) {
            // -var
            match(Sub);

            if (token == Num) {
                *++text = IMM;
                *++text = -token_val;
                match(Num);
            } else {

                *++text = IMM;
                *++text = -1;
                *++text = PUSH;
                expression(Inc);
                *++text = MUL;
            }

            expr_type = INTCAST;
        }
        else if (token == Inc || token == Dec) {
            tmp = token;
            match(token);
            expression(Inc);
            if (*text == LC) {
                *text = PUSH;  // to duplicate the address
                *++text = LC;
            } else if (*text == LI) {
                *text = PUSH;
                *++text = LI;
            } else {
                echoUSB("bad lvalue of pre-increment\n");
		longjmp(error_exit, line);
            }
            *++text = PUSH;
            *++text = IMM;
            *++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
            *++text = (tmp == Inc) ? ADD : SUB;
            *++text = (expr_type == CHARCAST) ? SC : SI;
        }
        else {
            echoUSB("bad expression\n");
	    longjmp(error_exit, line);
        }
    }

    // binary operator and postfix operators.
    {
        while (token >= level) {
            // handle according to current operator's precedence
            tmp = expr_type;
            if (token == Assign) {
                // var = expr;
                match(Assign);
                if (*text == LC || *text == LI) {
                    *text = PUSH; // save the lvalue's pointer
                } else {
                    echoUSB("bad lvalue in assignment\n");
		    longjmp(error_exit, line);
                }
                expression(Assign);

                expr_type = tmp;
                *++text = (expr_type == CHARCAST) ? SC : SI;
            }
            else if (token == Cond) {
                // expr ? a : b;
                match(Cond);
                *++text = JZ;
                addr = ++text;
                expression(Assign);
                if (token == ':') {
                    match(':');
                } else {
                    echoUSB("missing colon in conditional\n");
		    longjmp(error_exit, line);
                }
                *addr = (int)(text + 3);
                *++text = JMP;
                addr = ++text;
                expression(Cond);
                *addr = (int)(text + 1);
            }
            else if (token == Lor) {
                // logic or
                match(Lor);
                *++text = JNZ;
                addr = ++text;
                expression(Lan);
                *addr = (int)(text + 1);
                expr_type = INTCAST;
            }
            else if (token == Lan) {
                // logic and
                match(Lan);
                *++text = JZ;
                addr = ++text;
                expression(Or);
                *addr = (int)(text + 1);
                expr_type = INTCAST;
            }
            else if (token == Or) {
                // bitwise or
                match(Or);
                *++text = PUSH;
                expression(Xor);
                *++text = OR;
                expr_type = INTCAST;
            }
            else if (token == Xor) {
                // bitwise xor
                match(Xor);
                *++text = PUSH;
                expression(And);
                *++text = XOR;
                expr_type = INTCAST;
            }
            else if (token == And) {
                // bitwise and
                match(And);
                *++text = PUSH;
                expression(Eq);
                *++text = AND;
                expr_type = INTCAST;
            }
            else if (token == Eq) {
                // equal ==
                match(Eq);
                *++text = PUSH;
                expression(Ne);
                *++text = EQ;
                expr_type = INTCAST;
            }
            else if (token == Ne) {
                // not equal !=
                match(Ne);
                *++text = PUSH;
                expression(Lt);
                *++text = NE;
                expr_type = INTCAST;
            }
            else if (token == Lt) {
                // less than
                match(Lt);
                *++text = PUSH;
                expression(Shl);
                *++text = LT;
                expr_type = INTCAST;
            }
            else if (token == Gt) {
                // greater than
                match(Gt);
                *++text = PUSH;
                expression(Shl);
                *++text = GT;
                expr_type = INTCAST;
            }
            else if (token == Le) {
                // less than or equal to
                match(Le);
                *++text = PUSH;
                expression(Shl);
                *++text = LE;
                expr_type = INTCAST;
            }
            else if (token == Ge) {
                // greater than or equal to
                match(Ge);
                *++text = PUSH;
                expression(Shl);
                *++text = GE;
                expr_type = INTCAST;
            }
            else if (token == Shl) {
                // shift left
                match(Shl);
                *++text = PUSH;
                expression(Add);
                *++text = SHL;
                expr_type = INTCAST;
            }
            else if (token == Shr) {
                // shift right
                match(Shr);
                *++text = PUSH;
                expression(Add);
                *++text = SHR;
                expr_type = INTCAST;
            }
            else if (token == Add) {
                // add
                match(Add);
                *++text = PUSH;
                expression(Mul);

                expr_type = tmp;
                if (expr_type > PTR) {
                    // pointer type, and not `char *`
                    *++text = PUSH;
                    *++text = IMM;
                    *++text = sizeof(int);
                    *++text = MUL;
                }
                *++text = ADD;
            }
            else if (token == Sub) {
                // sub
                match(Sub);
                *++text = PUSH;
                expression(Mul);
                if (tmp > PTR && tmp == expr_type) {
                    // pointer subtraction
                    *++text = SUB;
                    *++text = PUSH;
                    *++text = IMM;
                    *++text = sizeof(int);
                    *++text = DIV;
                    expr_type = INTCAST;
                } else if (tmp > PTR) {
                    // pointer movement
                    *++text = PUSH;
                    *++text = IMM;
                    *++text = sizeof(int);
                    *++text = MUL;
                    *++text = SUB;
                    expr_type = tmp;
                } else {
                    // numeral subtraction
                    *++text = SUB;
                    expr_type = tmp;
                }
            }
            else if (token == Mul) {
                // multiply
                match(Mul);
                *++text = PUSH;
                expression(Inc);
                *++text = MUL;
                expr_type = tmp;
            }
            else if (token == Div) {
                // divide
                match(Div);
                *++text = PUSH;
                expression(Inc);
                *++text = DIV;
                expr_type = tmp;
            }
            else if (token == Mod) {
                // Modulo
                match(Mod);
                *++text = PUSH;
                expression(Inc);
                *++text = MOD;
                expr_type = tmp;
            }
            else if (token == Inc || token == Dec) {
                // postfix inc(++) and dec(--)
                // we will increase the value to the variable and decrease it
                // on `ax` to get its original value.
                if (*text == LI) {
                    *text = PUSH;
                    *++text = LI;
                }
                else if (*text == LC) {
                    *text = PUSH;
                    *++text = LC;
                }
                else {
                    echoUSB("bad value in increment\n");
		    longjmp(error_exit, line);
                }

                *++text = PUSH;
                *++text = IMM;
                *++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
                *++text = (token == Inc) ? ADD : SUB;
                *++text = (expr_type == CHARCAST) ? SC : SI;
                *++text = PUSH;
                *++text = IMM;
                *++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
                *++text = (token == Inc) ? SUB : ADD;
                match(token);
            }
            else if (token == Brak) {
                // array access var[xx]
                match(Brak);
                *++text = PUSH;
                expression(Assign);
                match(']');

                if (tmp > PTR) {
                    // pointer, `not char *`
                    *++text = PUSH;
                    *++text = IMM;
                    *++text = sizeof(int);
                    *++text = MUL;
                }
                else if (tmp < PTR) {
                    echoUSB("pointer type expected\n");
		    longjmp(error_exit, line);
                }
                expr_type = tmp - PTR;
                *++text = ADD;
                *++text = (expr_type == CHARCAST) ? LC : LI;
            }
            else {
                echoUSB("compiler error, line, token\n");
		longjmp(error_exit, line);
            }
        }
    }
}

void statement() {
    // there are 8 kinds of statements here:
    // 1. if (...) <statement> [else <statement>]
    // 2. while (...) <statement>
    // 3. { <statement> }
    // 4. return xxx;
    // 5. <empty statement>;
    // 6. expression; (expression end with semicolon)

    int *a, *b; // bess for branch control

    if (token == If) {
        // if (...) <statement> [else <statement>]
        //
        //   if (...)           <cond>
        //                      JZ a
        //     <statement>      <statement>
        //   else:              JMP b
        // a:
        //     <statement>      <statement>
        // b:                   b:
        //
        //
        match(If);
        match('(');
        expression(Assign);  // parse condition
        match(')');

        // emit code for if
        *++text = JZ;
        b = ++text;

        statement();         // parse statement
        if (token == Else) { // parse else
            match(Else);

            // emit code for JMP B
            *b = (int)(text + 3);
            *++text = JMP;
            b = ++text;

            statement();
        }

        *b = (int)(text + 1);
    }
    else if (token == While) {
        //
        // a:                     a:
        //    while (<cond>)        <cond>
        //                          JZ b
        //     <statement>          <statement>
        //                          JMP a
        // b:                     b:
        match(While);

        a = text + 1;

        match('(');
        expression(Assign);
        match(')');

        *++text = JZ;
        b = ++text;

        statement();

        *++text = JMP;
        *++text = (int)a;
        *b = (int)(text + 1);
    }
    else if (token == '{') {
        // { <statement> ... }
        match('{');

        while (token != '}') {
            statement();
        }

        match('}');
    }
    else if (token == Return) {
        // return [expression];
        match(Return);

        if (token != ';') {
            expression(Assign);
        }

        match(';');

        // emit code for return
        *++text = LEV;
    }
    else if (token == ';') {
        // empty statement
        match(';');
    }
    else {
        // a = b; or function_call();
        expression(Assign);
        match(';');
    }
}

void enum_declaration() {
    // parse enum [id] { a = 1, b = 3, ...}
    int i;
    i = 0;
    while (token != '}') {
        if (token != Id) {
            echoUSB("bad enum identifier line, token\n");
	    longjmp(error_exit, line);
        }
        next();
        if (token == Assign) {
            // like {a=10}
            next();
            if (token != Num) {
                echoUSB("bad enum initializer\n");
	        longjmp(error_exit, line);
            }
            i = token_val;
            next();
        }

        current_id[Class] = Num;
        current_id[Type] = INTCAST;
        current_id[Value] = i++;

        if (token == ',') {
            next();
        }
    }
}

void function_parameter() {
    int type;
    int params;
    params = 0;
    while (token != ')') {
        // int name, ...
        type = INTCAST;
        if (token == Int) {
            match(Int);
        } else if (token == Char) {
            type = CHARCAST;
            match(Char);
        }

        // pointer type
        while (token == Mul) {
            match(Mul);
            type = type + PTR;
        }

        // parameter name
        if (token != Id) {
            echoUSB("bad parameter declaration\n");
	    longjmp(error_exit, line);
        }
        if (current_id[Class] == Loc) {
            echoUSB("duplicate parameter declaration\n");
	    longjmp(error_exit, line);
        }

        match(Id);
        // store the local variable
        current_id[BClass] = current_id[Class]; current_id[Class]  = Loc;
        current_id[BType]  = current_id[Type];  current_id[Type]   = type;
        current_id[BValue] = current_id[Value]; current_id[Value]  = params++;   // index of current parameter

        if (token == ',') {
            match(',');
        }
    }
    index_of_bp = params+1;
}

void function_body() {
    // type func_name (...) {...}
    //                   -->|   |<--

    // ... {
    // 1. local declarations
    // 2. statements
    // }

    int pos_local; // position of local variables on the stack.
    int type;
    pos_local = index_of_bp;

    while (token == Int || token == Char) {
        // local variable declaration, just like global ones.
        basetype = (token == Int) ? INTCAST : CHARCAST;
        match(token);

        while (token != ';') {
            type = basetype;
            while (token == Mul) {
                match(Mul);
                type = type + PTR;
            }

            if (token != Id) {
                // invalid declaration
                echoUSB("bad local declaration\n");
	        longjmp(error_exit, line);
            }
            if (current_id[Class] == Loc) {
                // identifier exists
                echoUSB("duplicate local declaration\n");
	        longjmp(error_exit, line);
            }
            match(Id);

            // store the local variable
            current_id[BClass] = current_id[Class]; current_id[Class]  = Loc;
            current_id[BType]  = current_id[Type];  current_id[Type]   = type;
            current_id[BValue] = current_id[Value]; current_id[Value]  = ++pos_local;   // index of current parameter

            if (token == ',') {
                match(',');
            }
        }
        match(';');
    }

    // save the stack size for local variables
    *++text = ENT;
    *++text = pos_local - index_of_bp;

    // statements
    while (token != '}') {
        statement();
    }

    // emit code for leaving the sub function
    *++text = LEV;
}

void function_declaration() {
    // type func_name (...) {...}
    //               | this part

    match('(');
    function_parameter();
    match(')');
    match('{');
    function_body();
    //match('}');

    // unwind local variable declarations for all local variables.
    current_id = symbols;
    while (current_id[Token]) {
        if (current_id[Class] == Loc) {
            current_id[Class] = current_id[BClass];
            current_id[Type]  = current_id[BType];
            current_id[Value] = current_id[BValue];
        }
        current_id = current_id + IdSize;
    }
}

void global_declaration() {
    // int [*]id [; | (...) {...}]


    int type; // tmp, actual type for variable
    int i; // tmp

    basetype = INTCAST;

    // parse enum, this should be treated alone.
    if (token == Enum) {
        // enum [id] { a = 10, b = 20, ... }
        match(Enum);
        if (token != '{') {
            match(Id); // skip the [id] part
        }
        if (token == '{') {
            // parse the assign part
            match('{');
            enum_declaration();
            match('}');
        }

        match(';');
        return;
    }

    // parse type information
    if (token == Int) {
        match(Int);
    }
    else if (token == Char) {
        match(Char);
        basetype = CHARCAST;
    }

    // parse the comma seperated variable declaration.
    while (token != ';' && token != '}') {
        type = basetype;
        // parse pointer type, note that there may exist `int ****x;`
        while (token == Mul) {
            match(Mul);
            type = type + PTR;
        }

        if (token != Id) {
            // invalid declaration
            echoUSB("bad global declaration\n");
	    longjmp(error_exit, line);
        }
        if (current_id[Class]) {
            // identifier exists
            echoUSB("duplicate global declaration\n");
	    longjmp(error_exit, line);
        }
        match(Id);
        current_id[Type] = type;

        if (token == '(') {
            current_id[Class] = Fun;
            current_id[Value] = (int)(text + 1); // the memory address of function
            function_declaration();
        } else {
            // variable declaration
            current_id[Class] = Glo; // global variable
            current_id[Value] = (int)data; // assign memory address
            data = data + sizeof(int);
        }

        if (token == ',') {
            match(',');
        }
    }
    next();
}

void program() {
    // get next token
    next();
    while (token > 0) {
        global_declaration();
    }
}

int eval() {
    int op, *tmp;

    cycle = 0;

    while (1) {
	flushUSB(); // usb service
        IRhandler(); // service IR packets

	if (stacklow > sp) stacklow = sp; /* stack low water mark */
	
        cycle ++;
        op = *pc++; // get next operation code

        if (op == IMM)       {ax = *pc++;}                                     // load immediate value to ax
        else if (op == LC)   {ax = *(char *)ax;}                               // load character to ax, address in ax
        else if (op == LI)   {ax = *(int *)ax;}                                // load integer to ax, address in ax
        else if (op == SC)   {ax = *(char *)*sp++ = ax;}                       // save character to address, value in ax, address on stack
        else if (op == SI)   {*(int *)*sp++ = ax;}                             // save integer to address, value in ax, address on stack
        else if (op == PUSH) {*--sp = ax;}                                     // push the value of ax onto the stack
        else if (op == JMP)  {pc = (int *)*pc;}                                // jump to the address
        else if (op == JZ)   {pc = ax ? pc + 1 : (int *)*pc;}                   // jump if ax is zero
        else if (op == JNZ)  {pc = ax ? (int *)*pc : pc + 1;}                   // jump if ax is not zero
        else if (op == CALL) {*--sp = (int)(pc+1); pc = (int *)*pc;}           // call subroutine
        //else if (op == RET)  {pc = (int *)*sp++;}                              // return from subroutine;
        else if (op == ENT)  {*--sp = (int)bp; bp = sp; sp = sp - *pc++;}      // make new stack frame
        else if (op == ADJ)  {sp = sp + *pc++;}                                // add esp, <size>
        else if (op == LEV)  {sp = bp; bp = (int *)*sp++; pc = (int *)*sp++;}  // restore call frame and PC
        else if (op == LEA)  {ax = (int)(bp + *pc++);}                         // load address for arguments.

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

        else if (op == EXIT) { 
		//printf("exit(%d)", *sp); 
		return *sp;
	}
        else if (op == PRT) { /* changed: print string */
		//tmp = sp + pc[1]; 
		//ax = printf((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]);
                //echoUSB("print ");
                //echoUSB((char *)tmp[-1]);
		if ((char *)sp[0] < (char *)0xa0000000) 
		    echoUSB("prt <addr>");
                else
		    echoUSB((char *)sp[0]);
                echoUSB("\r\n");
                ax = 0;
	}
        else if (op == PRTD) { /* print(arg) as hex */ 
		static char dbuffer[9];
		decDump((int)sp[0], dbuffer);
                echoUSB(dbuffer);
                echoUSB("\r\n");
                ax = 0;
	}
        else if (op == PRTX) { /* print(arg) as hex */ 
		static char dbuffer[9];
		hexDump((int)sp[0], dbuffer);
                echoUSB(dbuffer);
                echoUSB("\r\n");
                ax = 0;
	}
        //else if (op == MALC) { ax = (int)malloc(*sp);}
        else if (op == MALC) { }
        //else if (op == MSET) { ax = (int)memset((char *)sp[2], sp[1], *sp);}
        else if (op == MSET) { }
        //else if (op == MCMP) { ax = memcmp((char *)sp[2], (char *)sp[1], *sp);}
        else if (op == MCMP) { }
        else if (op == FLARELED) { flareled((char)sp[2], (char)sp[1], (char)sp[0]); }
        else if (op == LED) { led((char)sp[2], (char)sp[1], (char)sp[0]); }
        else if (op == FBMOVE) { FbMove((char)sp[1], (char)sp[0]); }
        else if (op == FBWRITE) { FbWriteLine((char *)sp[0]); }
        else if (op == BACKLIGHT) { backlight((char)sp[0]); }
        else if (op == IRRECEIVE) { ax = (unsigned int)IRreceive(); }
        else if (op == IRSEND) { IRsend((int)sp[0]); }
        else if (op == SETNOTE) { setNote((int)sp[1], (int)sp[0]); }
        else if (op == GETBUTTON) { ax = (int)getButton(); } /* return button bitmask */
        else if (op == GETDPAD) { ax = (int)getDPAD(); }  /* return button bitmask */
        else if (op == CONTRAST) { contrast((char)sp[0]); }
        else if (op == IRSTATS) { IRstats(); }
        else if (op == SETTIME) { setTime((char)sp[2], (char)sp[1], (char)sp[0]); }
        else if (op == GETTIME) { ax = (int)getTime(); }
        else if (op == FBLINE) { FbLine((char)sp[3], (char)sp[2], (char)sp[1], (char)sp[0]); }
        else if (op == FBCLEAR) { FbClear(); }
        else if (op == FLASHW) { ax = (unsigned int)IflashWrite((unsigned int)sp[0]); }
        else if (op == FLASHR) { ax = IflashRead((unsigned int)sp[0]); }
        else if (op == IALLOC) { interpreter_alloc((unsigned int)sp[3], (unsigned int)sp[2], (unsigned int)sp[1], (unsigned int)sp[0]); }
        else {
            echoUSB("unknown instruction\n");
	    longjmp(error_exit, op);
        }
    }
}

const char Csrc[] = "char else enum if int return sizeof while "
          "print printd printx malloc memset memcmp "
	  "flareled led FbMove FbWrite backlight "
	  "IRreceive IRsend setNote getButton getDPAD contrast "
	  "IRstats setTime getTime FbLine FbClear flashWrite flashRead interpAlloc "
	  "exit void main";

/*
   use the frame buffer memory
   but you can't print/draw or will 
   corrupt the ram arean


*/
#define TEXTSZ (2048+1024)
#define DATASZ 256
#define STACKSZ 256
#define SYMBOLSZ 4096

#define INTERP_RAM interpreter_ram
#define INTERP_RAM_SIZE (TEXTSZ+DATASZ+STACKSZ+SYMBOLSZ)
char interpreter_ram[INTERP_RAM_SIZE];

/* default percent (100*scaled) of ram for each section */
unsigned int G_textpct=38, G_datapct=6, G_stackpct=6, G_symbolpct=50;

/* (percent * ramsize)/100 */
unsigned int textsz, datasz, stacksz, symbolsz;

static int alloc_done=0;
 
#define RESTART 99999
void interpreter_alloc(int textpct, int datapct, int stackpct, int symbolpct)
{
    if (alloc_done) return; // second time thru we don't run */

    G_textpct = textpct;
    G_datapct = datapct;
    G_stackpct = stackpct;
    G_symbolpct = symbolpct;

    longjmp(error_exit, RESTART);
}

int Gusb_fb = 0;
void interpreter_use_fb(int yes)
{
   Gusb_fb = 1;
}

void interpreter_memset()
{
    int ramsz;

    if (Gusb_fb == 0) {
	ta_heap_start = INTERP_RAM;
	ta_heap_limit = &INTERP_RAM[INTERP_RAM_SIZE];
    }
    else {
	/* 
	    LCDbuffer is short int 132*132*2=34848, 
	    leave first 4 text lines alone
	    == 32 scanlines or 8448 bytes
	    132*132*2-132*8*2*4
	    26400

	    reserved at the begining == Top scans of LCD
	*/
	#define TEXTLINES 4
	#define SCANLINES_RESERVED (8 * TEXTLINES)
	#define LCD_RESERVED (SCANLINES_RESERVED * LCD_XSIZE * sizeof(short))

	ta_heap_start = (char *)LCDbuffer;
	//ta_heap_start += 132*2 * 8*4;
	ta_heap_start += LCD_RESERVED;
	ta_heap_limit = (char *)&LCDbuffer[FBSIZE];

	FbClear(); 
	FbMove(0,0); /* in case user forgets */
    }
    ramsz = (ta_heap_limit - ta_heap_start);
    textsz = (G_textpct * ramsz) / 100;
    datasz = (G_datapct * ramsz) / 100;
    stacksz = (G_stackpct * ramsz) / 100;
    symbolsz = (G_symbolpct * ramsz) / 100;
}

void init_interpreter()
{
    int i, fd;

    text=0;
    stack=0;
    data=0;
    symbols=0;

    interpreter_memset();

    /* init tinyalloc */
    ta_init();

    textbase = text = ta_alloc(textsz);
    memset(textbase, 0, textsz);

    database = data = ta_alloc(datasz);
    memset(database, 0, datasz);

    stackbase = stack = ta_alloc(stacksz);
    memset(stackbase, 0, stacksz);

    symbolbase = symbols = ta_alloc(symbolsz);
    memset(symbolbase, 0, symbolsz);

    idmain = 0;
    src=0;

    pc=0;
    bp=0;
    sp=0;
    ax=0;
    cycle=0;
    current_id=0;
    token_val=0;
    token=0;
    basetype=0;
    expr_type=0;
    index_of_bp = 0;
    line = 1;

    token_val=0;

    line = 1;

    src = (char *)Csrc;

     // add keywords to symbol table
    i = Char;
    while (i <= While) {
        next();
        current_id[Token] = i++;
    }

    // add library to symbol table
    i = PRT;
    while (i <= EXIT) {
        next();
        current_id[Class] = Sys;
        current_id[Type] = INTCAST;
        current_id[Value] = i++;
    }

    next(); current_id[Token] = Char; // handle void type
    next(); idmain = current_id; // keep track of main
}

int run()
{
    int *tmp;
    int argc=0;
    char *argv[]={""};

    if (!(pc = (int *)idmain[Value])) {
	echoUSB("main() not defined");
	longjmp(error_exit, 666);
    }

    // setup stack
    stacklow = sp = (int *)((int)stack + stacksz);
    *--sp = EXIT; // call exit if main returns
    *--sp = PUSH; tmp = sp;
    *--sp = argc;
    *--sp = (int)argv;
    *--sp = (int)tmp;

    return eval();
}

void interp_stats()
{
    unsigned int used_textsz, used_datasz, used_stacksz, used_symbolsz;
    char textbuf[9]; /* 1 for null */

    used_textsz = (unsigned int)text - (unsigned int)textbase;
    used_textsz = (used_textsz * 100) / textsz;

    used_datasz = (unsigned int)data - (unsigned int)database;
    used_datasz = (used_datasz * 100) / datasz;

    /* top of stack - stacklow point */
    used_stacksz =  ((unsigned int)stackbase + stacksz) - (unsigned int)stacklow;
    used_stacksz = (used_stacksz * 100) / stacksz;

    current_id = symbols;
    while (current_id[Token]) {
        current_id = current_id + IdSize;
    }
    used_symbolsz = (unsigned int)current_id - (unsigned int)symbols ;
    used_symbolsz = (used_symbolsz * 100) / symbolsz;

    //echoUSB("TX,DA,ST,SY");

    decDump(used_textsz, textbuf);
    echoUSB("\r\nText %");
    echoUSB(textbuf);

    decDump(used_datasz, textbuf);
    echoUSB("\r\nData %");
    echoUSB(textbuf);

    decDump(used_stacksz, textbuf);
    echoUSB("\r\nStack %");
    echoUSB(textbuf);

    decDump(used_symbolsz, textbuf);
    echoUSB("\r\nSymbol %");
    echoUSB(textbuf);

    echoUSB("\r\n");
}

int interpreter_catcher(char *prog)
{
   int r;

   r=0;

   /* in case of error */
   if (r = setjmp(error_exit)) {
	if (r != RESTART) r += 100000; /* indicate error with offset */
   }
   else {
	init_interpreter();
	src = prog;
	program();
	r = run();
   }
   return r;
}

#ifdef MAINAPP
char progsrc[] = "\
int main() {\n\
   FbWrite(\"write me\");\n\
   return 0;\
}\
";

int main() 
{
    char *prog = progsrc;
#else


int interpreter_main(char *prog) 
{
#endif
    int r=0;

    alloc_done=0;

    if ((r = interpreter_catcher(prog)) == RESTART) {
	alloc_done = 1; /* only restart once */
	r = interpreter_catcher(prog);
	r += 200000; /* indicate restart with offset */
    }

    return r;
}
