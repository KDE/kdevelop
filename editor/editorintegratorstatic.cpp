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
  Document* doc = qobject_cast<Document*>(sender());
  if (!doc) {
    kWarning() << "Unexpected non-document sender called this slot!" ;
    return;
  }

  {
    QMutexLocker lock(mutex);

    documents.insert(HashedString(doc->url().prettyUrl()), doc);
  }
}

void EditorIntegratorStatic::documentUrlChanged(KTextEditor::Document* document)
{
  QMutexLocker lock(mutex);

  QMutableHashIterator<HashedString, Document*>  it = documents;
  while (it.hasNext()) {
    it.next();
    if (it.value() == document) {
      it.remove();
      documents.insert(document->url().prettyUrl(), document);
      // TODO trigger reparsing??
      return;
    }
  }

  //kWarning() << "Document URL change - couldn't find corresponding document!" ;
}
void EditorIntegratorStatic::removeDocument( KTextEditor::Document* document )
{
  QMutexLocker lock(mutex);

  documents.remove(document->url().prettyUrl());

  lock.unlock();
  
  emit documentAboutToBeDeleted(document);
}

}

#include "editorintegratorstatic.moc"

