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

#ifndef TESTVARIABLECONTROLLER_H
#define TESTVARIABLECONTROLLER_H

#include "testsexport.h"

#include <debugger/interfaces/ivariablecontroller.h>

namespace KDevelop {
/**
 * Dummy VariableController that does nothing.
 */
class KDEVPLATFORMTESTS_EXPORT TestVariableController
    : public IVariableController
{
    Q_OBJECT

public:
    explicit TestVariableController(IDebugSession* parent);

    Variable* createVariable(TreeModel* model, TreeItem* parent,
                             const QString& expression,
                             const QString& display = {}) override;

    KTextEditor::Range expressionRangeUnderCursor(KTextEditor::Document* doc,
                                                  const KTextEditor::Cursor& cursor) override;

    void addWatch(Variable* variable) override;
    void addWatchpoint(Variable* variable) override;

    /**
     * Number of times update() has been called since creation.
     */
    int updatedTimes() const;

protected:
    void update() override;

private:
    int m_updatedTimes;
};
} // end of namespace KDevelop
#endif // TESTVARIABLECONTROLLER_H
