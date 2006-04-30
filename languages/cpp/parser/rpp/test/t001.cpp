.
/*
  ciao roberto
*/
.
#define A 1
.
#if A - 1 == 0
OK
#else
KO
#endif
.
#undef A
.
#define A(a,b) a + b
#define C(a,b) A(b,a)
.
#define comp(a,b) #a #b
#define str(a) #a
.
C(1,2) comp(me, you) str(kdevelop is cool)
.