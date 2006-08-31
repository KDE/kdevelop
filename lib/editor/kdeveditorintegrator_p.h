/* This file is part of KDevelop
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef KDEVEDITORINTEGRATOR_P_H
#define KDEVEDITORINTEGRATOR_P_H

#include <QObject>
#include <QHash>

#include <kurl.h>

#include <ktexteditor/range.h>
#include <ktexteditor/rangefeedback.h>

#include "kdevdocumentcursor.h"

class QReadWriteLock;

class KDevDocumentRange;
class KDevDocumentCursor;

namespace KTextEditor { class SmartRange; class SmartCursor; class SmartInterface; }

class KDevEditorIntegratorPrivate : public QObject, public KTextEditor::SmartRangeWatcher
{
  Q_OBJECT

public:
  KDevEditorIntegratorPrivate();
  virtual ~KDevEditorIntegratorPrivate();

  virtual void rangeDeleted(KTextEditor::SmartRange* range);

public Q_SLOTS:
  /**
   * Removes the text editor \a document from the integrator.
   */
  void removeDocument(KTextEditor::Document* document);

  void documentLoaded();

  void documentUrlChanged(KTextEditor::Document* document);

public:
  QMutex* mutex;

  QHash<KUrl, KTextEditor::Document*> documents;
  QHash<KUrl, QVector<KTextEditor::Range*> > topRanges;

  /* FIXME port to this?

  struct URLData {
    KTextEditor::Document* document;
    QVector<KTextEditor::Range*> topRanges;
  }

  QHash<KUrl, URLData*> urls;*/
};

#endif // EDITORINTEGRATOR_H

// kate: indent-width 2;
