/*
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2006-2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
#include "probleminlinenoteprovider.h"
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
        auto* const v = new ProblemsView(parent);
        v->load();
        return v;
    }

    Qt::DockWidgetArea defaultPosition() const override
    {
        return Qt::BottomDockWidgetArea;
    }

    QString id() const override { return QStringLiteral("org.kdevelop.ProblemReporterView"); }
};

class ProblemReporterPlugin::ProblemVisualizer
{
public:
    /**
     * Construct a problem visualizer.
     *
     * @param document a non-null document to be embellished that must
     *        remain valid throughout this visualizer's lifetime.
     */
    explicit ProblemVisualizer(KTextEditor::Document* document)
        : m_highlighter(document)
        , m_inlineNoteProvider(document)
    {}

    KTextEditor::Document* document() const
    {
        return m_highlighter.document();
    }

    void setProblems(const QVector<IProblem::Ptr>& problems)
    {
        m_highlighter.setProblems(problems);
        m_inlineNoteProvider.setProblems(problems);
    }

private:
    ProblemHighlighter m_highlighter;
    ProblemInlineNoteProvider m_inlineNoteProvider;
};

ProblemReporterPlugin::ProblemReporterPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : KDevelop::IPlugin(QStringLiteral("kdevproblemreporter"), parent, metaData)
    , m_factory(new ProblemReporterFactory)
    , m_model(new ProblemReporterModel(this))
{
    KDevelop::ProblemModelSet* pms = core()->languageController()->problemModelSet();
    pms->addModel(QStringLiteral("Parser"), i18n("Parser"), m_model);
    core()->uiController()->addToolView(i18nc("@title:window", "Problems"), m_factory);
    setXMLFile(QStringLiteral("kdevproblemreporter.rc"));

    connect(ICore::self()->documentController(), &IDocumentController::documentClosed, this,
            &ProblemReporterPlugin::documentClosed);
    connect(ICore::self()->documentController(), &IDocumentController::textDocumentCreated, this,
            &ProblemReporterPlugin::textDocumentCreated);
    connect(ICore::self()->documentController(), &IDocumentController::documentUrlChanged, this,
            &ProblemReporterPlugin::documentUrlChanged);
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
    qDeleteAll(m_visualizers);
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

    const IndexedString url(doc->url());

    const auto it = m_visualizers.constFind(url);
    if (it == m_visualizers.cend()) {
        qCDebug(PLUGIN_PROBLEMREPORTER) << "closed an unregistered text document:" << doc << doc->url().toString();
        return;
    }

    if (it.value()->document() != doc->textDocument()) {
        // doc is being renamed, DocumentControllerPrivate::changeDocumentUrl() is closing it
        // because of a conflict with another open modified document at doc's new URL.
        // documentUrlChanged(doc, ...) will be invoked soon and will remove doc's visualizer. Nothing to do here.
        qCDebug(PLUGIN_PROBLEMREPORTER) << "closed a text document that shares another text document's URL:" << doc
                                        << doc->url().toString();
        return;
    }

    delete it.value();
    m_visualizers.erase(it);
    m_reHighlightNeeded.remove(url);
}

void ProblemReporterPlugin::textDocumentCreated(KDevelop::IDocument* document)
{
    Q_ASSERT(document->textDocument());
    const IndexedString documentUrl(document->url());

    Q_ASSERT(!m_visualizers.contains(documentUrl));
    m_visualizers.insert(documentUrl, new ProblemVisualizer{document->textDocument()});

    DUChain::self()->updateContextForUrl(documentUrl,
                                         KDevelop::TopDUContext::AllDeclarationsContextsAndUses, this);
}

void ProblemReporterPlugin::documentUrlChanged(IDocument* document, const QUrl& previousUrl)
{
    if (!document->textDocument())
        return;

    qCDebug(PLUGIN_PROBLEMREPORTER) << "document URL changed from" << previousUrl.toString() << "to"
                                    << document->url().toString();

    const IndexedString previousUrlIndexed(previousUrl);
    const auto it = m_visualizers.constFind(previousUrlIndexed);
    if (it == m_visualizers.cend()) {
        qCWarning(PLUGIN_PROBLEMREPORTER)
            << "a visualizer for renamed document is missing:" << document->textDocument();
        return;
    }
    Q_ASSERT(it.value()->document() == document->textDocument());

    m_reHighlightNeeded.remove(previousUrlIndexed);

    auto* const visualizer = it.value();
    m_visualizers.erase(it);

    const IndexedString currentUrl{document->url()};
    if (m_visualizers.contains(currentUrl)) {
        // The renamed document must have been closed already in DocumentControllerPrivate::changeDocumentUrl()
        // because of a conflict with another open modified document at its new URL. See a similar comment in
        // ProblemReporterPlugin::documentClosed(). Just destroy document's obsolete visualizer here.
        delete visualizer;
        qCDebug(PLUGIN_PROBLEMREPORTER) << "the renamed document's URL equals another document's URL:" << document;
        return;
    }
    m_visualizers.insert(currentUrl, visualizer);
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
    auto* const visualizer = m_visualizers.value(url);
    if (!visualizer)
        return;

    KDevelop::ProblemModelSet* pms(core()->languageController()->problemModelSet());
    QVector<IProblem::Ptr> documentProblems;

    const auto models = pms->models();
    for (const ModelData& modelData : models) {
        documentProblems += modelData.model->problems({url});
    }

    visualizer->setProblems(documentProblems);
}

void ProblemReporterPlugin::showModel(const QString& id)
{
    auto w = qobject_cast<ProblemsView*>(core()->uiController()->findToolView(i18nc("@title:window", "Problems"), m_factory));
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
                text = i18nc("@action:inmenu", "Solve Problem");
            else {
                text = i18nc("@action:inmenu", "Solve: %1", KDevelop::htmlToPlainText(title));
            }

            auto* menu = new QMenu(text, parent);
            for (QAction* action : std::as_const(actions)) {
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
        if (!document->textDocument())
            continue;

        IndexedString documentUrl(document->url());

        if (document->isActive())
            updateHighlight(documentUrl);
        else
            m_reHighlightNeeded.insert(documentUrl);
    }
}

#include "problemreporterplugin.moc"
#include "moc_problemreporterplugin.cpp"
