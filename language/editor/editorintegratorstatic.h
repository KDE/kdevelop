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

#ifndef KDEVPLATFORM_EDITORINTEGRATOR_H
#define KDEVPLATFORM_EDITORINTEGRATOR_H

#include <QObject>
#include <QHash>

#include <kurl.h>

#include <ktexteditor/range.h>

#include "../duchain/indexedstring.h"
#include "documentcursor.h"

class QMutex;

namespace KDevelop
{
class EditorIntegrator;

class EditorIntegratorStatic : public QObject
{
  Q_OBJECT

public:
  EditorIntegratorStatic();
  virtual ~EditorIntegratorStatic();

  void insertLoadedDocument(KTextEditor::Document* document);

Q_SIGNALS:
  void documentLoaded(KTextEditor::Document* document);
  void documentAboutToBeDeleted(KTextEditor::Document* document);
  ///Emitted after all editor-integrators have been cleared. At that point,
  ///smart-ranges can be deleted without the chance of crashing an editor-integrator.
  ///In this stage, editor-integrators don't work any more.
  ///The duchain is guaranteed to be write-locked, so no expensive operations should be performed.
  void documentAboutToBeDeletedFinal(KTextEditor::Document* document);
  void documentAboutToBeReloaded(KTextEditor::Document* document);

public Q_SLOTS:
  /**
   * Removes the text editor \a document from the integrator.
   */
  void removeDocument(KTextEditor::Document* document);

  void documentUrlChanged(KTextEditor::Document* document);

  void reloadDocument(KTextEditor::Document* document);

public:
  QMutex* mutex;

  QHash<IndexedString, KTextEditor::Document*> documents;
  QMultiHash<KTextEditor::Document*, EditorIntegrator*> editorIntegrators;
};

}
#endif // KDEVPLATFORM_EDITORINTEGRATOR_H

