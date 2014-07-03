/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_KSHAREDOBJECT_H
#define KDEVPLATFORM_KSHAREDOBJECT_H

#include <QExplicitlySharedDataPointer>
#include <QtCore/QObject>

/**
 * Wrapper around QSharedDatafor use with QExplicitlySharedDataPointer when the object is based on QObject as well.
 * Instead of deleting the object once the reference-count reaches zero, QObject::deleteLater() is called.
 * This prevents a possible crash when the reference-count reaches zero during event-processing while the queue
 * contains events to be delivered to that object.
 *
 * Notice however that the object will not be deleted immediately, which may lead to unintended behavior.
 */

namespace KDevelop {

struct FakeAtomic {
    inline FakeAtomic(QObject& object, QSharedData& real) : m_object(object), m_real(real) {
    }
    inline operator int() const {
      const int value = m_real.ref.loadAcquire();
      if(value == 0)
          return 1; //Always return true, because we handle the deleting by ourself using deleteLater
      return value;
    }
    
    inline bool ref() {
      return m_real.ref.ref();
    }
    
    inline bool deref() {
      bool ret = m_real.ref.deref();
      if(!ret)
        m_object.deleteLater();

      return true; //Always return true, because we handle the deleting by ourself
    }

    inline int load() const {
        return m_real.ref.load();
    }
    
    QObject& m_object;
    QSharedData& m_real;
};

struct KSharedObject : public QSharedData {
  inline KSharedObject(QObject& object) : ref(object, *this) {
  }
  
  mutable FakeAtomic ref;
};

}

#endif
