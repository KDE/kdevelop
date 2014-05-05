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

#ifndef CODEASSISTANT_H
#define CODEASSISTANT_H

#include <QObject>

#include "codegenexport.h"
#include "qsharedpointer.h"
#include <interfaces/iassistant.h>

#include "clangsignatureassistant.h"

#include <KTextEditor/Range>

class QTimer;

namespace KTextEditor {
    class Cursor;
    class Document;
    class Range;
    class View;
}

namespace KDevelop {
    class IDocument;
}

typedef QWeakPointer<KTextEditor::Document> SafeDocumentPointer;

class KDEVCLANGCODEGEN_EXPORT CodeAssistant : QObject
{
    Q_OBJECT
public:
    CodeAssistant();

public slots:
    void hideAssistant();

private slots:
    void eventuallyStartAssistant();

    void documentLoaded(KDevelop::IDocument* doc);
    void documentActivated(KDevelop::IDocument* doc);
    void textInserted(KTextEditor::Document*, const KTextEditor::Range&);
    void textRemoved(KTextEditor::Document*, const KTextEditor::Range&, const QString& removedText);
    void cursorPositionChanged(KTextEditor::View*, const KTextEditor::Cursor&);
    void timeout();
    void deleteRenameAssistantsForDocument(KTextEditor::Document* document);

private:
    void startAssistant(KDevelop::IAssistant::Ptr assistant);


    KDevelop::IndexedString m_currentDocument;
    QWeakPointer<KTextEditor::View> m_currentView;
    KSharedPtr<KDevelop::IAssistant> m_activeAssistant;
    KTextEditor::Cursor m_assistantStartedAt;

    QHash<KTextEditor::Document*, QSharedPointer<QHash<KTextEditor::View*, KSharedPtr<ClangSignatureAssistant>>>> m_sigAssistants;

    SafeDocumentPointer m_eventualDocument;
    bool m_insertRange;
    KTextEditor::Range m_eventualRange;

    QTimer *m_timer;
};

#endif //CODEASSISTANT_H
