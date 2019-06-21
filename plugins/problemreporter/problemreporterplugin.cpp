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
#include <KPluginFactory>

#include <KTextEditor/Document>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>

#include <interfaces/ilanguagecontroller.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <util/kdevstringhandler.h>

#include "problemhighlighter.h"
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
#include <debug.h>

#include <shell/problem.h>

K_PLUGIN_FACTORY_WITH_JSON(KDevProblemReporterFactory, "kdevproblemreporter.json",
                           registerPlugin<ProblemReporterPlugin>();)

using namespace KDevelop;

class ProblemReporterFactory : public KDevelop::IToolViewFactory
{
public:
    QWidget* create(QWidget* parent = nullptr) override
    {
        Q_UNUSED(parent);

        auto* v = new ProblemsView();
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
    pms->addModel(QStringLiteral("Parser"), i18n("Parser"), m_model);
    core()->uiController()->addToolView(i18n("Problems"), m_factory);
    setXMLFile(QStringLiteral("kdevproblemreporter.rc"));

    connect(ICore::self()->documentController(), &IDocumentController::documentClosed, this,
            &ProblemReporterPlugin::documentClosed);
    connect(ICore::self()->documentController(), &IDocumentController::textDocumentCreated, this,
            &ProblemReporterPlugin::textDocumentCreated);
    connect(ICore::self()->documentController(), &IDocumentController::documentActivated, this,
            &ProblemReporterPlugin::documentActivated);
    connect(DUChain::self(), &DUChain::updateReady,
            this, &ProblemReporterPlugin::updateReady);
    connect(ICore::self()->languageController()->staticAssistantsManager(), &StaticAssistantsManager::problemsChanged,
            this, &ProblemReporterPlugin::updateHighlight);
    connect(pms, &ProblemModelSet::showRequested, this, &ProblemReporterPlugin::showModel);
    connect(pms, &ProblemModelSet::problemsChanged, this, &ProblemReporterPlugin::updateOpenedDocumentsHighlight);
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
    m_reHighlightNeeded.remove(url);
}

void ProblemReporterPlugin::textDocumentCreated(KDevelop::IDocument* document)
{
    Q_ASSERT(document->textDocument());
    m_highlighters.insert(IndexedString(document->url()), new ProblemHighlighter(document->textDocument()));
    DUChain::self()->updateContextForUrl(IndexedString(document->url()),
                                         KDevelop::TopDUContext::AllDeclarationsContextsAndUses, this);
}

void ProblemReporterPlugin::documentActivated(KDevelop::IDocument* document)
{
  IndexedString documentUrl(document->url());

  const auto neededIt = m_reHighlightNeeded.find(documentUrl);
  if (neededIt != m_reHighlightNeeded.end()) {
    m_reHighlightNeeded.erase(neededIt);
    updateHighlight(documentUrl);
  }
}

void ProblemReporterPlugin::updateReady(const IndexedString& url, const KDevelop::ReferencedTopDUContext&)
{
    m_model->problemsUpdated(url);
    updateHighlight(url);
}

void ProblemReporterPlugin::updateHighlight(const KDevelop::IndexedString& url)
{
    ProblemHighlighter* ph = m_highlighters.value(url);
    if (!ph)
        return;

    KDevelop::ProblemModelSet* pms(core()->languageController()->problemModelSet());
    QVector<IProblem::Ptr> documentProblems;

    const auto models = pms->models();
    for (const ModelData& modelData : models) {
        documentProblems += modelData.model->problems({url});
    }

    ph->setProblems(documentProblems);
}

void ProblemReporterPlugin::showModel(const QString& id)
{
    auto w = qobject_cast<ProblemsView*>(core()->uiController()->findToolView(i18n("Problems"), m_factory));
    if (w)
      w->showModel(id);
}

KDevelop::ContextMenuExtension ProblemReporterPlugin::contextMenuExtension(KDevelop::Context* context, QWidget* parent)
{
    KDevelop::ContextMenuExtension extension;

    auto* editorContext = dynamic_cast<KDevelop::EditorContext*>(context);
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
            const auto problems = top->problems();
            for (auto& problem : problems) {
                if (problem->range().contains(
                        top->transformToLocalRevision(KTextEditor::Cursor(editorContext->position())))) {
                    KDevelop::IAssistant::Ptr solution = problem->solutionAssistant();
                    if (solution) {
                        title = solution->title();
                        const auto solutionActions = solution->actions();
                        for (auto& action : solutionActions) {
                            actions << action->toQAction(parent);
                        }
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

            auto* menu = new QMenu(text, parent);
            for (QAction* action : qAsConst(actions)) {
                menu->addAction(action);
            }

            extension.addAction(ContextMenuExtension::ExtensionGroup, menu->menuAction());
        }
    }
    return extension;
}

void ProblemReporterPlugin::updateOpenedDocumentsHighlight()
{
    const auto openDocuments = core()->documentController()->openDocuments();
    for (auto* document : openDocuments) {
        // Skip non-text documents.
        // This also fixes crash caused by calling updateOpenedDocumentsHighlight() method without
        // any opened documents. In this case documentController()->openDocuments() returns single
        // (non-text) document with url like file:///tmp/kdevelop_QW2530.patch which has fatal bug:
        // if we call isActive() method from this document the crash will happens.
        if (!document->isTextDocument())
            continue;

        IndexedString documentUrl(document->url());

        if (document->isActive())
            updateHighlight(documentUrl);
        else
            m_reHighlightNeeded.insert(documentUrl);
    }
}

#include "problemreporterplugin.moc"
