/*
 * KDevelop Problem Reporter
 *
 * Copyright (c) 2006-2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2006 Adam Treat <treat@kde.org>
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

#include "problemwidget.h"

#include <QMenu>
#include <QCursor>
#include <QContextMenuEvent>

#include <klocale.h>
#include <kicon.h>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/ilanguage.h>
#include <interfaces/idocument.h>
#include <language/interfaces/ilanguagesupport.h>
#include <language/backgroundparser/backgroundparser.h>
#include <language/backgroundparser/parsejob.h>

#include <language/duchain/duchain.h>
#include <language/duchain/duchainobserver.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/parsingenvironment.h>

#include "problemreporterplugin.h"
#include "problemmodel.h"
#include <kaction.h>
#include <interfaces/iassistant.h>

//#include "modeltest.h"

using namespace KDevelop;

ProblemWidget::ProblemWidget(QWidget* parent, ProblemReporterPlugin* plugin)
    : QTreeView(parent)
    , m_plugin(plugin)
{
    setObjectName("Problem Reporter Tree");
    setWindowTitle(i18n("Problems"));
    setWindowIcon( KIcon("dialog-information") ); ///@todo Use a proper icon
    setRootIsDecorated(true);
    setWhatsThis( i18n( "Problems" ) );
    setModel(new ProblemModel(m_plugin));

//     setContextMenuPolicy(Qt::CustomContextMenu);
    
    m_fullUpdateAction = new KAction(this);
    m_fullUpdateAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_fullUpdateAction->setText(i18n("Force Full Update"));
    m_fullUpdateAction->setToolTip(i18n("Re-parse the current file and all its imports."));
    m_fullUpdateAction->setIcon(KIcon("view-refresh"));
    connect(m_fullUpdateAction, SIGNAL(triggered(bool)), this, SLOT(forceFullUpdate()));
    addAction(m_fullUpdateAction);
    //new ModelTest(model());

    connect(this, SIGNAL(activated(const QModelIndex&)), SLOT(itemActivated(const QModelIndex&)));
    bool success = connect(ICore::self()->languageController()->backgroundParser(), SIGNAL(parseJobFinished(KDevelop::ParseJob*)), SLOT(parseJobFinished(KDevelop::ParseJob*)), Qt::DirectConnection);
    connect(this, SIGNAL(activated(const QModelIndex&)), SLOT(itemActivated(const QModelIndex&)));
    connect(ICore::self()->documentController(), SIGNAL(documentActivated(KDevelop::IDocument*)), SLOT(documentActivated(KDevelop::IDocument*)));
    Q_ASSERT(success);
}

ProblemWidget::~ProblemWidget()
{
}

void ProblemWidget::collectProblems(QList<ProblemPointer>& allProblems, TopDUContext* context, QSet<TopDUContext*>& hadContexts)
{
  if(!context) {
      kDebug() << "collecting from bad context";
      return;
  }
  if(hadContexts.contains(context))
    return;

  hadContexts.insert(context);

  allProblems += context->problems();

  bool isProxy = context->parsingEnvironmentFile() && context->parsingEnvironmentFile()->isProxyContext();
  foreach(const DUContext::Import &ctx, context->importedParentContexts()) {
      if(!ctx.indexedContext().indexedTopContext().isLoaded())
          continue;
    TopDUContext* topCtx = dynamic_cast<TopDUContext*>(ctx.context(0));
    if(topCtx) {
      //If we are starting at a proxy-context, only recurse into other proxy-contexts,
      //because those contain the problems.
      if(!isProxy || (topCtx->parsingEnvironmentFile() && topCtx->parsingEnvironmentFile()->isProxyContext()))
        collectProblems(allProblems, topCtx, hadContexts);
    }
  }
}

void ProblemWidget::forceFullUpdate() {
    kDebug() << "forcing full update";
    if(!m_activeUrl.isValid()) {
        kWarning() << "no active url";
        return;
    }
    DUChainReadLocker lock(DUChain::lock());
    DUChain::self()->updateContextForUrl(IndexedString(m_activeUrl), (TopDUContext::Features)(KDevelop::TopDUContext::VisibleDeclarationsAndContexts | KDevelop::TopDUContext::ForceUpdateRecursive));
}

void ProblemWidget::showProblems(TopDUContext* ctx, KDevelop::IDocument* doc)
{
  if(ctx) {
    QList<ProblemPointer> allProblems;
    QSet<TopDUContext*> hadContexts;
    DUChainReadLocker lock(DUChain::lock());
    collectProblems(allProblems, ctx, hadContexts);
    model()->setProblems(allProblems, m_activeDirectory);
    if (isVisible()) {
        // no need to resize columns if the toolview isn't visible
        // we will resize them right after show anyway
        for (int i = 0; i < model()->columnCount(); ++i)
            resizeColumnToContents(i);
    }
  }else{
    model()->clear();
  }
}

void ProblemWidget::documentActivated(KDevelop::IDocument* doc)
{
  m_activeDirectory = doc->url().upUrl();
  m_activeUrl = doc->url();

  QList<KDevelop::ILanguage*> languages = ICore::self()->languageController()->languagesForUrl(doc->url());

  KDevelop::TopDUContext* chosen = 0;

  DUChainReadLocker lock;
  
  foreach( KDevelop::ILanguage* language, languages)
    if(!chosen)
      chosen = language->languageSupport()->standardContext(doc->url(), true);

  showProblems(chosen, doc);
}

void ProblemWidget::parseJobFinished(KDevelop::ParseJob* job)
{
  KUrl url = job->document().toUrl();
  IDocument* active = ICore::self()->documentController()->activeDocument();

  DUChainReadLocker lock;
  
  if(active) {
    //For now, only show problems from the current document
    if(active->url() == url && job->duChain()) {
      showProblems(job->duChain(), active);
    }
  }
}

void ProblemWidget::itemActivated(const QModelIndex& index)
{
    if (!index.isValid())
        return;

  KTextEditor::Cursor start;
    KUrl url;

    {
      DUChainReadLocker lock(DUChain::lock());
      KDevelop::ProblemPointer problem = model()->problemForIndex(index);
      if (!index.internalPointer()) {
        url = KUrl(problem->finalLocation().document().str());
        start = problem->finalLocation().start();
      }else{
        url = KUrl(problem->locationStack().at(index.row()).document().str());
        start = problem->locationStack().at(index.row());
      }
    }

    m_plugin->core()->documentController()->openDocument(url, start);
}

ProblemModel * ProblemWidget::model() const
{
    return static_cast<ProblemModel*>(QTreeView::model());
}

void ProblemWidget::contextMenuEvent(QContextMenuEvent* event) {
    QModelIndex index = indexAt(event->pos());
    if(index.isValid()) {
        KDevelop::ProblemPointer problem = model()->problemForIndex(index);
        if(problem) {
            KSharedPtr<KDevelop::IAssistant> solution = problem->solutionAssistant();
            QList<QAction*> actions;
            if(solution) {
                foreach(KDevelop::IAssistantAction::Ptr action, solution->actions())
                    actions << action->toKAction();
            }
            if(!actions.isEmpty())
                QMenu::exec(actions, event->globalPos());
        }
    }
}

void ProblemWidget::showEvent(QShowEvent * event)
{
    Q_UNUSED(event)

    for (int i = 0; i < model()->columnCount(); ++i)
        resizeColumnToContents(i);
}

#include "problemwidget.moc"
