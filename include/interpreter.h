#ifndef INTERPRETER_H
#define INTERPRETER_H
    void interpreterInit0(); /* zero interal variable */
    /* optimize ram distribution */
    void setAlloc(int allocScanlines, int textpct, int datapct, int stackpct, int symbolpct);
    void interpreterAlloc(); /* alloc struct */
    void interpreterStats(); /* dump ram distribution stats */
    int dopersist(int argc, const int **argv); /* interp doesnt support arrays */
#endif
