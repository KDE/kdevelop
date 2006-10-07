#define Q_FOO(a) blah(int a)

#define qfoo(a) Q_FOO(a)

// Should be blah(int j)
qfoo(j)
