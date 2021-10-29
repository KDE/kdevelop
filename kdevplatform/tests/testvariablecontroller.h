/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
