/// "useCount" : 4
int i = 0;

/// "useCount" : 2
int foo()
{
    i = 1;
    return i = i + 1;
}

/// "useCount" : 2
class SomeClass
{
public:
    /// "useCount" : 3
    int i;
    /// "useCount" : 0
    char foo();
};

/// "useCount" : 4
namespace SomeNS
{
    /// "useCount" : 3
    SomeClass someClass;
    /// "useCount" : 2
    int i = foo();
}

int main()
{
    SomeNS::i = i + foo() + SomeNS::someClass.i;
    using namespace SomeNS;
    /// "useCount" : 1
    SomeClass user = someClass;
    user.i = someClass.i = SomeNS::i;
}

class myClass
{
    /// "useCount" : 1
    int pre_foo;

    void foo()
    {
        pre_foo = 1;
        post_foo = 1;
    }

    /// "useCount" : 1
    int post_foo;
};

/// "useCount" : 7
enum RandomEnum
{
   /// "useCount" : 4
   Random_Random
};
/// "useRanges" : [ "[(72, 5), (72, 7)]", "[(72, 10), (72, 12)]" ]
RandomEnum operator<<(RandomEnum, RandomEnum)
{
   return Random_Random;
}
/// "useRanges" : [ "[(73, 5), (73, 6)]", "[(73, 9), (73, 10)]" ]
RandomEnum operator<(RandomEnum, RandomEnum)
{
   return Random_Random;
}
void operators()
{
   RandomEnum r = Random_Random, t = Random_Random;
   r << t << r;
   r < t < r;
}
