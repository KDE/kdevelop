/***************************************************************************
  copyright            : (C) 2006 by David Nolden
  email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** This include-file contains most of the meta-programming code used in this network-library.
 *  The biggest part of it is management of simple type-lists(like adding a type, removing a type, finding one, etc.)
 *  and building a static tree for message-dispatching. Unfortunately it's a bit hard to overview. :)
 */

#ifndef STATICTREE
#define STATICTREE

#include "binder.h"
#include <string>

typedef unsigned char uchar;
namespace Tree {

template <class Out>
void printId( Out& out, const uchar* const id ) {
  for ( uint i = 0; i < strlen( ( char* ) id ); ++i ) {
    if ( i )
      out << ".";
    out << ( int ) id[ i ];
  }
}

template < int Condition, class Then, class Else >
struct If {
  typedef Then Result;
};

template < class Then, class Else >
struct If <0, Then, Else> {
  typedef Else Result;
};

struct Fail {
  enum {
    value = 0
  };
};

struct Success {
  enum {
    value = 1
  };
};

template < class Compare1, class Compare2, class Then, class Else >
struct IfSame {
  typedef Else Result;
};

template < class Compare, class Then, class Else >
struct IfSame<Compare, Compare, Then, Else> {
  typedef Then Result;
};

template <class Chain>
struct Count {
  enum {
    value = 1
  };
};

template <>
struct Count<Empty> {
  enum {
    value = 0
  };
};

template <int Condition>
struct Assert {
  typedef Empty OK;
};

template <>
struct Assert<0> {}
;

template <class Side1, class Side2>
struct Count<Binder<Side1, Side2> > {
  enum {
    value = Count<Side1>::value + Count<Side2>::value
  };
};

template <int number, class Chain>
struct GetListItem {
  ///The item does not exist
};

template <class Side1, class Side2>
struct GetListItem<0, Binder<Side1, Side2> > {
  typedef typename GetListItem< 0, Side1>::Result Result;
};

template <class Item>
struct GetListItem<0, Item> {
  typedef Item Result;
};

template <int number, class Side1, class Side2>
struct GetListItem< number, Binder<Side1, Side2> > {
  ///If you get an error here which says that GetListItem<-1, ...> has no type named "Result",
  ///that probably means that the searched Item is not in the list.  For the CrossMap it means that
  ///a key-type was used which is not in the list.
  typedef typename If < ( Count<Side1>::value > number ), GetListItem< number, Side1>, GetListItem< number - Count<Side1>::value, Side2 > > ::Result::Result Result;
};





template <class Type, uchar preferredSubId>
class Entry {}
;

template <class Stuff, class Item>
struct GetSlaves {
  typedef Entry<Empty, 0> Result;
};

template <class Stuff, class Item>
struct GetParents {
  typedef Entry<Empty, 0> Result;
};

template <class Item1, class Item2>
struct AddResults {
  typedef Binder<Item1, Item2> Result;
};

template <class Item1>
struct AddResults<Empty, Item1> {
  typedef Item1 Result;
};

template <class Item1>
struct AddResults<Item1, Empty> {
  typedef Item1 Result;
};

template <>
struct AddResults<Empty, Empty> {
  typedef Empty Result;
};

template <class Item1, uchar a>
struct AddResults<Entry<Empty, a>, Item1> {
  typedef Item1 Result;
};

template <class Item1, uchar a>
struct AddResults<Item1, Entry<Empty, a> > {
  typedef Item1 Result;
};

template <uchar a, uchar b>
struct AddResults<Entry<Empty, a>, Entry<Empty, b> > {
  typedef Entry<Empty, 0> Result;
};

///TODO!
template <class Stuff>
struct SortResultByIndex {
  typedef Stuff Result;
};

template <class Chain1, class Chain2, class Item>
struct GetParents< Binder<Chain1, Chain2>, Item > {
  typedef typename AddResults< typename GetParents<Chain1, Item>::Result, typename GetParents<Chain2, Item>::Result >::Result Result;
};

template <class Chain1, class Chain2, class Item>
struct GetSlaves< Binder<Chain1, Chain2>, Item > {
  typedef typename AddResults< typename GetSlaves<Chain1, Item>::Result, typename GetSlaves<Chain2, Item>::Result >::Result Result;
};

template <class Item1, class Item, uchar preferredSubId>
struct GetSlaves< Chain< Item1, Item, preferredSubId >, Item> {
  typedef Entry<Item1, preferredSubId> Result;
};

template <class Item1, class Item, uchar preferredSubId>
struct GetParents< Chain< Item1, Item, preferredSubId >, Item1> {
  typedef Entry<Item, preferredSubId> Result;
};

template <class Container >
struct ExtractItem {
  typedef Container Result;
};

template <class Item, uchar preferredSubId>
struct ExtractItem< Entry< Item, preferredSubId > > {
  typedef Item Result;
};

template < class Side1, class Side2 >
struct ExtractItem < Binder< Side1, Side2 > > {
  typedef typename ExtractItem< Side1 >::Result Result;
};

template <class Slaves, class Stuff, class EntryTemplate, uchar currentNum >
struct TreeNodeSlaves {
  enum ID {
    id = 0
  };
  template <class Action>
  inline uchar find( const uchar /*c*/, const uchar*const /*id*/, Action& /*act*/ ) const {
    return 0;
  }
  template < class Out >
  inline void print( Out& out, std::string prefix ) {
    out << prefix + "deadnode1\n";
  }
};

template <class Item1, class Item2>
struct BindTogether {
  typedef Binder<Item1, Item2> Result;
};

template < class Item >
struct BindTogether<Empty, Item> {
  typedef Item Result;
};
template < class Item >
struct BindTogether<Item, Empty> {
  typedef Item Result;
};

template <>
struct BindTogether<Empty, Empty> {
  typedef Empty Result;
};

///Action must have a member-template called Action, that has a typedef called Result. Action will be applied to each item, and the result bound together.
template<class Action, class List>
struct ForEachInList {
  typedef typename Action::template Action<List>::Result Result;
};

template<class Action, class List1, class List2>
struct ForEachInList< Action, Binder<List1, List2> > {
  typedef typename BindTogether< typename ForEachInList< Action, List1 >::Result, typename ForEachInList< Action, List2 >::Result >::Result Result;
};

template <class Item>
struct IsEmpty {
  enum {
    result = 0
  };
};


template <>
struct IsEmpty<Empty> {
  enum {
    result = 1
  };
};

template <>
struct IsEmpty<Binder<Empty, Empty> > {
  enum {
    result = 1
  };
};

template <class Item>
struct Weight {
  enum {
    Result = 1
  };
};

template <class Left, class Right>
struct Weight<Binder<Left, Right> > {
  enum {
    Result = Weight<Left>::Result + Weight<Right>::Result
  };
};

template <class Item>
struct Balance {
  enum {
    RightOverweight = 0,
    LeftOverweight = 0
  };
};

template <class Left, class Right>
struct Balance<Binder<Left, Right> > {
  enum {
    RightOverweight = Weight<Left>::Result < Weight<Right>::Result - 1 ? 1 : 0,
    LeftOverweight = Weight<Left>::Result > Weight<Right>::Result + 1 ? 1 : 0
  };
};


template <class From>
struct RemoveOneLeft {
  typedef Empty Result;
};

template <class Left, class Right>
struct RemoveOneLeft<Binder<Left, Right> > {
  typedef typename RemoveOneLeft<Left>::Result LeftRemoved;
  typedef typename IfSame<LeftRemoved, Empty, Right, typename BindTogether< LeftRemoved, Right >::Result >::Result Result;
};

template <class From>
struct GetOneLeft {
  typedef From Result;
};

template <class Left, class Right>
struct GetOneLeft<Binder<Left, Right> > {
  typedef typename GetOneLeft<Left>::Result Result;
};

template <class From>
struct RemoveOneRight {
  typedef Empty Result;
};

template <class Left, class Right>
struct RemoveOneRight<Binder<Left, Right> > {
  typedef typename RemoveOneRight<Right>::Result RightRemoved;
  typedef typename IfSame<RightRemoved, Empty, Left, typename BindTogether< Left, RightRemoved >::Result >::Result Result;
};

template <class From>
struct GetOneRight {
  typedef From Result;
};

template <class Left, class Right>
struct GetOneRight<Binder<Left, Right> > {
  typedef typename GetOneRight<Right>::Result Result;
};

template <class Into, class Item>
struct InsertOneLeft {
  typedef Binder<Item, Into> Result;
};

template <class Left, class Right, class Item>
struct InsertOneLeft< Binder<Left, Right>, Item> {
  typedef Binder<InsertOneLeft<Left, Item>, Right> Result;
};

template <class Into, class Item>
struct InsertOneRight {
  typedef Binder<Into, Item> Result;
};

template <class Left, class Right, class Item>
struct InsertOneRight< Binder<Left, Right>, Item> {
  typedef Binder<Left, InsertOneRight<Right, Item> > Result;
};

template <class Left, class Right, int LeftOverweight, int RightOverweight>
struct BalanceNow {
  typedef Binder<Left, Right> Result;
};

template <class Left, class Right>
struct BalanceNow<Left, Right, 1, 0> {  ///Take one element from left to right side, and recurse
  typedef typename RemoveOneRight<Left>::Result NewLeft;
  typedef typename InsertOneLeft<Right, GetOneRight<Left> >::Result NewRight;
  typedef typename BalanceNow< NewLeft, NewRight, Balance< Binder<NewLeft, NewRight> >::LeftOverweight, Balance< Binder<NewLeft, NewRight> >::RightOverweight >::Result Result;
};

template <class Left, class Right>
struct BalanceNow<Left, Right, 0, 1> {  ///Take one element from left to right side, and recurse
  typedef typename InsertOneRight<Left, GetOneLeft<Right> >::Result NewLeft;
  typedef typename RemoveOneLeft<Right>::Result NewRight;
  typedef typename BalanceNow< NewLeft, NewRight, Balance< Binder<NewLeft, NewRight> >::LeftOverweight, Balance< Binder<NewLeft, NewRight> >::RightOverweight >::Result Result;
};


template <class Stuff>
struct Rebalance {
  typedef Stuff Result;
};

template <class Left, class Right>
struct Rebalance<Binder<Left, Right> > {
  typedef typename BalanceNow<Left, Right, Balance< Binder<Left, Right> >::LeftOverweight, Balance< Binder<Left, Right> >::RightOverweight >::Result Result;
};

template <class Item>
struct Consider {}
;

class TreeNodeDataInterface {
  public:
};

template <class Type, class EntryTemplate>
class TreeNodeData : public TreeNodeDataInterface, public EntryTemplate::template Entry <Type> {
  public:
  TreeNodeData() {}
}
;

template < class Stuff, uchar cnum, uchar preferredSubId , class EntryTemplate>
struct TreeNodeSlaves<Consider<Entry<Empty, preferredSubId> >, Stuff, EntryTemplate, cnum > {
  enum ID {
    id = 0
  };
  template <class Action>
  inline uchar find( const uchar /*c*/, const uchar*const /*id*/, Action& /*act*/ ) const {
    return 0;
  }

