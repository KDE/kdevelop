class A{};
/*
 * "EXPECT_FAIL" : { "useCount" : "typedef'ed base initializers are not supported" },
 * "useCount": 2
 */
typedef A B;
class C : B { C() : B() {} };
