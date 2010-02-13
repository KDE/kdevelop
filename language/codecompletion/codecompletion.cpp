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
#include <interfaces/ipartcontroller.h>
#include <interfaces/ilanguage.h>
#include <interfaces/ilanguagecontroller.h>

#include "../duchain/duchain.h"
#include "../duchain/topducontext.h"
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
  connect (KDevelop::ICore::self()->partController(), SIGNAL(partAdded(KParts::Part*)),
    SLOT(documentLoaded(KParts::Part*)));
  connect( KDevelop::ICore::self()->documentController(), SIGNAL(documentSaved(KDevelop::IDocument*)), SLOT(documentSaved(KDevelop::IDocument*)) );
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

void CodeCompletion::documentSaved(KDevelop::IDocument* document)
{
  // The URL may have changed, so we re-register the document
  Document* textDocument = document->textDocument();
  
  if(textDocument)
  {
    unregisterDocument(textDocument);
    checkDocument(textDocument);
  }
}

void CodeCompletion::documentLoaded(KParts::Part* document)
{
  KTextEditor::Document *textDocument = dynamic_cast<KTextEditor::Document*>(document);
  if (textDocument) {
    checkDocument(textDocument);
  } else {
    kDebug() << "Non-text editor document added";
  }
}

void CodeCompletion::unregisterDocument(Document* textDocument)
{
  foreach (KTextEditor::View* view, textDocument->views())
    if (CodeCompletionInterface* cc = dynamic_cast<CodeCompletionInterface*>(view))
      cc->unregisterCompletionModel(m_model);
    
  disconnect(textDocument, SIGNAL(viewCreated(KTextEditor::Document*, KTextEditor::View*)), this, SLOT(viewCreated(KTextEditor::Document*, KTextEditor::View*)));
}

void CodeCompletion::checkDocument(Document* textDocument)
{
  QList<ILanguage*> langs=ICore::self()->languageController()->languagesForUrl( textDocument->url() );
  
  bool found=false;
  foreach(ILanguage* lang, langs) {
    if(m_language==lang->name()) {
      found=true;
      break;
    }
  }
  if(!found && !m_language.isEmpty())
      return;
  
  foreach (KTextEditor::View* view, textDocument->views())
    viewCreated(textDocument, view);

  connect(textDocument, SIGNAL(viewCreated(KTextEditor::Document*, KTextEditor::View*)), SLOT(viewCreated(KTextEditor::Document*, KTextEditor::View*)));
}


#include "codecompletion.moc"