  template < class Out >
  inline void print( Out& /*out*/, std::string /*prefix*/ ) {
  }

  inline void buildFullId( const std::string& /*pre*/ ) {
  }
};

template < class Stuff, uchar cnum, class EntryTemplate >
struct TreeNodeSlaves<Consider<Empty >, Stuff, EntryTemplate, cnum > {
  enum ID {
    id = 0
  };
  template <class Action>
  uchar find( const uchar /*c*/, const uchar*const /*id*/, Action& /*act*/ ) const {
    return 0;
  }
  template < class Out >
  inline void print( Out& /*out*/, std::string /*prefix*/ ) {
  }

  inline void buildFullId( const std::string& /*pre*/ ) {
  }
};

template <class Stuff>
struct SortByIndex {
  typedef Stuff Result;
};

template <class Item>
struct EntryIndex {
  ///The argument-type is wrong
};

template <class Type, uchar index>
struct EntryIndex< Entry<Type, index> > {
  enum Result {
    result = index
  };
};

template <int w>
struct StaticAssert {
  typedef Empty Ok;
};

template <>
struct StaticAssert<0> {}
;


template <class Left, class Right>
struct Merge {
  typedef typename GetOneLeft<Left>::Result LeftFirst;
  typedef typename GetOneLeft<Right>::Result RightFirst;

