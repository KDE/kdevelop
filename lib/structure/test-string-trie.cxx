//
//
// C++ Implementation for module: Test
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "Trie.hxx"
#include <string>

inline char blank(const string& str) {
  return ' ';
}

inline char first(const string& str) {
  return str[0];
}

inline void strip(string& str) {
  str.erase(0,1);
}

inline bool empty(const string& str) {
  return str.empty();
}

inline int length(const string& str) {
  return str.size();
}

inline void add_prefix(string& str, const char& prefix) {
  str.insert(str.begin(), prefix);
//   cout << "add_prefix: prefix=<" << prefix <<  ">" << endl;
//   cout << "add_prefix: <" << str << ">" << endl;
}

int main()
{
  Trie<char, string, int> A;

  A.insert("abd", 3);
  A.insert("abc", 2);
  A.insert("bcg", 1);
  A.insert("bug", 5);

  cout << "query abd ";
  int* comp = A.query("abd");
  if (comp) cout << "val " << *comp << endl;
  else cout << "not found" << endl;

  cout << "query abk ";
  comp = A.query("abk");
  if (comp) cout << "val " << *comp << endl;
  else cout << "not found" << endl;

  cout << "query abc ";
  comp = A.query("abc");
  if (comp) cout << "val " << *comp << endl;
  else cout << "not found" << endl;

  cout << "query bug ";
  comp = A.query("bug");
  if (comp) cout << "val " << *comp << endl;
  else cout << "not found" << endl;
  
}
