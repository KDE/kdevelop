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

#include <QTimer>

#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/ilanguagecontroller.h>

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainutils.h>

#include <language/backgroundparser/backgroundparser.h>
#include <language/backgroundparser/parsejob.h>
#include <language/duchain/problem.h>

#include "signatureassistant.h"

using namespace KDevelop;
using namespace KTextEditor;

namespace Cpp {

StaticCodeAssistant::StaticCodeAssistant() : m_activeProblemAssistant(false)
{
  m_timer = new QTimer(this);
  m_timer->setSingleShot(true),
  m_timer->setInterval(400);

  connect(m_timer, SIGNAL(timeout()), SLOT(timeout()));
  connect(KDevelop::ICore::self()->documentController(),
          SIGNAL(documentLoaded(KDevelop::IDocument*)),
          SLOT(documentLoaded(KDevelop::IDocument*)));
  connect(KDevelop::ICore::self()->documentController(),
          SIGNAL(documentActivated(KDevelop::IDocument*)),
          SLOT(documentActivated(KDevelop::IDocument*)));

  foreach(IDocument* document, ICore::self()->documentController()->openDocuments()) {
    documentLoaded(document);
  }
  connect(KDevelop::ICore::self()->languageController()->backgroundParser(),
          SIGNAL(parseJobFinished(KDevelop::ParseJob*)),
          SLOT(parseJobFinished(KDevelop::ParseJob*)));
}

void StaticCodeAssistant::documentLoaded(IDocument* document)
{
  if(document->textDocument()) {
    ///@todo Make these connections non-queued, and then reach forward using a QPointer,
    /// since else a crash may happen when the document is destroyed before the message is processed
    connect(document->textDocument(),
            SIGNAL(textInserted(KTextEditor::Document*,KTextEditor::Range)),
            SLOT(textInserted(KTextEditor::Document*,KTextEditor::Range)));
    connect(document->textDocument(),
            SIGNAL(textRemoved(KTextEditor::Document*,KTextEditor::Range,QString)),
            SLOT(textRemoved(KTextEditor::Document*,KTextEditor::Range,QString)));
  }
}

void StaticCodeAssistant::hideAssistant()
{
  m_activeAssistant = KSharedPtr<KDevelop::IAssistant>();
  m_activeProblemAssistant = false;
}

void StaticCodeAssistant::textInserted(Document* document, const Range& range)
{
  m_eventualDocument = document;
  m_eventualRange = range;
  m_eventualRemovedText.clear();
  QMetaObject::invokeMethod(this, "eventuallyStartAssistant", Qt::QueuedConnection);
}

void StaticCodeAssistant::textRemoved(Document* document, const Range& range,
                                      const QString& removedText)
{
  m_eventualDocument = document;
  m_eventualRange = range;
  m_eventualRemovedText = removedText;
  QMetaObject::invokeMethod(this, "eventuallyStartAssistant", Qt::QueuedConnection);
}

void StaticCodeAssistant::eventuallyStartAssistant()
{
  if(!m_eventualDocument)
    return;

  View* view = m_eventualDocument.data()->activeView();
  //Eventually pop up an assistant
  if(!view)
    return;

  //FIXME: update signature assistant to play well with the rename assistant
  Range sigAssistRange = m_eventualRange;
  if (!m_eventualRemovedText.isEmpty()) {
    sigAssistRange.setRange(sigAssistRange.start(), sigAssistRange.start());
  }

  KSharedPtr<AdaptDefinitionSignatureAssistant> signatureAssistant(
    new AdaptDefinitionSignatureAssistant(view, sigAssistRange)
  );

  if(signatureAssistant->isUseful()) {
    startAssistant(KSharedPtr<IAssistant>(signatureAssistant.data()));
  }

  RenameAssistant* renameAssistant = m_renameAssistants[view].data();
  if (!renameAssistant) {
    renameAssistant =  new RenameAssistant(view);
    m_renameAssistants[view].attach(renameAssistant);
    connect(m_eventualDocument.data(),
            SIGNAL(aboutToClose(KTextEditor::Document*)),
            SLOT(deleteRenameAssistantsForDocument(KTextEditor::Document*)));
  }

  renameAssistant->textChanged(m_eventualRange, m_eventualRemovedText);

  if(renameAssistant->isUseful()) {
    startAssistant(KSharedPtr<IAssistant>(renameAssistant));
  }

  // optimize, esp. for setText() calls as done in e.g. reformat source
  // only start the assitant once for multiple textRemoved/textInserted signals
  m_eventualDocument.clear();
  m_eventualRange = Range::invalid();
  m_eventualRemovedText.clear();
}

void StaticCodeAssistant::deleteRenameAssistantsForDocument(Document* document)
{
  foreach(View *view, document->views()) {
    if (m_renameAssistants.contains(view))
      m_renameAssistants.remove(view);
  }
}

void StaticCodeAssistant::startAssistant(IAssistant::Ptr assistant)
{
  if(m_activeAssistant)
    m_activeAssistant->doHide();

  if(!m_currentView)
    return;

  m_activeAssistant = assistant;
  if(m_activeAssistant) {
    connect(m_activeAssistant.data(), SIGNAL(hide()), SLOT(assistantHide()), Qt::DirectConnection);
    ICore::self()->uiController()->popUpAssistant(IAssistant::Ptr(m_activeAssistant.data()));

    m_assistantStartedAt =  m_currentView.data()->cursorPosition();
  }
}

void StaticCodeAssistant::parseJobFinished(ParseJob* job)
{
  if(job->document() == m_currentDocument) {
    if(m_activeAssistant) {
      if(m_activeProblemAssistant)
        m_activeAssistant->doHide(); //Hide the assistant, as we will create a new one if the problem is still there
      else
        return;
    }
    DUChainReadLocker lock(DUChain::lock(), 300);
    if(!lock.locked())
      return;
    if(job->duChain()) {
      checkAssistantForProblems(job->duChain());
    }
  }
}

void StaticCodeAssistant::cursorPositionChanged(View*, const Cursor& pos)
{
  if(m_activeAssistant && m_assistantStartedAt.isValid()
      && abs(m_assistantStartedAt.line() - pos.line()) >= 1)
  {
    m_activeAssistant->doHide();
  }

  m_timer->start();
}

void StaticCodeAssistant::documentActivated(IDocument* doc)
{
  if(doc)
    m_currentDocument = IndexedString(doc->url());

  if(m_currentView) {
    disconnect(m_currentView.data(),
               SIGNAL(cursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor)),
               this, SLOT(cursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor)));
    m_currentView.clear();
  }

  if(doc->textDocument()) {
    m_currentView = doc->textDocument()->activeView();
    if(m_currentView) {
      connect(m_currentView.data(),
              SIGNAL(cursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor)),
              SLOT(cursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor)));
    }
  }
}

void StaticCodeAssistant::checkAssistantForProblems(TopDUContext* top)
{
    foreach(ProblemPointer problem, top->problems()) {
      if(m_currentView && m_currentView.data()->cursorPosition().line() == problem->range().start.line) {
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
    DUChainReadLocker lock(DUChain::lock(), 300);
    if(!lock.locked())
      return;

    TopDUContext* top = DUChainUtils::standardContextForUrl(m_currentDocument.toUrl());
    if(top)
      checkAssistantForProblems(top);
  }
}

}

#include "codeassistant.moc"
