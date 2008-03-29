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

#include <QHeaderView>
#include <QMenu>
#include <QCursor>
#include <QContextMenuEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>

#include <klocale.h>
#include <kicon.h>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <icore.h>
#include <idocumentcontroller.h>
#include <ilanguagecontroller.h>
#include <ilanguagesupport.h>
#include <ilanguage.h>
#include <language/backgroundparser/backgroundparser.h>
#include <language/backgroundparser/parsejob.h>
#include <idocument.h>

#include "duchain.h"
#include "duchainobserver.h"
#include "duchainlock.h"

#include "problemreporterplugin.h"
#include "problemmodel.h"

//#include "modeltest.h"

using namespace KDevelop;

ProblemWidget::ProblemWidget(QWidget* parent, ProblemReporterPlugin* plugin)
    : QTreeView(parent)
    , m_plugin(plugin)
{
    setObjectName("Problem Reporter Tree");
    setWindowTitle(i18n("Problem Reporter"));
    setWindowIcon(KIcon("info"));
    setRootIsDecorated(true);
    setWhatsThis( i18n( "Problem Reporter" ) );
    setModel(new ProblemModel(m_plugin));

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

void collectProblems(QList<ProblemPointer>& allProblems, TopDUContext* context, QSet<TopDUContext*>& hadContexts)
{
  if(hadContexts.contains(context))
    return;
  
  hadContexts.insert(context);
  
  allProblems += context->problems();
  bool isProxy = context->flags() & TopDUContext::ProxyContextFlag;
  foreach(DUContextPointer ctx, context->importedParentContexts()) {
    TopDUContext* topCtx = dynamic_cast<TopDUContext*>(ctx.data());
    if(topCtx) {
      //If we are starting at a proxy-context, only recurse into other proxy-contexts,
      //because those contain the problems.
      if(!isProxy || (topCtx->flags() & TopDUContext::ProxyContextFlag))
        collectProblems(allProblems, topCtx, hadContexts);
    }
  }
}

void ProblemWidget::showProblems(TopDUContext* ctx)
{
  if(ctx) {
    QList<ProblemPointer> allProblems;
    QSet<TopDUContext*> hadContexts;
    DUChainReadLocker lock(DUChain::lock());
    collectProblems(allProblems, ctx, hadContexts);
    model()->setProblems(allProblems);
    resizeColumnToContents(0);
  }else{
    model()->clear();
  }
}

void ProblemWidget::documentActivated(KDevelop::IDocument* doc)
{
  kDebug() << "activated document:" << doc->url();
  
  QList<KDevelop::ILanguage*> languages = ICore::self()->languageController()->languagesForUrl(doc->url());

  KDevelop::TopDUContext* chosen = 0;
  
  foreach( KDevelop::ILanguage* language, languages)
    if(!chosen)
      chosen = language->languageSupport()->standardContext(doc->url(), true);

  showProblems(chosen);
}

void ProblemWidget::parseJobFinished(KDevelop::ParseJob* job)
{
  KUrl url(job->document().str());
  IDocument* active = ICore::self()->documentController()->activeDocument();

  if(active) {
    kDebug() << "active document:" << active->url() << "url:" << url;
    //For now, only show problems from the current document
    if(active->url() == url && job->duChain()) {
      showProblems(job->duChain());
    }else{
      //Clear all problems
      showProblems(0);
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

void ProblemWidget::showEvent(QShowEvent * event)
{
    Q_UNUSED(event)

    for (int i = 0; i < model()->columnCount(); ++i)
        resizeColumnToContents(i);
}

#include "problemwidget.moc"
