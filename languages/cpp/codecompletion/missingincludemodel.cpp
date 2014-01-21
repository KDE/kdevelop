/*
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

#include "missingincludemodel.h"
#include "item.h"
#include "missingincludeitem.h"
#include <expressionevaluationresult.h>
#include <language/duchain/duchainutils.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <language/backgroundparser/backgroundparser.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/backgroundparser/parsejob.h>
#include <interfaces/idocumentcontroller.h>
#include <ktexteditor/codecompletioninterface.h>

namespace Cpp {

KDevelop::CodeCompletionWorker* MissingIncludeCompletionModel::createCompletionWorker()
{
  return new MissingIncludeCompletionWorker(this);
}

namespace {
  KDevelop::IndexedString startCompletionAfterParsingUrl;
}

void MissingIncludeCompletionModel::startCompletionAfterParsing(const KDevelop::IndexedString& url) {
  startCompletionAfterParsingUrl = url;
}

void MissingIncludeCompletionModel::parseJobFinished(KDevelop::ParseJob* job) {
  if(job->document() == startCompletionAfterParsingUrl && !KDevelop::ICore::self()->languageController()->backgroundParser()->isQueued(job->document())) {
    startCompletionAfterParsingUrl = KDevelop::IndexedString();
    KDevelop::IDocument* doc = KDevelop::ICore::self()->documentController()->documentForUrl(job->document().toUrl());
    if(doc && doc->textDocument() && doc->textDocument()->activeView() && doc->textDocument()->activeView()->hasFocus()) {
      KTextEditor::CodeCompletionInterface* iface = dynamic_cast<KTextEditor::CodeCompletionInterface*>(doc->textDocument()->activeView());
      if(iface) {
        ///@todo 1. This is a non-public interface, and 2. Completion should be started in "automatic invocation" mode
        QMetaObject::invokeMethod(doc->textDocument()->activeView(), "userInvokedCompletion");
      }
    }
  }
}


MissingIncludeCompletionModel* missingIncludeCompletionModelInstance = 0;

MissingIncludeCompletionModel::MissingIncludeCompletionModel(QObject* parent) : CodeCompletionModel(parent) {
  missingIncludeCompletionModelInstance = this;
  connect(KDevelop::ICore::self()->languageController()->backgroundParser(), SIGNAL(parseJobFinished(KDevelop::ParseJob*)), SLOT(parseJobFinished(KDevelop::ParseJob*)));
}

MissingIncludeCompletionModel::~MissingIncludeCompletionModel() {
}

MissingIncludeCompletionModel& MissingIncludeCompletionModel::self() {
  return *missingIncludeCompletionModelInstance;
}

void MissingIncludeCompletionModel::stop() {
  QMutexLocker lock(&worker()->mutex);
  worker()->context = KDevelop::IndexedDUContext();
}

void MissingIncludeCompletionModel::startWithExpression(KDevelop::DUContextPointer context, const QString& expression, const QString& localExpression) {
  QMutexLocker lock(&worker()->mutex);
  worker()->prefixExpression = expression;
  worker()->localExpression = localExpression;
  {
    KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock(), 500);
    if(lock.locked())
      worker()->context = KDevelop::IndexedDUContext(context.data());
  }
  if(!context)
    worker()->abortCurrentCompletion();
  else {
    kDebug() << "starting special-processing within background, expr." << expression;
    worker()->allowCompletion();
    emit doSpecialProcessingInBackground(0);
  }
}

void MissingIncludeCompletionWorker::allowCompletion() {
  aborting() = false;
}

MissingIncludeCompletionWorker* MissingIncludeCompletionModel::worker() const {
  return static_cast<MissingIncludeCompletionWorker*>(CodeCompletionModel::worker());
}

void MissingIncludeCompletionModel::completionInvokedInternal(KTextEditor::View* view, const KTextEditor::Range& range, KTextEditor::CodeCompletionModel::InvocationType invocationType, const KUrl& url) {
  Q_UNUSED(view);
  Q_UNUSED(range);
  Q_UNUSED(invocationType);
  Q_UNUSED(url);
  QMutexLocker lock(&worker()->mutex);
  
//   CodeCompletionModel::completionInvokedInternal(view, range, invocationType, url);
  clear();
}

KTextEditor::CodeCompletionModelControllerInterface::MatchReaction MissingIncludeCompletionModel::matchingItem(const QModelIndex& matched) {
  Q_UNUSED(matched);
  //When something in this model matches, don't hide the completion-list
  kDebug() << "checking reaction";
  return None;
}

MissingIncludeCompletionWorker::MissingIncludeCompletionWorker(KDevelop::CodeCompletionModel* model) : KDevelop::CodeCompletionWorker(model), mutex(QMutex::Recursive) {
}

MissingIncludeCompletionWorker::~MissingIncludeCompletionWorker() {
  this->MissingIncludeCompletionWorker::abortCurrentCompletion();
  //To make sure computeCompletions is ready
  QMutexLocker lock(&mutex);
}

void MissingIncludeCompletionWorker::abortCurrentCompletion() {
  CodeCompletionWorker::abortCurrentCompletion();
}

KTextEditor::Range MissingIncludeCompletionModel::updateCompletionRange(KTextEditor::View* view, const KTextEditor::Range& range) {
  QMutexLocker lock(&worker()->mutex);
  if(worker()->context.topContextIndex()) {
    {
      KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock(), 500);
      if(lock.locked()) {
        KDevelop::TopDUContext* top = KDevelop::DUChainUtils::standardContextForUrl(view->document()->url());
        if(top)
          worker()->context = KDevelop::IndexedDUContext(top->findContextAt(top->transformToLocalRevision(KDevelop::SimpleCursor(range.end()))));
      }
    }
    worker()->localExpression = view->document()->text(range);
    worker()->allowCompletion();

    emit doSpecialProcessingInBackground(0);
  }

  return range;
}

QString MissingIncludeCompletionModel::filterString(KTextEditor::View* view, const KTextEditor::Range& range, const KTextEditor::Cursor& position) {
  Q_UNUSED(view);
  Q_UNUSED(range);
  Q_UNUSED(position);
  //No filtering at all
  return QString();
}

void MissingIncludeCompletionWorker::doSpecialProcessing(unsigned int data) {
  Q_UNUSED(data);
  QMutexLocker localLock(&mutex);
  
  kDebug() << context.data() << aborting() << localExpression << prefixExpression;
  
  KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock(), 500);
  if(!lock.locked() || !context.data() || aborting() || (localExpression.isEmpty() && prefixExpression.isEmpty()))
    return;
  
  QString expression = prefixExpression + localExpression;
  bool needInstance = localExpression.isEmpty();
  KDevelop::DUContext* ctx = context.data();
  QString localExpressionCopy = localExpression;
  
  localLock.unlock(); //We unlock the local lock, because else the UI will block until the function has completed.
  //This means that from now on, we must not use any local functions.
  
  QList<KSharedPtr<KDevelop::CompletionTreeElement> > items;
  KDevelop::CompletionCustomGroupNode* node = new KDevelop::CompletionCustomGroupNode(i18n("Not Included"), 1000);

  if(!localExpressionCopy.isEmpty())
  {
    typedef KSharedPtr<KDevelop::CompletionTreeItem> Item;
  
    QList<KDevelop::CompletionTreeItemPointer> items = missingIncludeCompletionItems(expression, expression + ": ", Cpp::ExpressionEvaluationResult(), ctx, 0, needInstance);
    foreach(Item item, items)
      node->appendChild(KSharedPtr<KDevelop::CompletionTreeElement>(item.data()));
  }
  
  items << KSharedPtr<KDevelop::CompletionTreeElement>(node);
  
  localLock.relock();
  
  if(localExpressionCopy != localExpression) {
    //If a new expression has been set meanwhile, re-start the processing and don't show old results
    localLock.unlock();
    lock.unlock();
    doSpecialProcessing(0);
    return;
  }else if(node->children.count() && context.isValid()) //Only emit if the local context has not been invalidated meanwhile during abort()
    emit foundDeclarations(items, KDevelop::CodeCompletionContext::Ptr());  
  else
    emit foundDeclarations(QList<KSharedPtr<KDevelop::CompletionTreeElement> >(), KDevelop::CodeCompletionContext::Ptr());  
}

void MissingIncludeCompletionWorker::computeCompletions(KDevelop::DUContextPointer _context, const KTextEditor::Cursor& position, QString followingText, const KTextEditor::Range& contextRange, const QString& contextText) {
  Q_UNUSED(position);
  Q_UNUSED(followingText);
  Q_UNUSED(contextRange);
  Q_UNUSED(contextText);
  QMutexLocker mLock(&mutex);
  KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock(), 500);
  if(lock.locked()) {
    context = KDevelop::IndexedDUContext(_context.data());
    doSpecialProcessing(0);
  }
}

}

#include "missingincludemodel.moc"
