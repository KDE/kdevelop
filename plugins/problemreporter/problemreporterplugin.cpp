/*
 * KDevelop Problem Reporter
 *
 * Copyright 2006 Adam Treat <treat@kde.org>
 * Copyright 2006-2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
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

#include <QMenu>

#include <KLocalizedString>
#include <kpluginfactory.h>
#include <kaboutdata.h>
#include <kpluginloader.h>

#include <KTextEditor/Document>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>

#include <interfaces/ilanguagecontroller.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <util/kdevstringhandler.h>

#include "problemhighlighter.h"
#include "problemtreeview.h"
#include "problemreportermodel.h"
#include "language/assistant/staticassistantsmanager.h"
#include <interfaces/context.h>
#include <language/interfaces/editorcontext.h>
#include <language/duchain/duchainutils.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iassistant.h>
#include <QAction>

#include "shell/problemmodelset.h"
#include "problemsview.h"

#include <shell/problem.h>

Q_LOGGING_CATEGORY(PLUGIN_PROBLEMREPORTER, "kdevplatform.plugins.problemreporter")
K_PLUGIN_FACTORY_WITH_JSON(KDevProblemReporterFactory, "kdevproblemreporter.json",
                           registerPlugin<ProblemReporterPlugin>();)

using namespace KDevelop;

class ProblemReporterFactory : public KDevelop::IToolViewFactory
{
public:
    QWidget* create(QWidget* parent = 0) override
    {
        Q_UNUSED(parent);

        ProblemsView* v = new ProblemsView();
        v->load();
        return v;
    }

    Qt::DockWidgetArea defaultPosition() override { return Qt::BottomDockWidgetArea; }

    QString id() const override { return QStringLiteral("org.kdevelop.ProblemReporterView"); }
};

ProblemReporterPlugin::ProblemReporterPlugin(QObject* parent, const QVariantList&)
    : KDevelop::IPlugin(QStringLiteral("kdevproblemreporter"), parent)
    , m_factory(new ProblemReporterFactory)
    , m_model(new ProblemReporterModel(this))
{
    KDevelop::ProblemModelSet* pms = core()->languageController()->problemModelSet();
    pms->addModel(QStringLiteral("Parser"), m_model);
    core()->uiController()->addToolView(i18n("Problems"), m_factory);
    setXMLFile(QStringLiteral("kdevproblemreporter.rc"));

    connect(ICore::self()->documentController(), &IDocumentController::documentClosed, this,
            &ProblemReporterPlugin::documentClosed);
    connect(ICore::self()->documentController(), &IDocumentController::textDocumentCreated, this,
            &ProblemReporterPlugin::textDocumentCreated);
    connect(DUChain::self(), &DUChain::updateReady,
            this, &ProblemReporterPlugin::updateReady);
}

ProblemReporterPlugin::~ProblemReporterPlugin()
{
    qDeleteAll(m_highlighters);
}

ProblemReporterModel* ProblemReporterPlugin::model() const
{
    return m_model;
}

void ProblemReporterPlugin::unload()
{
    KDevelop::ProblemModelSet* pms = KDevelop::ICore::self()->languageController()->problemModelSet();
    pms->removeModel(QStringLiteral("Parser"));

    core()->uiController()->removeToolView(m_factory);
}

void ProblemReporterPlugin::documentClosed(IDocument* doc)
{
    if (!doc->textDocument())
        return;

    IndexedString url(doc->url());
    delete m_highlighters.take(url);
}

void ProblemReporterPlugin::textDocumentCreated(KDevelop::IDocument* document)
{
    Q_ASSERT(document->textDocument());
    m_highlighters.insert(IndexedString(document->url()), new ProblemHighlighter(document->textDocument()));
    DUChain::self()->updateContextForUrl(IndexedString(document->url()),
                                         KDevelop::TopDUContext::AllDeclarationsContextsAndUses, this);
}

void ProblemReporterPlugin::updateReady(const IndexedString& url, const KDevelop::ReferencedTopDUContext& top)
{
    {
      DUChainWriteLocker lock(DUChain::lock(), 300);
      if ( !lock.locked() ) {
        return;
      }
      ICore::self()->languageController()->staticAssistantsManager()->notifyAssistants(url, top);
      auto assistantProblems = ICore::self()->languageController()->staticAssistantsManager()->problemsForContext(top);
      Q_FOREACH ( const auto p, assistantProblems ) {
        qDebug() << "adding problem:" << p << "assistant:" << p->solutionAssistant().data() << p->solutionAssistant()->actions().size();
        top->addProblem(p);
      }
    }

    m_model->problemsUpdated(url);
    ProblemHighlighter* ph = m_highlighters.value(url);
    if (ph) {
        auto allProblems = m_model->problems(url, false);
        ph->setProblems(allProblems);
    }
}

KDevelop::ContextMenuExtension ProblemReporterPlugin::contextMenuExtension(KDevelop::Context* context)
{
    KDevelop::ContextMenuExtension extension;

    KDevelop::EditorContext* editorContext = dynamic_cast<KDevelop::EditorContext*>(context);
    if (editorContext) {
        DUChainReadLocker lock(DUChain::lock(), 1000);
        if (!lock.locked()) {
            qCDebug(PLUGIN_PROBLEMREPORTER) << "failed to lock duchain in time";
            return extension;
        }

        QString title;
        QList<QAction*> actions;

        TopDUContext* top = DUChainUtils::standardContextForUrl(editorContext->url());
        if (top) {
            foreach (KDevelop::ProblemPointer problem, top->problems()) {
                if (problem->range().contains(
                        top->transformToLocalRevision(KTextEditor::Cursor(editorContext->position())))) {
                    KDevelop::IAssistant::Ptr solution = problem->solutionAssistant();
                    if (solution) {
                        title = solution->title();
                        foreach (KDevelop::IAssistantAction::Ptr action, solution->actions())
                            actions << action->toKAction();
                    }
                }
            }
        }

        if (!actions.isEmpty()) {
            QString text;
            if (title.isEmpty())
                text = i18n("Solve Problem");
            else {
                text = i18n("Solve: %1", KDevelop::htmlToPlainText(title));
            }

            QAction* menuAction = new QAction(text, 0);
            QMenu* menu(new QMenu(text, 0));
            menuAction->setMenu(menu);
            foreach (QAction* action, actions)
                menu->addAction(action);

            extension.addAction(ContextMenuExtension::ExtensionGroup, menuAction);
        }
    }
    return extension;
}

#include "problemreporterplugin.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
