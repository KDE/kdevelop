/* Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/ */

void test(double) {}
void test(...) {}
void test(char) {}

struct foo {};

int main()
{
  test(0.5d);
  test();
  test(foo());
  test('d', 0.1d);
  test('c');

  return 0;
}
