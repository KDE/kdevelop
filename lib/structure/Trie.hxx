//
//
// C++ Interface for module: Trie
//
// Description:
//
//
// Author: Eray Ozkural (exa) <erayo@cs.bilkent.edu.tr>
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef Trie_Interface
#define Trie_Interface

#include <vector>
#include <iterator>
#include <iostream>

namespace Structure {

  using std::vector;
  //using std::iterator;
  using std::ostream;

  // Generic trie structure
  // String must have access to its first element via first()
  // and a function strip(), length() and blank()  must be defined on
  // it
  template <typename Symbol, typename String, typename Component>
  class Trie_Impl
  {
  public:
    typedef std::vector<Trie_Impl*> Subtries;

    // here is the interface of the function class we use
    //class Node_Function {
    //public:
    //	void operator() (Trie_Impl* node)
    //};

    // constructors
    Trie_Impl() : empty_comp(true) {}
    Trie_Impl(const Symbol & s) : symbol(s), empty_comp(true) {}
    Trie_Impl(const Symbol & s, const Component & c)
      : symbol(s), component(c), empty_comp(false) {}

    // destructor
    ~Trie_Impl()
    {
      for (typename Subtries::iterator i=children.begin();
	   i!=children.end(); i++) {
	delete *i;
      }
    }

    bool have_symbol(const String& string) const {
      return symbol == first(string);
    }

    Trie_Impl* create_trie(const String& string, const Component& component)
    {
//       cout << "create trie with '" << string << "' comp: "
// 	   << component << endl;
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
    Trie_Impl* insert(const String& string, const Component & component) {
//       cout << "insert '" << string << "' component: " << component << endl;
      if (empty(string))
	return 0;
      else if (have_symbol(string)) {

	// shortcut: we've already got it
	if (length(string)==1)
	  return this;

	String rest = string; strip(rest);
	// trie (he he) to insert to children
	// and if successful return true
	for (typename Subtries::iterator i=children.begin();
	     i!=children.end(); i++) {
	  Trie_Impl* subtrie;
	  if (subtrie=(*i)->insert(rest, component))
	    return subtrie;
	}
	// nobody has it, let's create that freaking subtrie
	// which consists of a single path
	children.push_back(create_trie(rest, component));
	return children.back()->query(rest);
      }
      else {
//	cout << "no symbol!" << " symbol:" << symbol << ", empty?: "
//	     << empty_comp << endl;
	return 0;
      }
    }

    Trie_Impl* query(const String& string) {
      assert(!empty(string));
//       cout << "query '" << string << "'" << endl;
      if (have_symbol(string)) {

// 	cout << "have symbol!" << endl;
	// we've found it
	if (length(string)==1) {
// 	  cout << "found it!" << endl;
	  return this;
	}

	String rest = string; strip(rest);

	//scan children to see if they have got it
	for (typename Subtries::iterator i=children.begin();
	     i!=children.end(); i++) {
	  if ((*i)->have_symbol(rest))
	    return (*i)->query(rest);
	}
      }
      else {
	// nobody's got it
// 	cout << "nobody has it!" << endl;
	return 0;
      }
	return 0;
    }

    bool remove(const String& string) {
      Subtries path;
      if (query_path(string, path)) {
	if (path.last().subtries.empty()) { // if we have no children
	  typename Subtries::iterator empty_subpath = path.end();
	  do {
	    empty_subpath--;
	  }
	  while (empty_subpath.subtries.empty());
	  delete *empty_subpath;
	  erase(empty_subpath, path.end());
	}
	path.last().empty = true;
      }
      else
	return false;
    }

    bool query_path(const String& string, Subtries& path) const {
      assert(!empty(string));
      path.push_back(this);
      if (have_symbol(string)) {

	// we've found it
	if (length(string)==1)
	  return true;

	String rest = string; strip(rest);

	//scan children to see if they have got it
	for (typename Subtries::iterator i=children.begin();
	     i!=children.end(); i++) {
	  if ((*i)->have_symbol(rest))
	    return (*i)->query(rest);
	}
      }
      else {
	// nobody's got it
	return false;
      }
    }

    unsigned int num_components() const {
      int count = 0;
      if (!empty_comp)
	count ++;
      if (!children.empty()) {
	for (typename Subtries::const_iterator i=children.begin();
	     i!=children.end(); i++) {
	  count += (*i)->num_components();
	}
      }
      return count;
    }

    // apply a function to each node
    template <typename Node_Fun>
    void apply(Node_Fun& fun) {
      for (typename Subtries::iterator i=children.begin();
	   i!=children.end(); i++) {
	if (!(*i)->empty_comp)
	  fun(*i);
      }
    }

    ostream& print(ostream& out)  const {
//       out << " [ " << " sym: " << symbol << " comp: " << component
// 	  << " empty?: " << empty_comp;
      for (typename Subtries::const_iterator i=children.begin();
	   i!=children.end(); i++) {
	(*i)->print(out);
      }
      out << " ] ";
      return out;
    }

    //protected:
    Symbol symbol;
    Component component;
    bool empty_comp;
    Subtries children;
  };


  template <typename Sy, typename St, typename C>
  ostream& operator << (ostream& out, const Trie_Impl<Sy,St,C>& trie)
  {
    return trie.print(out);
  }

  // This is trie interface
  template <typename Symbol, typename String, typename Component>
  class Trie : private Trie_Impl<Symbol, String, Component>
  {
  public:
    typedef Trie_Impl<Symbol, String, Component> Base;
    Trie() : Base(blank(String())) {}

    Component* query(String string) {
      Symbol symbol = blank(string);
      add_prefix(string, symbol);
      return &Base::query(string)->component;
    }

    Component* insert(const String & string, const Component & component)
    {
      Symbol symbol = blank(string);
      String str = string;
      add_prefix(str, symbol);
      return &Base::insert(str, component)->component;
    }

    Component* remove(const String & string, const Component & component)
    {
      Symbol symbol = blank(string);
      String str = string;
      add_prefix(string, symbol);
      return &Base::remove(string, component)->component;
    }

    unsigned int size() const {
      return num_components();
    }

  private:
  };

}

#endif
