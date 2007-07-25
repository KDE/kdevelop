/***************************************************************************
  Copyright 2006 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** How typelists work:
 * Typelists are represented by the class Binder. That class is a container for only two types, but if one of
 * those is of type Binder<..,..> itself, the types contained by the binder are interpreted as members of the list.
 * That way lists off arbitrary size can be defined recursively.
 *
 * To make the whole thing look a little nicer, either the Append member of Binder(see below), or the BIND_LIST_X(..) macros can be used(see bottom).
* */

#ifndef BINDER_H
#define BINDER_H

#include "binder.h"


/**  This class is used to group multiple types together. They can be appended in a nicer looking manner
then recursion, Example:
Binder<Message1>::Append<Message2>::Append<Message3>::Result
Would be the same as Binder<Binder<Message1, Message2>, Message3 >
*/

///In the type-lists, the class Empty is counted as non-existent
class Empty {}
;
template <class Chain1, class Chain2 = Empty>
struct Binder {
  template <class NChain>
  struct Append {
    typedef Binder< Binder< Chain1, Chain2 >, NChain > Result;
  };
};

template <class Item1>
struct Binder<Empty, Item1> {
  typedef Item1 Result;
  template <class NChain>
  struct Append {
    typedef Binder< Item1, NChain > Result;
  };
};

template <class Item1>
struct Binder<Item1, Empty> {
  typedef Item1 Result;
  template <class NChain>
  struct Append {
    typedef Binder< Item1, NChain > Result;
  };

};

template <>
struct Binder<Empty, Empty> {
  typedef Empty Result;
  template <class NChain>
  struct Append {
    typedef Binder< NChain, Empty > Result;
  };
}
;

template <class Slave, class Parent = typename Slave::Precursor, unsigned char preferredSubId = Slave::preferredIndex>
class Chain {};


///This macro simplifies the syntax of defining a static type-list. The first parameter is the name of the new type, the other parameters are the types.
#define BIND_LIST_1( name, x1 ) typedef Binder<x1> name;
#define BIND_LIST_2( name, x1, x2 ) typedef Binder<x1>::Append<x2>::Result name;
#define BIND_LIST_3( name, x1, x2, x3 ) typedef Binder<x1>::Append<x2>::Result::Append<x3>::Result name;
#define BIND_LIST_4( name, x1, x2, x3, x4 ) typedef Binder<x1>::Append<x2>::Result::Append<x3>::Result::Append<x4>::Result name;
#define BIND_LIST_5( name, x1, x2, x3, x4, x5 ) typedef Binder<x1>::Append<x2>::Result::Append<x3>::Result::Append<x4>::Result::Append<x5>::Result name;
#define BIND_LIST_6( name, x1, x2, x3, x4, x5, x6 ) typedef Binder<x1>::Append<x2>::Result::Append<x3>::Result::Append<x4>::Result::Append<x5>::Result::Append<x6>::Result name;

#endif
