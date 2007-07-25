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

#include "cppcodecompletion.h"

#include <kparts/partmanager.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/codecompletioninterface.h>

#include <icore.h>

#include "cpplanguagesupport.h"
#include "cppcodecompletionmodel.h"

#include <duchain.h>
#include <topducontext.h>

using namespace KTextEditor;

CppCodeCompletion::CppCodeCompletion( CppLanguageSupport * parent )
  : QObject(parent)
  , m_model(new CppCodeCompletionModel(this))
{
  connect (parent->core()->partManager(), SIGNAL(partAdded(KParts::Part*)),
    SLOT(documentLoaded(KParts::Part*)));
}

CppCodeCompletion::~CppCodeCompletion()
{
}

void CppCodeCompletion::viewCreated(KTextEditor::Document * document, KTextEditor::View * view)
{
  Q_UNUSED(document);

  if (CodeCompletionInterface* cc = dynamic_cast<CodeCompletionInterface*>(view)) {
    cc->setAutomaticInvocationEnabled(true);
    cc->registerCompletionModel(m_model);
    kDebug(9007) << "Registered completion model" << endl;
  }
}

void CppCodeCompletion::documentLoaded(KParts::Part* document)
{
  KTextEditor::Document *textDocument = dynamic_cast<KTextEditor::Document*>(document);
  if (textDocument) {
    foreach (KTextEditor::View* view, textDocument->views())
      viewCreated(textDocument, view);

    connect(textDocument, SIGNAL(viewCreated(KTextEditor::Document*, KTextEditor::View*)), SLOT(viewCreated(KTextEditor::Document*, KTextEditor::View*)));

  } else {
    kDebug(9007) << k_funcinfo << "Non-text editor document added" << endl;
  }
}

#include "cppcodecompletion.moc"

// kate: space-indent on; indent-width 2; replace-tabs on
