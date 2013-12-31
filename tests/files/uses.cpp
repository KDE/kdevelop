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
};

/// "useCount" : 2
namespace SomeNS
{
    /// "useCount" : 2
    SomeClass someClass;
    /// "useCount" : 2
    int i = foo();
}

int main()
{
    SomeNS::i = i + foo();
    using namespace SomeNS;
    SomeClass user = SomeNS::someClass;
}
