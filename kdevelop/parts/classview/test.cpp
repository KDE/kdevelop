class GlobalFoo {
public:
  GlobalFoo();
  ~GlobalFoo();
  void doSomething() { oerks(); }
  double countSomething() { return murks(); }
  static int i;
};

class GlobalFooB : GlobalFoo { };
class GlobalFooC : GlobalFoo, GlobalFooB { };
class GlobalFooD : GlobalFoo, GlobalFooB, GlobalFooC { };

GlobalFoo::i = 0;

int main()
{}
    
struct aha {
    int x;
    int y;
    void foo();
};

int max;
double min;

namespace blubb {

class Foo {
public:
  Foo();
  ~Foo();
  void doSomething() { oerks(); }
  double countSomething() { return murks(); }
};

class FooBlubb {
public:
  FooBlubb();
  ~FooBlubb();
  void doSomething() { oerks(); }
  int countSomething() { return murks(); }
};

}

namespace bimmel {
namespace bommel {

class Foo {
public:
  Foo();
  ~Foo();
  void doSomething() { oerks(); }
  int countSomething() { return murks(); }
};

class FooBimmelBommel {
public:
  FooBimmelBommel();
  ~FooBimmelBommel();
  void doSomething() { oerks(); }
  int countSomething() { return murks(); }
  static int i;
};

FooBimmelBommel::i = 0;

int main()
{}
    
struct aha {
    int x;
    int y;
    void foo();
};

int max;
double min;
        
}
}
