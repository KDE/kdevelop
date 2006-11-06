EXPECT 2x DIV0 ERROR
#define A 0
.
#if 10 / A
KO
#else
OK
#endif
.
#if 10 % A
KO
#else
OK
#endif
.