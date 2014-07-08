/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
   Copyright 2014 Kevin Funk <kfunk@kde.org>

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

#include "staticassistantsmanager.h"

#include <QTimer>

#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/ilanguage.h>

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainutils.h>

#include <language/backgroundparser/backgroundparser.h>
#include <language/backgroundparser/parsejob.h>
#include <language/duchain/problem.h>

using namespace KDevelop;
using namespace KTextEditor;

struct StaticAssistantsManager::Private
{
    Private(StaticAssistantsManager* qq)
        : q(qq)
    {
    }

    void eventuallyStartAssistant();
    void startAssistant(KDevelop::IAssistant::Ptr assistant);
    void checkAssistantForProblems(KDevelop::TopDUContext* top);

    void documentLoaded(KDevelop::IDocument*);
    void textInserted(KTextEditor::Document*, const KTextEditor::Range&);
    void textRemoved(KTextEditor::Document*, const KTextEditor::Range&, const QString& removedText);
    void parseJobFinished(KDevelop::ParseJob*);
    void documentActivated(KDevelop::IDocument*);
    void cursorPositionChanged(KTextEditor::View*, const KTextEditor::Cursor&);
    void timeout();

    StaticAssistantsManager* q;

    QPointer<KTextEditor::View> m_currentView;
    KTextEditor::Cursor m_assistantStartedAt;
    KDevelop::IndexedString m_currentDocument;
    QExplicitlySharedDataPointer<KDevelop::IAssistant> m_activeAssistant;
    QList<StaticAssistant::Ptr> m_registeredAssistants;
    bool m_activeProblemAssistant = false;
    QTimer* m_timer;

    SafeDocumentPointer m_eventualDocument;
    KTextEditor::Range m_eventualRange;
    QString m_eventualRemovedText;
};

StaticAssistantsManager::StaticAssistantsManager(QObject* parent)
    : QObject(parent)
    , d(new Private(this))
{
    d->m_timer = new QTimer(this);
    d->m_timer->setSingleShot(true);
    d->m_timer->setInterval(400);
    connect(d->m_timer, SIGNAL(timeout()), SLOT(timeout()));

    connect(KDevelop::ICore::self()->documentController(),
            SIGNAL(documentLoaded(KDevelop::IDocument*)),
            SLOT(documentLoaded(KDevelop::IDocument*)));
    connect(KDevelop::ICore::self()->documentController(),
            SIGNAL(documentActivated(KDevelop::IDocument*)),
            SLOT(documentActivated(KDevelop::IDocument*)));

    foreach (IDocument* document, ICore::self()->documentController()->openDocuments()) {
        d->documentLoaded(document);
    }
}

StaticAssistantsManager::~StaticAssistantsManager()
{
}

QExplicitlySharedDataPointer<IAssistant> StaticAssistantsManager::activeAssistant()
{
    return d->m_activeAssistant;
}

void StaticAssistantsManager::registerAssistant(const StaticAssistant::Ptr assistant)
{
    if (d->m_registeredAssistants.contains(assistant))
        return;

    d->m_registeredAssistants << assistant;
}

void StaticAssistantsManager::unregisterAssistant(const StaticAssistant::Ptr assistant)
{
    d->m_registeredAssistants.removeOne(assistant);
}

QList<StaticAssistant::Ptr> StaticAssistantsManager::registeredAssistants() const
{
    return d->m_registeredAssistants;
}

void StaticAssistantsManager::Private::documentLoaded(IDocument* document)
{
    if (document->textDocument()) {
        connect(document->textDocument(),
                SIGNAL(textInserted(KTextEditor::Document*,KTextEditor::Range)), q,
                SLOT(textInserted(KTextEditor::Document*,KTextEditor::Range)));
        connect(document->textDocument(),
                SIGNAL(textRemoved(KTextEditor::Document*,KTextEditor::Range,QString)), q,
                SLOT(textRemoved(KTextEditor::Document*,KTextEditor::Range,QString)));
    }
}

void StaticAssistantsManager::hideAssistant()
{
    d->m_activeAssistant = QExplicitlySharedDataPointer<KDevelop::IAssistant>();
    d->m_activeProblemAssistant = false;
}

void StaticAssistantsManager::Private::textInserted(Document* document, const Range& range)
{
    m_eventualDocument = document;
    m_eventualRange = range;
    m_eventualRemovedText.clear();
    QMetaObject::invokeMethod(q, "eventuallyStartAssistant", Qt::QueuedConnection);
}

