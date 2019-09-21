typedef struct {
    /// "useCount": 1
    int foo;
} asdf;

typedef struct {
    /// "useCount": 1
    char foo;
} meh;

void blub()
{
    meh m;
    (void) m.foo;
    asdf a;
    (void) a.foo;
}

typedef struct {
    /// "useCount": 1
    int bitmask[1];
} bitmask_a;

typedef struct {
    /// "useCount": 1
    int bitmask[6];
} bitmask_c;

typedef union {
    /// "useCount": 1
    bitmask_c bitmask;
} bitmask_union;

void blub2()
{
    bitmask_union u;
    (void) u.bitmask;
    bitmask_c c;
    (void) c.bitmask;
    bitmask_a a;
    (void) a.bitmask;
}
