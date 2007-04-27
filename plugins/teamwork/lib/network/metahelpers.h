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

#ifndef METAHELPERS_H
#define METAHELPERS_H

#include "statictree.h"

namespace Meta {
using namespace Tree;

///If the same type is in the list multiple times, wantOccurrences can be the index within the list of those types
template <class Type, class List, int wantOccurrences = 0, int previousCount = 0, int hadOccurrences = 0>
struct FindInList {
  enum {
    value = -1,
    occurrences = hadOccurrences
  };
};

template <class Type, int wantOccurrences, int previousCount, int hadOccurrences>
struct FindInList< Type, Type, wantOccurrences, previousCount, hadOccurrences> {
  enum {
    value = previousCount,
    occurrences = hadOccurrences + 1
  };
};

template <class Type, class Side1, class Side2, int wantOccurrences, int previousCount, int hadOccurrences>
struct FindInList< Type, Binder<Side1, Side2>, wantOccurrences, previousCount, hadOccurrences> {
  typedef FindInList<Type, Side1, wantOccurrences, previousCount, hadOccurrences> Side1Find;
  typedef typename If < Side1Find::value != -1 && Side1Find::occurrences - 1 == wantOccurrences, Side1Find, FindInList < Type, Side2, wantOccurrences, Count<Side1>::value + previousCount, hadOccurrences + Side1Find::occurrences > > ::Result EndFind;
  enum {
    value = EndFind::value,
    occurrences = EndFind::occurrences
  };
};


///Takes the absolute index of a type in the list, and converts it to an occurrence-index(only counting the same types)
template <class Type, class List, int wantNumber = 0, int previousCount = 0, int hadOccurrences = 0>
struct FindRelativeInList {
  enum {
    value = -1,
    occurrences = hadOccurrences,
    relativeIndex = occurrences - 1
  };
};

template <class Type, int wantNumber, int previousCount, int hadOccurrences>
struct FindRelativeInList< Type, Type, wantNumber, previousCount, hadOccurrences> {
  enum {
    value = previousCount,
    occurrences = hadOccurrences + 1,
    relativeIndex = occurrences - 1
  };
};

template <class Type, class Side1, class Side2, int wantNumber, int previousCount, int hadOccurrences>
struct FindRelativeInList< Type, Binder<Side1, Side2>, wantNumber, previousCount, hadOccurrences> {
  typedef FindRelativeInList<Type, Side1, wantNumber, previousCount, hadOccurrences> Side1Find;
  typedef typename If < Side1Find::value == wantNumber, Side1Find, FindRelativeInList < Type, Side2, wantNumber, Count<Side1>::value + previousCount, hadOccurrences + Side1Find::occurrences > > ::Result EndFind;
  enum {
    value = EndFind::value,
    occurrences = EndFind::occurrences,
    relativeIndex = occurrences - 1
  };
};

template <int a, int b>
struct AssertSame {
  typedef typename Assert < a == b > ::OK OK;
};

struct True {};
struct False{};

}
#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
