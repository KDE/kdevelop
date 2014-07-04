/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2009 Niko Sams <niko.sams@gmail.com>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_IVARIABLECONTROLLER_H
#define KDEVPLATFORM_IVARIABLECONTROLLER_H

#include <QtCore/QObject>
class QString;

#include "debuggerexport.h"
#include "idebugsession.h"

namespace KTextEditor {
class Document;
class Cursor;
}

namespace KDevelop {

class IDebugSession;
class VariableCollection;
class Variable;
class TreeModel;
class TreeItem;

class KDEVPLATFORMDEBUGGER_EXPORT IVariableController : public QObject
{
    Q_OBJECT
public:
    IVariableController(IDebugSession* parent);

    /* Create a variable for the specified expression in the currentl
       thread and frame.  */     
    virtual Variable* createVariable(TreeModel* model, TreeItem* parent, 
                                     const QString& expression,
                                     const QString& display = "") = 0;

    virtual QString expressionUnderCursor(KTextEditor::Document* doc, const KTextEditor::Cursor& cursor) = 0;

    virtual void addWatch(Variable* variable) = 0;
    virtual void addWatchpoint(Variable* variable) = 0;

    enum UpdateType {
        UpdateNone    = 0x0,
        UpdateLocals  = 0x1,
        UpdateWatches = 0x2
    };
    Q_DECLARE_FLAGS(UpdateTypes, UpdateType)
    void setAutoUpdate(QFlags<UpdateType> autoUpdate);
    QFlags<UpdateType> autoUpdate();

protected:
    /**
     * Convenience function that returns the VariableCollection
     **/
    VariableCollection *variableCollection();

    /**
     * Convenience function that returns the used DebugSession
     **/
    IDebugSession *session() const;

    virtual void update() = 0;

    virtual void handleEvent(IDebugSession::event_t event);
    friend class IDebugSession;

private Q_SLOTS:
    void stateChanged(KDevelop::IDebugSession::DebuggerState);

private:
    void updateIfFrameOrThreadChanged();

    QFlags<UpdateType> m_autoUpdate;
    int m_activeThread;
    int m_activeFrame;

};
 
} // namespace KDevelop

Q_DECLARE_OPERATORS_FOR_FLAGS(KDevelop::IVariableController::UpdateTypes)

#endif
