/*
 * KDevelop Generic Code Completion Support
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

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/codecompletionmodel.h>
#include <ktexteditor/codecompletioninterface.h>

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>

#include "../duchain/duchain.h"
#include "../duchain/topducontext.h"
#include "util/debug.h"
#include "codecompletionmodel.h"
#include <interfaces/idocumentcontroller.h>

using namespace KTextEditor;
using namespace KDevelop;

CodeCompletion::CodeCompletion(QObject *parent, KTextEditor::CodeCompletionModel* aModel, const QString& language)
  : QObject(parent), m_model(aModel), m_language(language)
{
  KDevelop::CodeCompletionModel* kdevModel = dynamic_cast<KDevelop::CodeCompletionModel*>(aModel);
  if(kdevModel)
    kdevModel->initialize();
  connect(KDevelop::ICore::self()->documentController(), &IDocumentController::textDocumentCreated,
          this, &CodeCompletion::textDocumentCreated);
  connect( ICore::self()->documentController(), &IDocumentController::documentUrlChanged,
           this, &CodeCompletion::documentUrlChanged );
  aModel->setParent(this);

  // prevent deadlock
  QMetaObject::invokeMethod(this, "checkDocuments", Qt::QueuedConnection);
}

CodeCompletion::~CodeCompletion()
{
}

void CodeCompletion::checkDocuments()
{
  foreach( KDevelop::IDocument* doc, KDevelop::ICore::self()->documentController()->openDocuments() ) {
    if (doc->textDocument()) {
      checkDocument(doc->textDocument());
    }
  }
}

void CodeCompletion::viewCreated(KTextEditor::Document * document, KTextEditor::View * view)
{
  Q_UNUSED(document);

  if (CodeCompletionInterface* cc = dynamic_cast<CodeCompletionInterface*>(view)) {
    cc->registerCompletionModel(m_model);
    qCDebug(LANGUAGE) << "Registered completion model";
  }
}

void CodeCompletion::documentUrlChanged(KDevelop::IDocument* document)
{
  // The URL has changed (might have a different language now), so we re-register the document
  Document* textDocument = document->textDocument();

  if(textDocument)
  {
    checkDocument(textDocument);
  }
}

void CodeCompletion::textDocumentCreated(KDevelop::IDocument* document)
{
  Q_ASSERT(document->textDocument());
  checkDocument(document->textDocument());
}

void CodeCompletion::unregisterDocument(Document* textDocument)
{
  foreach (KTextEditor::View* view, textDocument->views())
    if (CodeCompletionInterface* cc = dynamic_cast<CodeCompletionInterface*>(view))
      cc->unregisterCompletionModel(m_model);

  disconnect(textDocument, &Document::viewCreated, this, &CodeCompletion::viewCreated);
}

void CodeCompletion::checkDocument(Document* textDocument)
{
  unregisterDocument(textDocument);

  auto langs = ICore::self()->languageController()->languagesForUrl( textDocument->url() );

  bool found = false;
  foreach(auto lang, langs) {
    if(m_language==lang->name()) {
      found=true;
      break;
    }
  }
  if(!found && !m_language.isEmpty())
      return;

  foreach (KTextEditor::View* view, textDocument->views())
    viewCreated(textDocument, view);

  connect(textDocument, &Document::viewCreated, this, &CodeCompletion::viewCreated);
}



