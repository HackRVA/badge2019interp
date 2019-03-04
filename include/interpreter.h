#ifdef INTERPRETER_H
#define INTERPRETER_H
    void interpreterInit0(); /* zero interal variable */
    /* optimize ram distribution */
    void setAlloc(int allocScanlines, int textpct, int datapct, int stackpct, int symbolpct);
    void interpreterAlloc(); /* alloc struct */
    interpreterStats(); /* dump ram distribution stats */
#endif
