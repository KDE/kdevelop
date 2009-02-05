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

#include "./missingincludecompletionmodel.h"
#include "completionitem.h"
#include <klocalizedstring.h>
#include "missingincludecompletionitem.h"
#include <language/duchain/duchainutils.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>

KDevelop::CodeCompletionWorker* MissingIncludeCompletionModel::createCompletionWorker()
{
  return new MissingIncludeCompletionWorker(this);
}

MissingIncludeCompletionModel* missingIncludeCompletionModelInstance = 0;

MissingIncludeCompletionModel::MissingIncludeCompletionModel(QObject* parent) : CodeCompletionModel(parent) {
  missingIncludeCompletionModelInstance = this;
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

void MissingIncludeCompletionModel::startWithExpression(KDevelop::DUContextPointer context, QString expression, QString localExpression) {
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
  QMutexLocker lock(&worker()->mutex);
  
//   CodeCompletionModel::completionInvokedInternal(view, range, invocationType, url);
  clear();
}

#if KDE_IS_VERSION(4,2,62)
KTextEditor::CodeCompletionModelControllerInterface2::MatchReaction MissingIncludeCompletionModel::matchingItem(const QModelIndex& matched) {
  //When something in this model matches, don't hide the completion-list
  kDebug() << "checking reaction";
  return None;
}
#endif

MissingIncludeCompletionWorker::MissingIncludeCompletionWorker(QObject* parent) : KDevelop::CodeCompletionWorker(parent), mutex(QMutex::Recursive) {
}

MissingIncludeCompletionWorker::~MissingIncludeCompletionWorker() {
  this->MissingIncludeCompletionWorker::abortCurrentCompletion();
  //To make sure computeCompletions is ready
  QMutexLocker lock(&mutex);
}

void MissingIncludeCompletionWorker::abortCurrentCompletion() {
  CodeCompletionWorker::abortCurrentCompletion();
}

void MissingIncludeCompletionModel::updateCompletionRange(KTextEditor::View* view, KTextEditor::SmartRange& range) {
  QMutexLocker lock(&worker()->mutex);
  if(worker()->context.topContextIndex()) {
    {
      KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock(), 500);
      if(lock.locked()) {
        KDevelop::TopDUContext* top = KDevelop::DUChainUtils::standardContextForUrl(view->document()->url());
        if(top)
          worker()->context = KDevelop::IndexedDUContext(top->findContextAt(KDevelop::SimpleCursor(range.end())));
      }
    }
    worker()->localExpression = range.text().join("\n");
    worker()->allowCompletion();

    emit doSpecialProcessingInBackground(0);
  }
}

QString MissingIncludeCompletionModel::filterString(KTextEditor::View* view, const KTextEditor::SmartRange& range, const KTextEditor::Cursor& position) {
  //No filtering at all
  return QString();
}

void MissingIncludeCompletionWorker::doSpecialProcessing(unsigned int data) {
  QMutexLocker mutexLock(&mutex);
  kDebug() << context.data() << aborting() << localExpression << prefixExpression;
  
  KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock(), 500);
  if(!lock.locked() || !context.data() || aborting() || (localExpression.isEmpty() && prefixExpression.isEmpty()))
    return;
  
  QString expression = prefixExpression + localExpression;
  
  
  QList<KSharedPtr<KDevelop::CompletionTreeElement> > items;
  KDevelop::CompletionCustomGroupNode* node = new KDevelop::CompletionCustomGroupNode(i18n("Not Included"), 1000);

  if(!localExpression.isEmpty())
  {
    typedef KSharedPtr<KDevelop::CompletionTreeItem> Item;
  
    foreach(Item item, missingIncludeCompletionItems(expression, expression + ": ", Cpp::ExpressionEvaluationResult(), context.data(), 0, localExpression.isEmpty()))
      node->appendChild(KSharedPtr<KDevelop::CompletionTreeElement>(item.data()));
  }
  
  items << KSharedPtr<KDevelop::CompletionTreeElement>(node);
  
  if(node->children.count())
    emit foundDeclarations(items, KDevelop::CodeCompletionContext::Ptr());  
  else
    emit foundDeclarations(QList<KSharedPtr<KDevelop::CompletionTreeElement> >(), KDevelop::CodeCompletionContext::Ptr());  
}

void MissingIncludeCompletionWorker::computeCompletions(KDevelop::DUContextPointer _context, const KTextEditor::Cursor& position, KTextEditor::View* view, const KTextEditor::Range& contextRange, const QString& contextText) {
  QMutexLocker mLock(&mutex);
  KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock(), 500);
  if(lock.locked()) {
    context = KDevelop::IndexedDUContext(_context.data());
    doSpecialProcessing(0);
  }
}

#include "missingincludecompletionmodel.moc"
