//----------------------------------------------------------------------------
//    filename             : qextmdinulliterator.h
//----------------------------------------------------------------------------
//    Project              : Qt MDI extension
//
//    begin                : 02/2000       by Massimo Morin
//    changes              : 02/2000       by Falk Brettschneider to create an
//                           - 06/2000     stand-alone Qt extension set of
//                                         classes and a Qt-based library
//
//    copyright            : (C) 1999-2000 by Massimo Morin (mmorin@schedsys.com)
//                                         and
//                                         Falk Brettschneider
//    email                :  gigafalk@yahoo.com (Falk Brettschneider)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Library General Public License as
//    published by the Free Software Foundation; either version 2 of the
//    License, or (at your option) any later version.
//
//----------------------------------------------------------------------------

#ifndef _QEXTMDINULLITERATOR_H_
#define _QEXTMDINULLITERATOR_H_

#include "qextmdiiterator.h"

template <class Item>
class QextMdiNullIterator : public QextMdiIterator<Item> {
public:
   QextMdiNullIterator() {};
   virtual void first() {}
   virtual void last() {}
   virtual void next() {}
   virtual void prev() {}
   virtual bool isDone() const { return TRUE; }
   virtual Item currentItem() const {
   /* should really never go inside here */
   return 0;
   }
};

#endif // _QEXTMDINULLITERATOR_H_
