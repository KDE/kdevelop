//----------------------------------------------------------------------------
//    filename             : kmdiiterator.h
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 02/2000       by Massimo Morin
//    changes              : 02/2000       by Falk Brettschneider to create an
//                           - 06/2000     stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                           2000-2003     maintained by the KDevelop project
//
//    copyright            : (C) 1999-2003 by Massimo Morin (mmorin@schedsys.com)
//                                         and
//                                         Falk Brettschneider
//    email                :  falkbr@kdevelop.org (Falk Brettschneider)
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

#ifndef _KMDIITERATOR_H_
#define _KMDIITERATOR_H_


template<class Item>
class KMdiIterator {
public:
   virtual ~KMdiIterator() {}

   virtual void first() = 0;
   virtual void last() = 0;
   virtual void next() = 0;
   virtual void prev() = 0;
   virtual bool isDone() const = 0;
   virtual Item currentItem() const = 0;

protected:
   KMdiIterator() {}
};

#endif // _KMDIITERATOR_H_
