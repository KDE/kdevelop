// this file is allowed to have erroneous code, which is still parseable

/*This example used to crash while building the type of Bar*/
/// "type" : { "toString" : "Bar< T >" }
template <typename T>
class Bar
{
    /// "type" : { "toString" : "function void (int)" }
    void foo(UnknownType);
    /// "returnType" : { "toString" : "int" }
    UnknownType foo();
};

/* Contains invalid namespace aliases, used to crash*/
namespace dr373 { // dr373: no
  // FIXME: This is valid.
  namespace X { int dr373; } // expected-note 2{{here}}
  struct dr373 { // expected-note {{here}}
    void f() {
      using namespace dr373::X; // expected-error {{no namespace named 'X' in 'dr373::dr373'}}
      int k = dr373; // expected-error {{does not refer to a value}}

      namespace Y = dr373::X; // expected-error {{no namespace named 'X' in 'dr373::dr373'}}
      k = Y::dr373;
    }
  };
}

#define BAR 2
/// "internalContext" : { "localDeclarationCount" : 4, "findDeclarations" : {
///    "sure" : { "defaultParameter" : "nullptr" },
///    "a" : { "defaultParameter" : "<parse error>" },
///    "b" : { "defaultParameter" : "BAR" },
///    "c" : { "defaultParameter" : "3" }
/// } }
int asdf(int *sure = nullptr, int a = NOT_DEFINED, int b = BAR, int c = 3);