  typedef typename RemoveOneLeft<Left>::Result LeftTail;
  typedef typename RemoveOneLeft<Right>::Result RightTail;

  typedef typename StaticAssert < ( int ) EntryIndex<LeftFirst>::result != ( int ) EntryIndex<RightFirst>::result > ::Ok notSame;

  //typedef typename If<typename IsEmpty<LeftFirst>::Result, Right, typename If<typename IsEmpty<RightFirst>::Result, Left, typename MergeImpl<LeftFirst, RightFirst, LeftTail, RightTail>::Result >::Result>::Result Result;
  typedef typename If < ( int ) EntryIndex<LeftFirst>::result <= ( int ) EntryIndex<RightFirst>::result,
  Binder< LeftFirst, typename Merge< LeftTail, Binder<RightFirst, RightTail> >::Result >,
  Binder< RightFirst, typename Merge< Binder<LeftFirst, LeftTail>, RightTail >::Result > > ::Result Result;
};

template <class Only>
struct Merge<Only, Empty> {
  typedef Only Result;
};

template <class Only>
struct Merge<Empty, Only> {
  typedef Only Result;
};


template <class Left, class Right>
struct Merge<Binder<Empty, Left>, Right> {
  typedef typename Merge<Left, Right>::Result Result;
};

template <class Left, class Right>
struct Merge<Binder<Empty, Left>, Binder<Empty, Right> > {
  typedef typename Merge<Left, Right>::Result Result;
};

template <class Left, class Right>
struct Merge<Left, Binder<Empty, Right> > {
  typedef typename Merge<Left, Right>::Result Result;
};

template <class Left, class Right>
struct SortByIndex<Binder<Left, Right> > {
  typedef typename Merge< typename SortByIndex<Left>::Result, typename SortByIndex<Right>::Result >::Result Result;
};

template <class CurrentType, class Stuff, class EntryTemplate>
class TreeNode : public TreeNodeData<CurrentType, EntryTemplate> {
    //typedef TreeNode<CurrentType, Stuff> TreeNodeType;
    typedef TreeNodeSlaves< Consider< typename SortByIndex< typename GetSlaves<Stuff, CurrentType>::Result>::Result > , Stuff, EntryTemplate, 1 > Slaves;

