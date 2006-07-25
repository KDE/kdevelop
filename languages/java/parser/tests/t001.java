
/*
 * an example
 */

import java.lang.*;

class SuperClass {
  { /* initializer */ }
};

class JavaExample extends SuperClass
{
  int fact( int n, String ... args[] ) {
    if( n == 0 )
      return 1;
    else {
      n *= fact(n - 1);
    }
    return n;
  }

  String test() {
    return new Integer(10).toString();
  }

  int main(){ return fact(1000); }
};

