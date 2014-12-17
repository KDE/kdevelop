/*
* This file is part of KDevelop
*
* Copyright 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Library General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include "urlparselock.h"

#include <QThread>
#include <QHash>

using namespace KDevelop;

namespace
{
///Facilities to prevent multiple parse-jobs from processing the same url.
QMutex urlParseMutex;
QHash<IndexedString, QPair<QThread*, uint> > parsingUrls;
}

UrlParseLock::UrlParseLock(const IndexedString& url)
    : m_url(url)
{
  QMutexLocker lock(&urlParseMutex);
  while(parsingUrls.contains(m_url) && parsingUrls[m_url].first != QThread::currentThread()) {
    //Wait here until no other thread is updating parsing the url
    lock.unlock();
    QThread::sleep(1);
    lock.relock();
  }
  if(parsingUrls.contains(m_url))
    ++parsingUrls[m_url].second;
  else
    parsingUrls.insert(m_url, qMakePair(QThread::currentThread(), 1u));
}

UrlParseLock::~UrlParseLock()
{
  QMutexLocker lock(&urlParseMutex);
  Q_ASSERT(parsingUrls.contains(m_url));
  Q_ASSERT(parsingUrls[m_url].first == QThread::currentThread());
  --parsingUrls[m_url].second;
  if(parsingUrls[m_url].second == 0)
    parsingUrls.remove(m_url);
}