    Slaves slaves;

    ///This walks the tree until it cannot go on, then applies the given Action to the current node(additionally giving the type at the node as template-argument), and gives back the return-type(Action::ReturnType).
  public:

    inline void buildFullId( const std::string& pre ) {
      slaves.buildFullId( pre );
    }

    template < class Out >
    void print( Out& out, std::string prefix = "" ) {
      //out << prefix + " " << typeid( CurrentType ).name() << "\n";
      out << "no rtti";
      slaves.template print<Out>( out, prefix );
    }

    ///Searches the tree. Returns 0 if the action could be applied to no node, 1 if the action was applied but not to the searched leaf, and 2 if it was applied to the exact searched target
    template <class Action>
    inline uchar find( const uchar*const id, Action& act ) const {
      const uchar c = id[ 0 ];
      if ( c ) {
        uchar r = slaves.Slaves::template find<Action>
        ( c, id + 1, act );
        if ( !r ) {
          act.template operator() <CurrentType> ( *this );
          ;
          return 1;
        } else {
          return r;
        }
      } else {
        act.template operator() <CurrentType> ( *this );
        ;
        return 2;
      }
    }
};

struct StandardEntryTemplate {
  template <class Type>
  class Entry {}
  ;
};

template < class Class >
struct GetBase {
  typedef typename Class::Precursor Result;
};

struct All {}
;

template <class Stuff>
struct ExtractChains {
  typedef Chain< Stuff, typename GetBase< Stuff >::Result, Stuff::preferredIndex> Result;
};

template < class Side1, class Side2 >
struct ExtractChains< Binder< Side1, Side2 > > {
  typedef typename BindTogether< typename ExtractChains< Side1 >::Result, typename ExtractChains<Side2>::Result >::Result Result;
};

template < class Side2 >
struct ExtractChains< Binder< Empty, Side2 > > {
  typedef typename ExtractChains<Side2>::Result Result;
};

template < class Side2 >
struct ExtractChains< Binder< Side2, Empty > > {
  typedef typename ExtractChains<Side2>::Result Result;
};

template <class A, class B, uchar C>
struct ExtractChains< Chain< A, B, C > > {
  typedef Chain< A, B, C > Result;
};


template < class Item, class Master, class Stuff >
struct WhatIsMissing {
  typedef typename Item::Template_Error_Wrong_Input_Should_Be_Chain Error;
};

template <class Stuff, class Item>
struct Contains {
  enum {
    Result = 0
  };
};

template < class Item, class Parent, uchar C >
struct Contains< Chain< Item, Parent, C >, Item > {
  enum {
    Result = 1
  };
};

template <class Side1, class Side2, class Item>
struct Contains< Binder< Side1, Side2 >, Item > {
  enum {
    Result = Contains< Side1, Item >::Result | Contains< Side2, Item> :: Result
  };
};


template < class Item, class Parent, uchar C, class Master, class Stuff>
struct WhatIsMissing< Chain< Item, Parent, C >, Master, Stuff > {
  enum {
    Result = !Contains< Stuff, Parent >::Result
  };
  typedef typename If< Result, Parent, Empty >::Result Items;
};

template < class Item, uchar C, class Master, class Stuff>
struct WhatIsMissing< Chain< Item, Master, C >, Master, Stuff > {
  enum {
    Result = 0
  };
  typedef Empty Items;
};

template < class Side1, class Side2, class Master, class Stuff >
struct WhatIsMissing< Binder< Side1, Side2 > , Master, Stuff > {
  enum {
    Result = WhatIsMissing<Side1, Master, Stuff>::Result | WhatIsMissing<Side2, Master, Stuff>::Result
  };

