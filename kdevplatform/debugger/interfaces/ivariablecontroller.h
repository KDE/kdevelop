/*
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IVARIABLECONTROLLER_H
#define KDEVPLATFORM_IVARIABLECONTROLLER_H

#include <QObject>

#include "idebugsession.h"

namespace KTextEditor {
class Document;
class Cursor;
class Range;
}

namespace KDevelop {

class VariableCollection;
class Variable;
class TreeModel;
class TreeItem;
class IVariableControllerPrivate;

class KDEVPLATFORMDEBUGGER_EXPORT IVariableController : public QObject
{
    Q_OBJECT
public:
    explicit IVariableController(IDebugSession* parent);
    ~IVariableController() override;

    /* Create a variable for the specified expression in the currentl
       thread and frame.  */     
    virtual Variable* createVariable(TreeModel* model, TreeItem* parent, 
                                     const QString& expression,
                                     const QString& display = {}) = 0;

    virtual KTextEditor::Range expressionRangeUnderCursor(KTextEditor::Document* doc, const KTextEditor::Cursor& cursor) = 0;

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

private:
    const QScopedPointer<class IVariableControllerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(IVariableController)
};

#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
Q_DECLARE_OPERATORS_FOR_FLAGS(IVariableController::UpdateTypes)
#endif

} // namespace KDevelop

#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
Q_DECLARE_OPERATORS_FOR_FLAGS(KDevelop::IVariableController::UpdateTypes)
#endif

#endif
