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
  QHashIterator<KUrl, QVector<KTextEditor::Range*> > it = topRanges;
  while (it.hasNext()) {
    it.next();
    foreach (KTextEditor::Range* range, it.value())
      if (range && range->isSmartRange())
        range->toSmartRange()->removeWatcher(this);
  }

  delete mutex;
}

void EditorIntegratorStatic::documentLoaded()
{
  Document* doc = qobject_cast<Document*>(sender());
  if (!doc) {
    kWarning() << k_funcinfo << "Unexpected non-document sender called this slot!" ;
    return;
  }

  {
    QMutexLocker lock(mutex);

    documents.insert(doc->url(), doc);
  }
}

void EditorIntegratorStatic::documentUrlChanged(KTextEditor::Document* document)
{
  QMutexLocker lock(mutex);

  QMutableHashIterator<KUrl, Document*>  it = documents;
  while (it.hasNext()) {
    it.next();
    if (it.value() == document) {
      if (topRanges.contains(it.key())) {
        kDebug(9506) << k_funcinfo << "Document URL change - found corresponding document";
        topRanges.insert(document->url(), topRanges.take(it.key()));
      }

      it.remove();
      documents.insert(document->url(), document);
      // TODO trigger reparsing??
      return;
    }
  }

  //kWarning() << k_funcinfo << "Document URL change - couldn't find corresponding document!" ;
}
void EditorIntegratorStatic::removeDocument( KTextEditor::Document* document )
{
  QMutexLocker lock(mutex);

  // TODO save smart stuff to non-smart cursors and ranges

  documents.remove(document->url());

  foreach (KTextEditor::Range* range, topRanges[document->url()])
    if (range && range->isSmartRange())
      range->toSmartRange()->removeWatcher(this);

  topRanges.remove(document->url());
}

void EditorIntegratorStatic::rangeDeleted(KTextEditor::SmartRange * range)
{
  QMutexLocker lock(mutex);

  QMutableHashIterator<KUrl, QVector<KTextEditor::Range*> > it = topRanges;
  while (it.hasNext()) {
    it.next();
    //kDebug(9506) << k_funcinfo << "Searching for" << range << ", potentials" << it.value().toList();
    int index = it.value().indexOf(range);
    if (index != -1) {
      it.value()[index] = 0;
      return;
    }
  }

  // Should have found the top level range by now
  kWarning() << k_funcinfo << "Could not find record of top level range" << range << "!" ;
}

}

#include "editorintegratorstatic.moc"

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
