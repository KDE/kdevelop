namespace Blah2 {
  int nsTest2;
}

namespace Blah {
  class Foo {
    int m_test;
    static int s_test2;

    void test(int input);

    void test5() {}
  };

  int nsTest;

  using namespace Blah2;
}

int Blah::Foo::s_test2 = 0;

int test3()   
{
  // Unqualified + before using statement - error
  nsTest = 2;
  nsTest2 = 2;

  return Blah2::nsTest2;
}

using namespace Blah;

int test2()
{
  // Success - using statement applies
  nsTest = 4;
  // Success - found in nested using statement.
  nsTest2 = 5;
  // Success - uses nested statement
  Blah::nsTest2 = 7;

  return Blah::nsTest;
}

class Foo2 {};

int Foo::test(int input)
{
  // Use before definition - error
  result = 3;

  int result = m_test;

  try {
    int Z = 4;
    test(Z);
  } catch (Foo2 error) {
    return Z;
  }

  Foo f;
  if (result = f.test())
    return 1;

  // Declaration, because f2 is not defined prior
  Foo2* f2;
  f2 = new Foo2();

  // Expression, because both are defined
  f * f2;

  for (int j = 0; j < 4; ++j) {
    if (j ^ input > 200)
      return -1;
  }

  // Use of j outside of scope - error
  if (j == 4)
    return 4;

  result *= input;

  return m_test;
}