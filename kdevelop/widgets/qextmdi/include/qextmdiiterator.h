//----------------------------------------------------------------------------
//    filename             : qextmdiiterator.h
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

#ifndef _QEXTMDIITERATOR_H_
#define _QEXTMDIITERATOR_H_


template<class Item>
class QextMdiIterator {
public:
   virtual ~QextMdiIterator() {}

   virtual void first() = 0;
   virtual void last() = 0;
   virtual void next() = 0;
   virtual void prev() = 0;
   virtual bool isDone() const = 0;
   virtual Item currentItem() const = 0;

protected:
   QextMdiIterator() {}
};

#endif // _QEXTMDIITERATOR_H_
