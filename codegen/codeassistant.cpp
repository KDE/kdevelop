/*
 * Copyright 2014 David Stevens <dgedstevens@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "codeassistant.h"

#include <kdebug.h>

#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KTextEditor/Range>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iuicontroller.h>

#include <language/backgroundparser/backgroundparser.h>
#include <language/backgroundparser/parsejob.h>

#include "clangsignatureassistant.h"

#include <KSharedPtr>

#include <QTimer>
#include <QHash>

using namespace KDevelop;
using namespace KTextEditor;

CodeAssistant::CodeAssistant()
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true),
    m_timer->setInterval(333);

    connect(m_timer, SIGNAL(timeout()), SLOT(timeout()));
    connect(KDevelop::ICore::self()->documentController(),
            SIGNAL(documentLoaded(KDevelop::IDocument*)),
            SLOT(documentLoaded(KDevelop::IDocument*)));
    connect(KDevelop::ICore::self()->documentController(),
            SIGNAL(documentActivated(KDevelop::IDocument*)),
            SLOT(documentActivated(KDevelop::IDocument*)));

    foreach (IDocument* document, ICore::self()->documentController()->openDocuments()) {
        documentLoaded(document);
    }
};

void CodeAssistant::timeout()
{
    m_insertRange = false;
    QMetaObject::invokeMethod(this, "eventuallyStartAssistant", Qt::QueuedConnection);
}

void CodeAssistant::documentLoaded(IDocument* document)
{
    if (document->textDocument()) {
        ///@todo Make these connections non-queued, and then reach forward using a QPointer,
        /// since else a crash may happen when the document is destroyed before the message is processed
        connect(document->textDocument(),
                SIGNAL(textInserted(KTextEditor::Document*, KTextEditor::Range)),
                SLOT(textInserted(KTextEditor::Document*, KTextEditor::Range)));
        connect(document->textDocument(),
                SIGNAL(textRemoved(KTextEditor::Document*, KTextEditor::Range, QString)),
                SLOT(textRemoved(KTextEditor::Document*, KTextEditor::Range, QString)));
    }
}

void CodeAssistant::documentActivated(IDocument* doc)
{
    if (doc)
        m_currentDocument = IndexedString(doc->url());

    if (m_currentView) {
        disconnect(m_currentView.data(),
                   SIGNAL(cursorPositionChanged(KTextEditor::View*, KTextEditor::Cursor)),
                   this, SLOT(cursorPositionChanged(KTextEditor::View*, KTextEditor::Cursor)));
        m_currentView.clear();
    }

    if (m_timer->isActive()) {
        m_timer->stop();
    }

    if (doc->textDocument()) {
        m_currentView = doc->textDocument()->activeView();
        if (m_currentView) {
            connect(m_currentView.data(),
                    SIGNAL(cursorPositionChanged(KTextEditor::View*, KTextEditor::Cursor)),
                    SLOT(cursorPositionChanged(KTextEditor::View*, KTextEditor::Cursor)));
        }
    }
}

void CodeAssistant::cursorPositionChanged(View*, const Cursor& pos)
{
    if (m_activeAssistant && m_assistantStartedAt.isValid()
        && abs(m_assistantStartedAt.line() - pos.line()) >= 1)
    {
        m_activeAssistant->doHide();
    }
}

void CodeAssistant::hideAssistant()
{
    if (m_activeAssistant) {
        ICore::self()->uiController()->popUpAssistant(IAssistant::Ptr(nullptr));
        m_activeAssistant = KSharedPtr<KDevelop::IAssistant>();
    }
};

void CodeAssistant::textInserted(KTextEditor::Document* document, const KTextEditor::Range& range)
{
    //We need to coalesce consecutive insertions into a single insertion, to prevent the position
    //of the insertions from diverging too much from what clang has parsed in the translation units
    if (m_timer->isActive()) {
        m_timer->stop();
    }
    if (m_insertRange && document == m_eventualDocument.data() && m_eventualRange.end() == range.start()) {
        m_eventualRange = KTextEditor::Range(m_eventualRange.start(), range.end());
    } else {
        m_eventualRange = range;
        m_eventualDocument = document;
    }
    m_insertRange = true;
    m_timer->start();
}

void CodeAssistant::textRemoved(KTextEditor::Document* document, const KTextEditor::Range& range,
                                     const QString& /*removedText*/)
{
    if (m_timer->isActive()) {
        m_timer->stop();
    }
    //Handle the simple case where the user backspaces over recently inserted text
    if (m_insertRange) {
        if (document == m_eventualDocument.data() && m_eventualRange.end() == range.end()) {
            if (m_eventualRange.start() < range.start()) {
                m_eventualRange = KTextEditor::Range(m_eventualRange.start(), range.start());
                m_timer->start();
                return;
            } else if (m_eventualRange.start() == range.start()) {
                m_insertRange = false;
                return;
            } else {
                m_insertRange = false;
                m_eventualRange = KTextEditor::Range(range.start(), m_eventualRange.start());
            }
        }
    } else {
        m_insertRange = false;
        m_eventualDocument = document;
        m_eventualRange = range;
    }

    QMetaObject::invokeMethod(this, "eventuallyStartAssistant", Qt::QueuedConnection);
}

