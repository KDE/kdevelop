/*
 * KDevelop C++ Code Completion Support
 *
 * Copyright 2006 Hamish Rodda <rodda@kde.org>
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

#include "codecompletion.h"

#include <kparts/partmanager.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/codecompletionmodel.h>
#include <ktexteditor/codecompletioninterface.h>

#include <icore.h>

#include <duchain.h>
#include <topducontext.h>

using namespace KTextEditor;
using namespace KDevelop;

CodeCompletion::CodeCompletion(QObject *parent, KTextEditor::CodeCompletionModel* aModel)
  : QObject(parent), m_model(aModel)
{
  connect (KDevelop::ICore::self()->partManager(), SIGNAL(partAdded(KParts::Part*)),
    SLOT(documentLoaded(KParts::Part*)));
  aModel->setParent(this);
}

CodeCompletion::~CodeCompletion()
{
}

void CodeCompletion::viewCreated(KTextEditor::Document * document, KTextEditor::View * view)
{
  Q_UNUSED(document);

  if (CodeCompletionInterface* cc = dynamic_cast<CodeCompletionInterface*>(view)) {
    cc->registerCompletionModel(m_model);
    kDebug() << "Registered completion model";
  }
}

void CodeCompletion::documentLoaded(KParts::Part* document)
{
  KTextEditor::Document *textDocument = dynamic_cast<KTextEditor::Document*>(document);
  if (textDocument) {
    foreach (KTextEditor::View* view, textDocument->views())
      viewCreated(textDocument, view);

    connect(textDocument, SIGNAL(viewCreated(KTextEditor::Document*, KTextEditor::View*)), SLOT(viewCreated(KTextEditor::Document*, KTextEditor::View*)));

  } else {
    kDebug() << "Non-text editor document added";
  }
}

#include "codecompletion.moc"

