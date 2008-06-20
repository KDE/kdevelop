/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef EDITORINTEGRATOR_H
#define EDITORINTEGRATOR_H

#include <QObject>
#include <QHash>

#include <kurl.h>

#include <ktexteditor/range.h>
#include <ktexteditor/rangefeedback.h>

#include "hashedstring.h"
#include "documentcursor.h"

namespace KTextEditor
{
   class SmartRange;
      }

class QMutex;

namespace KDevelop
{


class EditorIntegratorStatic : public QObject, public KTextEditor::SmartRangeWatcher
{
  Q_OBJECT

public:
  EditorIntegratorStatic();
  virtual ~EditorIntegratorStatic();

Q_SIGNALS:
  void documentAboutToBeDeleted(KTextEditor::Document* document);
  void documentAboutToBeReloaded(KTextEditor::Document* document);

public Q_SLOTS:
  /**
   * Removes the text editor \a document from the integrator.
   */
  void removeDocument(KTextEditor::Document* document);

  void documentLoaded();

  void documentUrlChanged(KTextEditor::Document* document);
  
  void reloadDocument(KTextEditor::Document* document);

public:
  QMutex* mutex;

  QHash<HashedString, KTextEditor::Document*> documents;

  /* FIXME port to this?

  struct URLData {
    KTextEditor::Document* document;
    QVector<KTextEditor::Range*> topRanges;
  }

  QHash<KUrl, URLData*> urls;*/
};

}
#endif // EDITORINTEGRATOR_H

