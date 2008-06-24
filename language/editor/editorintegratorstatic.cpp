/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2006 Hamish Rodda <rodda@kde.org>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "editorintegratorstatic.h"

#include <QtCore/QMutex>
#include <QtCore/QCoreApplication>

#include <ktexteditor/smartrange.h>
#include <ktexteditor/document.h>
#include <ktexteditor/smartinterface.h>

using namespace KTextEditor;

namespace KDevelop
{

EditorIntegratorStatic::EditorIntegratorStatic()
  : mutex(new QMutex)
{
  // This object must live on the main thread for the application.
  if (thread() != QCoreApplication::instance()->thread()) {
    moveToThread(QCoreApplication::instance()->thread());
  }
}

EditorIntegratorStatic::~EditorIntegratorStatic()
{
  delete mutex;
}

void EditorIntegratorStatic::documentLoaded()
{
  KTextEditor::Document* doc = qobject_cast<KTextEditor::Document*>(sender());
  Q_ASSERT(doc);
  
  DocumentInfo i;
  i.document = doc;
  i.revision = -1;

  if (SmartInterface* smart = dynamic_cast<SmartInterface*>(doc)) {
    // Don't clear smart ranges on reload. They will be collapsed, and can be repositioned or deleted on the next parsing run.
    smart->setClearOnDocumentReload(false);
    i.revision = smart->currentRevision();
    // Don't use revision 0, we don't want it (it's pre-loading from disk)
    if (i.revision == 0) {
      i.revision = -1;
      smart->releaseRevision(0);
    }
  }

  disconnect(doc, SIGNAL(completed()), this, SLOT(documentLoaded()));
  disconnect(doc, SIGNAL(textChanged(KTextEditor::Document*)), this, SLOT(documentLoaded()));

  {
    QMutexLocker lock(mutex);

    documents.insert(IndexedString(doc->url().pathOrUrl()), i);
  }
}

void EditorIntegratorStatic::documentUrlChanged(KTextEditor::Document* document)
{
  QMutexLocker lock(mutex);

  QMutableHashIterator<IndexedString, DocumentInfo>  it = documents;
  while (it.hasNext()) {
    it.next();
    if (it.value().document == document) {
      DocumentInfo i = it.value();
      it.remove();
      documents.insert(IndexedString(document->url().pathOrUrl()), i);
      // TODO trigger reparsing??
      return;
    }
  }

  //kWarning() << "Document URL change - couldn't find corresponding document!" ;
}

void EditorIntegratorStatic::removeDocument( KTextEditor::Document* document )
{
  QMutexLocker lock(mutex);

  IndexedString url(document->url().pathOrUrl());
  if (documents.contains(url)) {
    DocumentInfo i = documents[url];
    if (i.revision != -1)
      if (SmartInterface* smart = dynamic_cast<SmartInterface*>(i.document))
        smart->releaseRevision(i.revision);

    documents.remove(url);
  }

  lock.unlock();
  
  emit documentAboutToBeDeleted(document);
}

void EditorIntegratorStatic::reloadDocument(KTextEditor::Document* document)
{
  emit documentAboutToBeReloaded(document);
}

}

#include "editorintegratorstatic.moc"

