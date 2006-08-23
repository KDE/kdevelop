namespace Blah {
  class Foo {
    int m_test;
    static int s_test2;

    void test();
  };

  int nsTest;
}

Blah::Foo::s_test2 = 0;

int test3()
{
  // Unqualified + before using statement - error
  nsTest = 2;

  return Blah::nsTest;
}

using namespace Blah;

int test2()
{
  // Success - using statement applies
  nsTest = 4;

  return Blah::nsTest;
}

int Foo::test(int input)
{
  // Use before definition - error
  result = 3;

  int result = m_test;

  Foo f;
  if (result = f.test())
    return;

  Foo* f2 = new Foo();

  for (int j = 0; j < 4; ++j) {
    if (j ^ input > 200)
      return -1;
  }

  // Use of j outside of scope - error
  if (j == 4)
    return;

  result *= input;

  return m_test;
} 