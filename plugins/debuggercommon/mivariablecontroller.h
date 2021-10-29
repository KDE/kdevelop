/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2008 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MIVARIABLECONTROLLER_H
#define MIVARIABLECONTROLLER_H

#include "dbgglobal.h"

#include <debugger/interfaces/ivariablecontroller.h>

namespace KDevMI {

namespace MI {
struct AsyncRecord;
struct ResultRecord;
struct Value;
}

class MIDebugSession;
class MIVariableController : public KDevelop::IVariableController
{
    Q_OBJECT

public:
    explicit MIVariableController( MIDebugSession* parent);

    KDevelop::Variable* createVariable(KDevelop::TreeModel* model, KDevelop::TreeItem* parent,
                                       const QString& expression,
                                       const QString& display = {}) override;

    KTextEditor::Range expressionRangeUnderCursor(KTextEditor::Document* doc,
                                                  const KTextEditor::Cursor& cursor) override;

    void addWatch(KDevelop::Variable* variable) override;
    void addWatchpoint(KDevelop::Variable* variable) override;
    void update() override;

protected:
    void updateLocals();

private Q_SLOTS:
    void programStopped(const MI::AsyncRecord &r);
    void stateChanged(KDevelop::IDebugSession::DebuggerState);

private:
    MIDebugSession* debugSession() const;


    void handleVarUpdate(const MI::ResultRecord& r);
    void addWatch(const MI::ResultRecord& r);
    void addWatchpoint(const MI::ResultRecord& r);

    void handleEvent(KDevelop::IDebugSession::event_t event) override;
};

} // end of namespace KDevMI

#endif // MIVARIABLECONTROLLER_H
