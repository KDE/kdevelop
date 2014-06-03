/*****************************************************************************
 *   Copyright 2011-2012 Sven Brauch <svenbrauch@googlemail.com>             *
 *                                                                           *
 *   This program is free software: you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation, either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>    *
 *****************************************************************************/

#ifndef QMLCOMPLETIONTEST_H
#define QMLCOMPLETIONTEST_H

#include <QtCore/QObject>

#include <language/editor/cursorinrevision.h>
#include <language/codecompletion/codecompletioncontext.h>

#include "codecompletion/model.h"

using namespace KDevelop;

namespace QmlJS {

struct CompletionParameters {
    DUContextPointer contextAtCursor;
    QString snip;
    QString remaining;
    CursorInRevision cursorAt;
};

class QmlCompletionTest : public QObject
{
    Q_OBJECT
    public:
        explicit QmlCompletionTest(QObject* parent = 0);
        void initShell();

        const QList<CompletionTreeItem*> invokeCompletionOn(const QString& initCode, const QString& invokeCode, bool qml);
        const CompletionParameters prepareCompletion(const QString& initCode, const QString& invokeCode, bool qml);
        const QList<CompletionTreeItem*> runCompletion(const CompletionParameters data);

        bool containsItemForDeclarationNamed(const QList< CompletionTreeItem* > items, QString itemName);
        bool declarationInCompletionList(const QString& initCode, const QString& invokeCode, QString itemName, bool qml);

    private slots:
        void testContainsDeclaration();
        void testContainsDeclaration_data();
        void testDoesNotContainDeclaration();
        void testDoesNotContainDeclaration_data();

    private:
        QList<CompletionTreeItemPointer> m_ptrs;
};

}
#endif
