
#include <iostream>
#include "Trie.h"

int main()
{
  Structure::KStringTrie A;

  A.insert("abd", 3);
  A.insert("abc", 2);
  A.insert("bcg", 1);
  A.insert("bug", 5);

  cout << "query abd ";
  int* comp = A.query("abd");
  if (comp) cout << "val " << *comp << endl;
  else cout << "not found" << endl;
}
