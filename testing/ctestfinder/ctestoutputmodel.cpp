/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "ctestoutputmodel.h"
#include "ctestsuite.h"
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>

#include <KDebug>

using namespace KDevelop;

K_GLOBAL_STATIC_WITH_ARGS(QRegExp, testCaseExpression, ("::([a-z][a-zA-Z0-9]+)\\(\\)"));
K_GLOBAL_STATIC_WITH_ARGS(QRegExp, locationExpression, ("Loc:\\s+\\[(.+)\\((\\d+)\\)\\]"));

CTestOutputModel::CTestOutputModel(CTestSuite* suite, QObject* parent): OutputModel(parent),
m_suite(suite)
{

}

CTestOutputModel::~CTestOutputModel()
{

}

QModelIndex CTestOutputModel::previousHighlightIndex(const QModelIndex& currentIndex)
{
    const QAbstractItemModel* model = currentIndex.model();
    const int rows = currentIndex.model()->rowCount();
    const int currentRow = currentIndex.row();
    int i = (currentIndex.row() - 1) % rows;
    for (;i != currentRow; --i)
    {
        if (isHighlighted(model->index(i, 0)))
        {
            return model->index(i,0);
        }
    }
    return QModelIndex();
}

QModelIndex CTestOutputModel::nextHighlightIndex(const QModelIndex& currentIndex)
{
    const QAbstractItemModel* model = currentIndex.model();
    const int rows = currentIndex.model()->rowCount();
    const int currentRow = currentIndex.row();
    int i = (currentIndex.row() + 1) % rows;
    for (;i != currentRow; ++i)
    {
        if (isHighlighted(model->index(i, 0)))
        {
            return model->index(i,0);
        }
    }
    return QModelIndex();
}

void CTestOutputModel::activate(const QModelIndex& index)
{
    const QString line = index.data().toString();
    if (testCaseExpression->indexIn(line) > -1)
    {
        const QString testCase = testCaseExpression->cap(1);
        DUChainReadLocker locker(DUChain::lock());
        Declaration* declaration = m_suite->caseDeclaration(testCase).data();
        if (!declaration)
        {
            return;
        }
        
        KUrl url = declaration->url().toUrl();
        KTextEditor::Cursor cursor = declaration->rangeInCurrentRevision().start.textCursor();
        locker.unlock();
        
        ICore::self()->documentController()->openDocument(url, cursor);
    } 
    else if (locationExpression->indexIn(line) > -1)
    {
        const KUrl url = locationExpression->cap(1);
        int line = locationExpression->cap(2).toInt();
        ICore::self()->documentController()->openDocument(url, KTextEditor::Cursor(line-1, 0));
    }
}

bool CTestOutputModel::isHighlighted(const QModelIndex& index)
{
    const QString line = index.data().toString();
    return line.startsWith("PASS") || line.startsWith("FAIL") || line.startsWith("XFAIL");
}
