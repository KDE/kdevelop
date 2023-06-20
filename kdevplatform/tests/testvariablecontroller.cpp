/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "testvariablecontroller.h"

#include <KTextEditor/Range>

using namespace KDevelop;
using KTextEditor::Range;
using KTextEditor::Document;
using KTextEditor::Cursor;

TestVariableController::TestVariableController(IDebugSession* parent)
    : IVariableController(parent)
    , m_updatedTimes(0)
{
}

Variable* TestVariableController::createVariable(TreeModel* model, TreeItem* parent,
                                                 const QString& expression,
                                                 const QString& display)
{
    Q_UNUSED(model);
    Q_UNUSED(parent);
    Q_UNUSED(expression);
    Q_UNUSED(display);

    return nullptr;
}

Range TestVariableController::expressionRangeUnderCursor(Document* doc,
                                                         const Cursor& cursor)
{
    Q_UNUSED(doc);
    Q_UNUSED(cursor);

    return {};
}

void TestVariableController::addWatch(Variable* variable)
{
    Q_UNUSED(variable);
}

void TestVariableController::addWatchpoint(Variable* variable)
{
    Q_UNUSED(variable);
}

void TestVariableController::update()
{
    ++m_updatedTimes;
}

int TestVariableController::updatedTimes() const
{
    return m_updatedTimes;
}

#include "moc_testvariablecontroller.cpp"