  ///Only return one by one, because multiple items might be missing the same parent and we don't want them multiple times
  typedef typename If< WhatIsMissing<Side1, Master, Stuff>::Result, typename WhatIsMissing<Side1, Master, Stuff>::Items, typename WhatIsMissing<Side2, Master, Stuff>::Items >::Result Items;
  //typedef BindTogether< typename WhatIsMissing<Side1, Master, Stuff>::Items, WhatIsMissing<Side2, Master, Stuff>::Items >::Items Items;
};

template <class Stuff, class Master, class Missing = typename WhatIsMissing<Stuff, Master, Stuff>::Items >
struct MakeCompleteImpl {
  typedef typename MakeCompleteImpl< typename BindTogether< Stuff, typename ExtractChains< Missing >::Result >::Result , Master >::Result Result;
};

template <class Stuff, class Master >
struct MakeCompleteImpl< Stuff, Master, Empty > {
  typedef Stuff Result;
};

///Makes sure that the chains for all necessary parent-nodes are available
template < class Stuff, class Master >
struct MakeComplete {
  typedef typename MakeCompleteImpl< Stuff, Master >::Result Result;
};

/**Stuff is the list of types that define the tree
in form of multiple Chain<Type, Parent-Type, int preferredIndex>, and EntryTemplate is a class which embeds
a template-class called "Entry" which will be inherited by every single node, with the current type of the node as template-parameter(See StandardEntryTemplate).
Instead of using the explicit Chain<...>-notation, just a type can be used, and the chain will be extracted by using the type's Precursor"-Typedef, and it's preferredIndex constant/enum. Every item MUST have an explicit sub-id, the automatic generation of ID's was abandoned to make building partial trees easiert.
Stuff is a set of "Chain"'s or Types, bound together with "Binder". It can be incomplete, only the important leafs have to be there, and the tree leading to them is extracted automatically.
 */
template < class Master, class Stuff, class EntryTemplate = StandardEntryTemplate >
struct TreeMasterNode : public TreeNode < Master, /* typename Rebalance<*/ typename MakeComplete< typename ExtractChains< Stuff >::Result, Master >::Result /*>::Result*/, EntryTemplate > {
  typedef TreeNode < Master,  /*typename Rebalance<*/ typename MakeComplete< typename ExtractChains< Stuff >::Result, Master >::Result /*>::Result*/, EntryTemplate > Base;

  template < class Out >
  void print( Out& out ) {
    Base::buildFullId( "" );
    Base::print( out );
  }

  template <class Action>
  uchar find( const uchar*const id, Action& act ) const {
    return Base::find( id, act );
  }

};
/*
  template< class Item, class Stuff, class EndItem, int real >
  struct IsOnWay {
  };*/

template <class Type>
struct GetResultValue {
  enum {
    Result = Type::Result
  };
};

template <class Item>
struct BaseItem {
  typedef Empty Result;
};

template <class Item1, class Item2>
struct BaseItem< Binder< Item1, Item2 > > {
  typedef typename BaseItem<Item1>::Result Result;
};

template <class Base, class Parent, uchar preferredSubId>
struct BaseItem< Chain< Base, Parent, preferredSubId > > {
  typedef Base Result;
};


/*  template<class Item>
  struct GetParentHighLevel {
    typedef typename BaseItem<typename GetParents< typename MakeComplete< typename ExtractChains< Stuff >::Result >::Result>::Result>::Result  Result;
};*/

template < class Item, class Stuff, class EndItem >
struct IsOnWay {
  enum {
    Result = GetResultValue< IsOnWay< Item, Stuff, typename GetParents< Stuff, EndItem >::Result > >::Result
  };
};

template < class Item, class Stuff >
struct IsOnWay< Item, Stuff, Item > {
  enum {
    Result = 1
  };
};

template < class Item, class Stuff >
struct IsOnWay < Item, Stuff, Empty > {
  enum {
    Result = 0
  };
};

template < class Item, class Stuff, class Leaf >
struct IsNecessary {
  enum {
    Result = IsOnWay< Item, Stuff, Leaf>::Result
  };
};

template < class Item, class Stuff >
struct IsNecessary<Item, Stuff, All> {
  enum {
    Result = 1
  };
};

template < class Item, class Stuff, class Side1, class Side2 >
struct IsNecessary<Item, Stuff, Binder< Side1, Side2 > > {
  enum {
    Result = IsNecessary< Item, Stuff, Side1 >::Result | IsNecessary< Item, Stuff, Side2 >::Result
  };
};

template <int necessary, class SlaveType, class Stuff, uchar id_, uchar preferredSubId, class EntryTemplate>
struct TreeSlaveData {
  enum ID {
    id = id_ > preferredSubId ? id_ : preferredSubId
  };
  std::string fullId;
  TreeNode<SlaveType, Stuff, EntryTemplate> slave;

