//
//
// C++ Interface for module: Trie
//
// Description: 
//
//
// Author: exa
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef Trie_Interface
#define Trie_Interface

//#include "generic/General.hxx"
#include <vector>

// Generic trie structure
// Index must have access to its first element via first()
// and a function strip() and length()  must be defined on it
template <typename Symbol, typename String, typename Component>
class Trie_Impl
{
public:
  typedef vector<Trie_Impl*> Subtries;

  Trie_Impl() {}
  Trie_Impl(const Symbol & s) : node(s) {}
  Trie_Impl(const Symbol & s, const Component & c) : node(s,c) {}

  // destruct
  ~Trie_Impl()
  {
    for (typename Subtries::iterator i=children.begin();
	 i!=children.end(); i++) {
      delete *i;
    }
//   }

  class Node {
  public:
    Symbol symbol;
    Component component;
    Node() {}
    Node(const Symbol & s)
      : symbol(s) {}
    Node(const Symbol & s, const Component & c)
      : symbol(s), component(c) {}
//     Symbol & symbol () {
//       return symbol;
//     }
//     Component & component() {
//       return component;
//     }
  };

  bool have_symbol(const String& string) {
    return node.symbol == first(string);
  }

  Trie_Impl* create_trie(const String& string, const Component& component)
  {
    assert(!empty(string));
    Symbol sym = first(string);
    String stripped = string; strip(stripped);
    if (empty(stripped)) {
      return new Trie_Impl(sym, component);
    }
    else {
      Trie_Impl* trie = new Trie_Impl(sym);
      trie->children.push_back(create_trie(stripped, component));
      return trie;
    }
  }

  // returns the subtrie node into which it has been inserted
  Component* insert(const String& string, const Component & component) {
    if (empty(string))
      return 0;
    else if (have_symbol(string)) {

      // shortcut: we've already got it
      if (length(string)==1)
	return &node.component;

      String rest = string; strip(rest);
      // trie (he he) to insert to children
      // and if successful return true
      for (Subtries::iterator i=children.begin(); i!=children.end();
	   i++) {
	Component* comp; 
	if (comp=(*i)->insert(rest, component))
	  return comp;
      }
      // nobody has it, let's create that freaking subtrie
      // which consists of a single path
      children.push_back(create_trie(rest, component));
      return children.back()->query(rest);
    }
    return 0;
  }

  Component* query(const String& string) {
    assert(!empty(string));
    if (have_symbol(string)) {

      // we've found it
      if (length(string)==1)
	return &node.component;

      String rest = string; strip(rest);

      //scan children to see if they have got it
      for (Subtries::iterator i=children.begin(); i!=children.end(); i++) {
	if ((*i)->have_symbol(rest))
	  return (*i)->query(rest);
      }
    }
    else {
      // nobody's got it
      return 0;
    }
  }

private:
  Node node;
  Subtries children;
};

// This is trie interface
template <typename _Symbol, typename _String, typename _Component>
class Trie : public Trie_Impl<_Symbol, _String, _Component>
{
public:
  typedef Trie_Impl<Symbol, String, Component> Base;
  Trie() : Base(blank(String())) {}

  Component* query(String string) {
    Symbol symbol = blank(string);
    add_prefix(string, symbol);
    return Base::query(string);
  }

  Component* insert(String string, const Component & component)
  {
    Symbol symbol = blank(string);
    add_prefix(string, symbol);
    return Base::insert(string, component);
  }

private:
};

#endif
