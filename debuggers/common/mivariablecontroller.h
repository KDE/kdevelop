/*
 * Variable controller implementation common to MI based debugger
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2008 Vladimir Prus <ghost@cs.msu.su>
 * Copyright 2009 Niko Sams <niko.sams@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
    MIVariableController( MIDebugSession* parent);

    KDevelop::Variable* createVariable(KDevelop::TreeModel* model, KDevelop::TreeItem* parent,
                                       const QString& expression,
                                       const QString& display = "") override;

    KTextEditor::Range expressionRangeUnderCursor(KTextEditor::Document* doc,
                                                  const KTextEditor::Cursor& cursor) override;

    void addWatch(KDevelop::Variable* variable) override;
    void addWatchpoint(KDevelop::Variable* variable) override;
    void update() override;

protected:
    void updateLocals();

private slots:
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