void StaticAssistantsManager::Private::textRemoved(Document* document, const Range& range,
                                      const QString& removedText)
{
    m_eventualDocument = document;
    m_eventualRange = range;
    m_eventualRemovedText = removedText;
    QMetaObject::invokeMethod(q, "eventuallyStartAssistant", Qt::QueuedConnection);
}

void StaticAssistantsManager::Private::eventuallyStartAssistant()
{
    if (!m_eventualDocument) {
        return;
    }

    View* view = ICore::self()->documentController()->activeTextDocumentView();
    if (!view) {
        return;
    }

    ILanguage* language = ICore::self()->languageController()->languagesForUrl(m_eventualDocument.data()->url()).value(0);
    if (!language) {
        return;
    }

    kDebug() << "Trying to find assistants for language" << language->name();
    foreach (auto assistant, m_registeredAssistants) {
        if (assistant->supportedLanguage() != language->languageSupport())
            continue;

        // notify assistant about editor changes
        assistant->textChanged(view, m_eventualRange, m_eventualRemovedText);

        if (assistant->isUseful()) {
            startAssistant(IAssistant::Ptr(assistant.data()));
            break;
        }
    }

    // optimize, esp. for setText() calls as done in e.g. reformat source
    // only start the assitant once for multiple textRemoved/textInserted signals
    m_eventualDocument.clear();
    m_eventualRange = Range::invalid();
    m_eventualRemovedText.clear();
}

void StaticAssistantsManager::Private::startAssistant(IAssistant::Ptr assistant)
{
    if (m_activeAssistant) {
        m_activeAssistant->doHide();
    }

    if (!m_currentView)
        return;

    m_activeAssistant = assistant;
    if (m_activeAssistant) {
        connect(m_activeAssistant.data(), SIGNAL(hide()), q, SLOT(hideAssistant()), Qt::UniqueConnection);
        ICore::self()->uiController()->popUpAssistant(IAssistant::Ptr(m_activeAssistant.data()));

        m_assistantStartedAt =  m_currentView.data()->cursorPosition();
    }
}

void StaticAssistantsManager::Private::parseJobFinished(ParseJob* job)
{
    if (job->document() != m_currentDocument) {
        return;
    }

    if (m_activeAssistant) {
        if (m_activeProblemAssistant) {
            m_activeAssistant->doHide(); //Hide the assistant, as we will create a new one if the problem is still there
        } else {
            return;
        }
    }

    DUChainReadLocker lock(DUChain::lock(), 300);
    if (!lock.locked()) {
        return;
    }

    if (job->duChain()) {
        checkAssistantForProblems(job->duChain());
    }
}

void StaticAssistantsManager::Private::cursorPositionChanged(View*, const Cursor& pos)
{
    if (m_activeAssistant && m_assistantStartedAt.isValid()
        && abs(m_assistantStartedAt.line() - pos.line()) >= 1)
    {
        m_activeAssistant->doHide();
    }

    m_timer->start();
}

void StaticAssistantsManager::Private::documentActivated(IDocument* doc)
{
    if (doc) {
        m_currentDocument = IndexedString(doc->url());
    }

    connect(KDevelop::ICore::self()->languageController()->backgroundParser(),
            SIGNAL(parseJobFinished(KDevelop::ParseJob*)), q,
            SLOT(parseJobFinished(KDevelop::ParseJob*)),
            Qt::UniqueConnection);

    if (m_currentView) {
        disconnect(m_currentView.data(),
                   SIGNAL(cursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor)), q,
                   SLOT(cursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor)));
        m_currentView.clear();
    }

    m_currentView = ICore::self()->documentController()->activeTextDocumentView();

    if (m_currentView) {
        connect(m_currentView.data(),
                SIGNAL(cursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor)), q,
                SLOT(cursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor)));
    }
}

void StaticAssistantsManager::Private::checkAssistantForProblems(TopDUContext* top)
{
    foreach (ProblemPointer problem, top->problems()) {
        if (m_currentView && m_currentView.data()->cursorPosition().line() == problem->range().start.line) {
            IAssistant::Ptr solution = problem->solutionAssistant();
            if(solution) {
                startAssistant(solution);
                m_activeProblemAssistant = true;
                break;
            }
        }
    }
}

void StaticAssistantsManager::Private::timeout()
{
    if (!m_activeAssistant && m_currentView) {
        DUChainReadLocker lock(DUChain::lock(), 300);
        if (!lock.locked()) {
            return;
        }

        TopDUContext* top = DUChainUtils::standardContextForUrl(m_currentDocument.toUrl());
        if (top) {
            checkAssistantForProblems(top);
        }
    }
}

#include "moc_staticassistantsmanager.cpp"
