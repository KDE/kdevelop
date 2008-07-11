/*
 * KDevelop Problem Reporter
 *
 * Copyright 2006 Adam Treat <treat@kde.org>
 * Copyright 2006-2007 Hamish Rodda <rodda@kde.org>
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

#include "problemreporterplugin.h"

#include <QTreeWidget>

#include <klocale.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>

#include <KTextEditor/Document>

#include <icore.h>
#include <iuicontroller.h>
#include <idocumentcontroller.h>

#include "language/backgroundparser/parsejob.h"
#include "language/editor/editorintegrator.h"
#include "interfaces/ilanguagecontroller.h"
#include "language/backgroundparser/backgroundparser.h"
#include "language/duchain/duchainlock.h"
#include "language/duchain/duchain.h"

#include "problemhighlighter.h"
#include "problemwidget.h"

K_PLUGIN_FACTORY(KDevProblemReporterFactory, registerPlugin<ProblemReporterPlugin>(); )
K_EXPORT_PLUGIN(KDevProblemReporterFactory("kdevproblemreporter"))

using namespace KDevelop;

class ProblemReporterFactory: public KDevelop::IToolViewFactory
{
public:
  ProblemReporterFactory(ProblemReporterPlugin *plugin): m_plugin(plugin) {}

  virtual QWidget* create(QWidget *parent = 0)
  {
    return new ProblemWidget(parent, m_plugin);
  }

  virtual Qt::DockWidgetArea defaultPosition()
  {
    return Qt::BottomDockWidgetArea;
  }

  virtual QString id() const
  {
    return "org.kdevelop.ProblemReporterView";
  }

private:
  ProblemReporterPlugin *m_plugin;
};

ProblemReporterPlugin::ProblemReporterPlugin(QObject *parent, const QVariantList&)
    : KDevelop::IPlugin(KDevProblemReporterFactory::componentData(), parent)
    , m_factory(new ProblemReporterFactory(this))
{
  core()->uiController()->addToolView(i18n("Problems"), m_factory);
  setXMLFile( "kdevproblemreporter.rc" );

  connect(EditorIntegrator::notifier(), SIGNAL(documentAboutToBeDeleted(KTextEditor::Document*)), SLOT(documentAboutToBeDeleted(KTextEditor::Document*)));
  connect(ICore::self()->documentController(), SIGNAL(documentLoaded(KDevelop::IDocument*)), this, SLOT(documentLoaded(KDevelop::IDocument*)));
  connect(ICore::self()->languageController()->backgroundParser(), SIGNAL(parseJobFinished(KDevelop::ParseJob*)), this, SLOT(parseJobFinished(KDevelop::ParseJob*)), Qt::DirectConnection);
}

ProblemReporterPlugin::~ProblemReporterPlugin()
{
  qDeleteAll(m_highlighters);
}

void ProblemReporterPlugin::unload()
{
  core()->uiController()->removeToolView(m_factory);
}

void ProblemReporterPlugin::documentAboutToBeDeleted(KTextEditor::Document* doc)
{
  QMutableHashIterator<IndexedString, ProblemHighlighter*> it = m_highlighters;

  IndexedString url(doc->url().pathOrUrl());

  if (m_highlighters.contains(url))
    delete m_highlighters.take(url);
}

void ProblemReporterPlugin::documentLoaded(KDevelop::IDocument* document)
{
  if (document->textDocument())
    m_highlighters.insert(IndexedString(document->url().pathOrUrl()), new ProblemHighlighter(document->textDocument()));
}

void ProblemReporterPlugin::parseJobFinished(KDevelop::ParseJob* parseJob)
{
  if (m_highlighters.contains(parseJob->document())) {
    ProblemHighlighter* ph = m_highlighters[parseJob->document()];
    if (!ph)
      return;

    QList<ProblemPointer> allProblems;
    QSet<TopDUContext*> hadContexts;

    {
      DUChainReadLocker lock(DUChain::lock());
      if (!parseJob->duChain())
        return;

      ProblemWidget::collectProblems(allProblems, parseJob->duChain(), hadContexts);
    }

    ph->setProblems(allProblems);
  }
}

#include "problemreporterplugin.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
