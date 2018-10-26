/*
 * This file is part of KDevelop
 *
 * Copyright 2016  Aetf <aetf@unlimitedcodeworks.xyz>
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
