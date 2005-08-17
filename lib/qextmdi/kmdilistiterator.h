//----------------------------------------------------------------------------
//    filename             : kmdilistiterator.h
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

#ifndef _KMDILISTITERATOR_H_
#define _KMDILISTITERATOR_H_

#include <kmdiiterator.h>
//Added by qt3to4:
#include <Q3PtrList>

template<class I> class Q3PtrList;
template<class I> class Q3PtrListIterator;

template <class Item>
class KMdiListIterator : public KMdiIterator<Item*> {
public:
   KMdiListIterator(Q3PtrList<Item>& list) {
      m_iterator = new Q3PtrListIterator<Item>(list);
   }

   virtual void first() {
      m_iterator->toFirst();
   }
   virtual void last() {
      m_iterator->toLast();
   }
   virtual void next()  { ++(*m_iterator); }
   virtual void prev()  { --(*m_iterator); }
   virtual bool isDone() const { return m_iterator->current() == NULL; }
   virtual Item* currentItem() const { return m_iterator->current(); }

   virtual ~KMdiListIterator() {
      delete m_iterator;
   }
  
private:
   Q3PtrListIterator<Item> *m_iterator;
};

#endif // _KMDILISTITERATOR_H_
