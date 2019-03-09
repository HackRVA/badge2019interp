enum {
  MAINBUTTON,
  UPBUTTON,
  DOWNBUTTON,
  LEFTBUTTON,
  RIGHTBUTTON,
  HOUR,
  MIN,
  SEC,
  BADGEID,
  NAME,
  FLASHEDID,
  FLASHADDR,
  DRAWLCD8,
  DRBOB_BIND,
  LASTBINDING,
};

enum {
    B_VARIABLE,
    B_FUNCTION,
};

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
    int (*fun)();
    int *ip;
    int i;
    char *cp;
    char c;
};

struct binding_t {
    union var_fun vf;
};

extern struct binding_t bindings[];
extern struct param_t parms[];