void CodeAssistant::deleteRenameAssistantsForDocument(Document* document)
{
    if (m_sigAssistants.contains(document)) {
        for (auto i = m_sigAssistants[document].data()->begin(); i != m_sigAssistants[document].data()->end(); ++i) {
            if (m_activeAssistant && i.value().data() == m_activeAssistant.data()) {
                hideAssistant();
            }
            i.value().clear();
        }
        m_sigAssistants.remove(document);
    }
}

void CodeAssistant::eventuallyStartAssistant()
{
    if (!m_eventualDocument)
        return;

    View* view = m_eventualDocument.data()->activeView();
    //Eventually pop up an assistant
    if (!view)
        return;

    QHash<KTextEditor::View*, KSharedPtr<ClangSignatureAssistant>> *inner = m_sigAssistants[m_eventualDocument.data()].data();
    if (!inner) {
        inner = new QHash<KTextEditor::View*, KSharedPtr<ClangSignatureAssistant>>();
        m_sigAssistants[m_eventualDocument.data()] = QSharedPointer<QHash<KTextEditor::View*, KSharedPtr<ClangSignatureAssistant>>>(inner);
    }

    ClangSignatureAssistant *sigAssistant = (*inner)[view].data();
    if (!sigAssistant) {
        sigAssistant =  new ClangSignatureAssistant(view);
        (*inner)[view].attach(sigAssistant);
        connect(m_eventualDocument.data(),
                SIGNAL(aboutToClose(KTextEditor::Document*)),
                SLOT(deleteRenameAssistantsForDocument(KTextEditor::Document*)));
    }
    sigAssistant->textChanged(m_eventualRange);

    if (sigAssistant->isUseful()) {
        startAssistant(KSharedPtr<IAssistant>(sigAssistant));
    } else if (m_activeAssistant) {
//         m_activeAssistant->doHide();
//         ICore::self()->uiController()->popUpAssistant(IAssistant::Ptr(nullptr));
    }

    // optimize, esp. for setText() calls as done in e.g. reformat source
    // only start the assitant once for multiple textRemoved/textInserted signals
    m_eventualDocument.clear();
    m_eventualRange = Range::invalid();
}

void CodeAssistant::startAssistant(IAssistant::Ptr assistant)
{
    if (m_activeAssistant) {
        m_activeAssistant->doHide();
    }

    if (!m_currentView)
        return;

    m_activeAssistant = assistant;
    if (m_activeAssistant) {
        connect(m_activeAssistant.data(), SIGNAL(hide()), SLOT(hideAssistant()), Qt::DirectConnection);
        ICore::self()->uiController()->popUpAssistant(IAssistant::Ptr(m_activeAssistant.data()));

        m_assistantStartedAt =  m_currentView.data()->cursorPosition();
    }
}

#include "moc_codeassistant.cpp"
