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

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/smartconverter.h>

#include "editorintegrator.h"

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

void EditorIntegratorStatic::insertLoadedDocument(KTextEditor::Document* doc)
{
  {
    QMutexLocker lock(mutex);

    if(documents.contains(IndexedString(doc->url().pathOrUrl())))
      return;
  }

  if (SmartInterface* smart = dynamic_cast<SmartInterface*>(doc)) {
    // Don't clear smart ranges on reload. They will be collapsed, and can be repositioned or deleted on the next parsing run.
    smart->setClearOnDocumentReload(false);
  }

  {
    QMutexLocker lock(mutex);

    documents.insert(IndexedString(doc->url().pathOrUrl()), doc);
  }

  emit documentLoaded(doc);
}

void EditorIntegratorStatic::documentUrlChanged(KTextEditor::Document* document)
{
  QMutexLocker lock(mutex);

  QMutableHashIterator<IndexedString, KTextEditor::Document*>  it = documents;
  while (it.hasNext()) {
    it.next();
    if (it.value() == document) {
      it.remove();
      documents.insert(IndexedString(document->url().pathOrUrl()), document);
      // TODO trigger reparsing??
      return;
    }
  }

  //kWarning() << "Document URL change - couldn't find corresponding document!" ;
}

void EditorIntegratorStatic::removeDocument( KTextEditor::Document* document )
{
  emit documentAboutToBeDeleted(document);
  
  //We lock the duchain for write because only that way we can
  //disable all editor-integrators and deconvert the contexts within one cycle
  DUChainWriteLocker lock( DUChain::lock() );
  
  {
    QMutexLocker lock(mutex);
  
    IndexedString url(document->url().pathOrUrl());
    if (documents.contains(url)) {
      KTextEditor::Document* d = documents[url];
  
      // Grab the smart mutex to make sure kdevelop is finished with this document.
      SmartInterface* smart = dynamic_cast<SmartInterface*>(d);
      QMutexLocker smartLock(smart ? smart->smartMutex() : 0);
  
      if (editorIntegrators.contains(document)) {
        foreach (EditorIntegrator* editor, editorIntegrators.values(document)) {
          editor->clearCurrentDocument();
        }
      }
  
      documents.remove(url);
    }
  }
  
  emit documentAboutToBeDeletedFinal(document);
}

void EditorIntegratorStatic::reloadDocument(KTextEditor::Document* document)
{
  emit documentAboutToBeReloaded(document);
}

}

#include "editorintegratorstatic.moc"