  TreeSlaveData() {}

  template < class Out >
  inline void print( Out& out, std::string prefix ) {
    prefix += "-";
    out << prefix + " ";
    if ( !fullId.empty() )
      printId( out, ( uchar* ) & fullId[ 0 ] );
    else
      out << "invalid-id";
    out << "\n";
    slave.print( out, prefix );
  }

  inline void buildFullId( const std::string& pre ) {
    fullId = pre;
    fullId.push_back( id );
    slave.buildFullId( fullId );
  }
};

/// It would be useful not to create this type at all, but currently it must be done for numbering-consistency
template <class SlaveType, class Stuff, uchar id_, uchar preferredSubId, class EntryTemplate>
struct TreeSlaveData< 0, SlaveType, Stuff, id_, preferredSubId, EntryTemplate> {
  enum {
    id = id_ > preferredSubId ? id_ : preferredSubId
  };

  inline void buildFullId( const std::string& pre ) {}

  ///dummies:
  struct DummySlave {
    template <class Action>
    inline uchar find( const uchar*const /*id*/, Action& /*act*/ ) const {
      return 0;
    }
  };
  DummySlave slave;

  template < class Out >
  inline void print( Out& out, std::string prefix ) {}

}
;

///IsNecessary<Slave, Stuff, ExtractLeafs>::Result
template <class Slave, class Stuff, uchar cnum, uchar preferredSubId, class EntryTemplate>
struct TreeNodeSlaves< Consider<Entry<Slave, preferredSubId> >, Stuff, EntryTemplate, cnum> : public TreeSlaveData<1, Slave, Stuff, cnum, preferredSubId, EntryTemplate> {
  typedef TreeSlaveData<1, Slave, Stuff, cnum, preferredSubId, EntryTemplate> Base;
  inline void buildFullId( const std::string& pre ) {
    Base::buildFullId( pre );
  }

  template <class Action>
  inline uchar find( const uchar c, const uchar*const id, Action& act ) const {
    if ( c == Base::id ) {
      return Base::slave.template find<Action>( id, act );
    } else {
      return 0;
    }
  }

  template < class Out >
  inline void print( Out& out, std::string prefix ) {
    Base::print( out, prefix );
  }
};

template <class Slave1, class Slave2, class Stuff, uchar cnum, class EntryTemplate>
struct TreeNodeSlaves< Consider<Binder<Slave1, Slave2> >, Stuff, EntryTemplate, cnum> : private TreeNodeSlaves<Consider<Slave1>, Stuff, EntryTemplate, cnum>, private TreeNodeSlaves < Consider<Slave2>, Stuff, EntryTemplate, TreeNodeSlaves<Consider<Slave1>, Stuff, EntryTemplate, cnum>::id + 1 > {
  typedef TreeNodeSlaves<Consider<Slave1>, Stuff, EntryTemplate, cnum> Side1;
  typedef TreeNodeSlaves < Consider<Slave2>, Stuff, EntryTemplate, Side1::id + 1 > Side2;
  enum {
    begin = cnum,
    middle = Side1::id,
    id = Side2::id
  };

  inline void buildFullId( const std::string& pre ) {
    Side1::buildFullId( pre );
    Side2::buildFullId( pre );
  };

  template <class Action>
  inline uchar find( const uchar c, const uchar*const id, Action& act ) const {
    if ( c > middle ) {
      return Side2::find( c, id, act );
    } else {
      return Side1::find( c, id, act );
    }
  }
  template < class Out >
  inline void print( Out& out, std::string prefix ) {
    Side1::print( out, prefix );
    Side2::print( out, prefix );
  }
};
}

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
