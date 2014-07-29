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

#include "waitforupdate.h"

using namespace KDevelop;

WaitForUpdate::WaitForUpdate() : m_dataMutex(QMutex::Recursive), m_ready(false)
{
}

void WaitForUpdate::updateReady(KDevelop::IndexedString /*url*/, KDevelop::ReferencedTopDUContext topContext) {
  QMutexLocker lock(&m_dataMutex);
  QMutexLocker lock2(&m_waitMutex);
  m_ready = true;
  m_topContext = topContext;
  m_wait.wakeAll();
}

#include "waitforupdate.moc"
