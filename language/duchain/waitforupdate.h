/*
   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef KDEVPLATFORM_WAITFORUPDATE_H
#define KDEVPLATFORM_WAITFORUPDATE_H

#include "duchain/topducontext.h"

#include <serialization/indexedstring.h>

#include <QtCore/QObject>
#include <QMutex>
#include <QWaitCondition>

namespace KDevelop {

struct WaitForUpdate : public QObject {
  Q_OBJECT
  public slots:
    
    void updateReady(KDevelop::IndexedString url, KDevelop::ReferencedTopDUContext topContext);

  public:
    WaitForUpdate();
    
    QMutex m_dataMutex;
    QMutex m_waitMutex;
    QWaitCondition m_wait;
    bool m_ready;
    KDevelop::IndexedString m_url;
    KDevelop::ReferencedTopDUContext m_topContext;
};

}

#endif // KDEVPLATFORM_WAITFORUPDATE_H
