
#include <iostream>
using std::cout;
using std::endl;

#include "kdict.h"

int main()
{

  int v1 = 7;
  int v2 = 5;
  int v3 = 11;

  KDict<int> A;

  cout << "A empty" << A << endl;

  A.insert("abd", &v1);
  cout << "inserted abd 7 " << A << endl;

  A.insert("abc", &v2);
  cout << "inserted abc 5 " << A << endl;

  A.insert("bcg", &v3);
  cout << "inserted bcg 11 " << A << endl;

  A.insert("bug", &v1);
  cout << "inserted bug 7 " << A << endl;

  unsigned int s = A.count();
  cout << "A has " << s << " elements" << endl;

  cout << "query abd ";
  int* comp = A.find("abd");
  cout << "found this: " << comp << endl;
  if (comp) cout << "val " << *comp << endl;
  else cout << "not found" << endl;

  return 0; // success

}
