/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
   
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

#include "codeassistant.h"
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <ktexteditor/document.h>
#include <interfaces/iuicontroller.h>
#include <ktexteditor/view.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/backgroundparser/backgroundparser.h>
#include <language/duchain/declaration.h>
#include <language/backgroundparser/parsejob.h>
#include "signatureassistant.h"
#include <language/duchain/duchainutils.h>
#include <language/duchain/duchainutils.h>

using namespace KDevelop;

Q_DECLARE_METATYPE(SafeDocumentPointer)
Q_DECLARE_METATYPE(KTextEditor::Range)

namespace Cpp {

StaticCodeAssistant::StaticCodeAssistant() : m_activeProblemAssistant(false) {
  
  qRegisterMetaType<KTextEditor::Range>("KTextEditor::Range");
  qRegisterMetaType<SafeDocumentPointer>("SafeDocumentPointer");  
  
  m_timer = new QTimer(this);
  m_timer->setSingleShot(true),
  m_timer->setInterval(400);
  connect(m_timer, SIGNAL(timeout()), SLOT(timeout()));
  connect(KDevelop::ICore::self()->documentController(), SIGNAL(documentLoaded(KDevelop::IDocument*)), SLOT(documentLoaded(KDevelop::IDocument*)));
  connect(KDevelop::ICore::self()->documentController(), SIGNAL(documentActivated(KDevelop::IDocument*)), SLOT(documentActivated(KDevelop::IDocument*)));
  foreach(KDevelop::IDocument* document, KDevelop::ICore::self()->documentController()->openDocuments())
    documentLoaded(document);
  connect(KDevelop::ICore::self()->languageController()->backgroundParser(), SIGNAL(parseJobFinished(KDevelop::ParseJob*)), SLOT(parseJobFinished(KDevelop::ParseJob*)));
}

void StaticCodeAssistant::documentLoaded(KDevelop::IDocument* document) {
  
  if(document->textDocument())
  {
    ///@todo Make these connections non-queued, and then reach forward using a QPointer, since else a crash may happen when the document is destroyed before the message is processed
    connect(document->textDocument(), SIGNAL(textInserted(KTextEditor::Document*,KTextEditor::Range)), SLOT(textInserted(KTextEditor::Document*,KTextEditor::Range)));
    connect(document->textDocument(), SIGNAL(textRemoved(KTextEditor::Document*,KTextEditor::Range)), SLOT(textRemoved(KTextEditor::Document*,KTextEditor::Range)));
  }
}

void StaticCodeAssistant::assistantHide() {
  m_activeAssistant = KSharedPtr<KDevelop::IAssistant>();
  m_activeProblemAssistant = false;
}

void StaticCodeAssistant::textInserted(KTextEditor::Document* document, KTextEditor::Range range) {
  
  QMetaObject::invokeMethod(this, "eventuallyStartAssistant", Qt::QueuedConnection, Q_ARG(SafeDocumentPointer, document), Q_ARG(KTextEditor::Range, range));
}

void StaticCodeAssistant::textRemoved(KTextEditor::Document* document, KTextEditor::Range range) {
  range = KTextEditor::Range(range.start(), range.start());
  QMetaObject::invokeMethod(this, "eventuallyStartAssistant", Qt::QueuedConnection, Q_ARG(SafeDocumentPointer, document), Q_ARG(KTextEditor::Range, range));
}

void StaticCodeAssistant::eventuallyStartAssistant(SafeDocumentPointer document, KTextEditor::Range range) {

  if(!document)
    return;
  
  if(m_activeAssistant) {
//     kDebug() << "there still is an active assistant";
//     if(abs(m_activeAssistant->invocationCursor().line() < range.start().line()) >= 1) {
//       kDebug() << "assistant was not deleted in time";
//     }else{
//       return;
//     }
  }
  //Eventually pop up an assistant
  if(!document->activeView())
    return;
  
  KSharedPtr<AdaptDefinitionSignatureAssistant> signatureAssistant(new AdaptDefinitionSignatureAssistant(document->activeView(), range));
  
  if(signatureAssistant->isUseful()) {
    startAssistant(KSharedPtr<IAssistant>(signatureAssistant.data()));
  }
}

void StaticCodeAssistant::startAssistant(KSharedPtr< KDevelop::IAssistant > assistant, bool manage) {
  Q_UNUSED(manage);
  if(m_activeAssistant)
    m_activeAssistant->doHide();
  
  if(!m_currentView)
    return;
  
  m_activeAssistant = assistant;
  if(m_activeAssistant) {
    connect(m_activeAssistant.data(), SIGNAL(hide()), SLOT(assistantHide()));
    ICore::self()->uiController()->popUpAssistant(IAssistant::Ptr(m_activeAssistant.data()));
    
    m_assistantStartedAt =  m_currentView->cursorPosition();
  }
}

void StaticCodeAssistant::parseJobFinished(KDevelop::ParseJob* job) {
  if(job->document() == m_currentDocument) {
    if(m_activeAssistant) {
      if(m_activeProblemAssistant)
        m_activeAssistant->doHide(); //Hide the assistant, as we will create a new one if the problem is still there
      else
        return;
    }
    KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock(), 300);
    if(!lock.locked())
      return;
    if(job->duChain()) {
      checkAssistantForProblems(job->duChain());
    }
  }
}

void StaticCodeAssistant::cursorPositionChanged(KTextEditor::View* , KTextEditor::Cursor pos ) {
  if(m_activeAssistant && m_assistantStartedAt.isValid())
    if(abs(m_assistantStartedAt.line() - pos.line()) >= 1)
      m_activeAssistant->doHide();
    
  m_timer->start();
}

void StaticCodeAssistant::documentActivated(KDevelop::IDocument* doc) {
  if(doc)
    m_currentDocument = KDevelop::IndexedString(doc->url());
  
  if(m_currentView) {
    disconnect(m_currentView, SIGNAL(cursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor)), this, SLOT(cursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor)));
    m_currentView = 0;
  }
  
  if(doc->textDocument()) {
    m_currentView = doc->textDocument()->activeView();
    if(m_currentView)
      connect(m_currentView, SIGNAL(cursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor)), this, SLOT(cursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor)));
  }
}

void StaticCodeAssistant::checkAssistantForProblems(KDevelop::TopDUContext* top) {
    //Check whether one of the problems is in the current line, and if yes, show its assistant
//     kDebug() << "checking, problem-count:" << top->problems().size();
    foreach(KDevelop::ProblemPointer problem, top->problems()) {
//       kDebug() << "range of problem:" << problem->range().textRange() << "matching to" << m_currentView->cursorPosition().line();
      if(m_currentView && m_currentView->cursorPosition().line() == problem->range().start.line) {
        IAssistant::Ptr solution = problem->solutionAssistant();
        if(solution) {
          startAssistant(solution);
          m_activeProblemAssistant = true;
          break;
        }
      }
    }
}

void StaticCodeAssistant::timeout() {
  if(!m_activeAssistant && m_currentView) {
    KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock(), 300);
    if(!lock.locked())
      return;
    
    TopDUContext* top = KDevelop::DUChainUtils::standardContextForUrl(m_currentDocument.toUrl());
    if(top)
      checkAssistantForProblems(top);
  }
}

}

#include "codeassistant.moc"
