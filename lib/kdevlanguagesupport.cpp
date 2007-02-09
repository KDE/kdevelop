/* This file is part of the KDE project
   Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2002 F@lk Brettschneider <falkbr@kdevelop.org>
   Copyright (C) 2003 Alexander Dymo <adymo@kdevelop.org>
   Copyright (C) 2003 Amilcar do Carmo Lucas <amilcar@ida.ing.tu-bs.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "kdevlanguagesupport.h"

#include <QThread>

#include <kdevast.h>
#include <kdevdocument.h>

namespace KDevelop
{

LanguageSupport::LanguageSupport(const KComponentData &instance,
                                         QObject *parent)
    : IPlugin(instance, parent)
    , m_mutexMutex(new QMutex)
{
}

LanguageSupport::~LanguageSupport()
{
    delete m_mutexMutex;
}

bool LanguageSupport::supportsDocument( Document *document )
{
    KMimeType::Ptr mimetype = document->mimeType();
    return supportsDocumentInternal( mimetype );
}

bool LanguageSupport::supportsDocument( const KUrl &url )
{
    KMimeType::Ptr mimetype = KMimeType::findByUrl( url );
    return supportsDocumentInternal( mimetype );
}

bool LanguageSupport::supportsDocumentInternal( KMimeType::Ptr mimetype )
{
    QStringList mimeList = mimeTypes();
    QStringList::const_iterator it = mimeList.begin();
    //cache end(). it's faster when dealing with large lists
    QStringList::const_iterator itEnd = mimeList.end();
    for ( ; it != itEnd; it++ )
        if ( mimetype->is( *it ) )
            return true;
    return false;
}

void LanguageSupport::read( AST *ast, std::ifstream &in )
{
    Q_UNUSED( ast );
    Q_UNUSED( in );
    //Do Nothing
}

void LanguageSupport::write( AST *ast, std::ofstream &out )
{
    Q_UNUSED( ast );
    Q_UNUSED( out );
    //Do Nothing
}

CodeHighlighting *LanguageSupport::codeHighlighting() const
{
  // No default highlighting support
  return 0L;
}

void LanguageSupport::releaseAST( AST *)
{
  // FIXME make pure + implement in the kdev-pg parsers
}

QMutex * LanguageSupport::parseMutex(QThread * thread) const
{
  QMutexLocker lock(m_mutexMutex);

  if (!m_parseMutexes.contains(thread)) {
    connect(thread, SIGNAL(finished()), SLOT(threadFinished()));
    m_parseMutexes.insert(thread, new QMutex);
  }

  return m_parseMutexes[thread];
}

void LanguageSupport::lockAllParseMutexes()
{
  m_mutexMutex->lock();

  QList<QMutex*> waitForLock;

  // Grab the easy pickings first
  QHashIterator<QThread*, QMutex*> it = m_parseMutexes;
  while (it.hasNext()) {
    it.next();
    if (!it.value()->tryLock())
      waitForLock.append(it.value());
  }

  // Work through the stragglers
  foreach (QMutex* mutex, waitForLock)
    mutex->lock();
}

void LanguageSupport::unlockAllParseMutexes()
{
  QHashIterator<QThread*, QMutex*> it = m_parseMutexes;
  while (it.hasNext()) {
    it.next();
    it.value()->unlock();
  }

  m_mutexMutex->unlock();
}

void LanguageSupport::threadFinished()
{
  Q_ASSERT(sender());

  QMutexLocker lock(m_mutexMutex);

  QThread* thread = static_cast<QThread*>(sender());

  Q_ASSERT(m_parseMutexes.contains(thread));

  QMutex* mutex = m_parseMutexes[thread];
  mutex->unlock();
  delete mutex;
  m_parseMutexes.remove(thread);
}

void LanguageSupport::documentLoaded( AST *ast, const KUrl& document )
{
  Q_UNUSED(ast);
  Q_UNUSED(document);
}

}

#include "kdevlanguagesupport.moc"
