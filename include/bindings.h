enum {
  MAINBTN,
  UPBTN,
  DOWNBTN,
  LEFTBTN,
  RIGHTBTN,
  HOUR,
  MIN,
  SEC,
  BADGEID,
  NAME,
  FL_ID,
  FL_ADDR,
  DRAWLCD8,
  DRBOB_,
  ADC,
  ROTATE,
  LEDBRIGHT,
  MUTE,
  RETVAL,
  LASTBINDING,
};

/* binding record type */
enum {
    B_VARIABLE,
    B_FUNCTION,
};

/* binding record return, parm, or type */
enum {
    B_VOID,
    B_CHAR,
    B_CHARSTAR,
    B_INT,
    B_INTSTAR,
};

/*
   8 bytes
*/
struct param_t {
    char *name;
    unsigned var_fun:1;
    unsigned ret:3;
    unsigned arg0:4;
    unsigned arg1:4;
    unsigned arg2:4;
    unsigned arg3:4;
    unsigned arg4:4;
    unsigned arg5:4;
    unsigned arg6:4;
};

/*
   4 bytes
*/

union var_fun {
    void (*vfn_uc_i)(unsigned char , int);
    int (*ifni)(int);
    void (*vfnuc)(unsigned char);
    char *(*cfn)();
    void (*vfn)();
    void (*vfni)(int);
    const unsigned int *cuip;
    int *ip;
    unsigned short *usp;
    int i;
    unsigned char *ucp;
    char *cp;
    char c;
};

struct binding_t {
    union var_fun vf;
};

extern struct binding_t bindings[];
extern struct param_t parms[];
