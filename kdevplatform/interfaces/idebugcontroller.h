/*
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IDEBUGCONTROLLER_H
#define KDEVPLATFORM_IDEBUGCONTROLLER_H

#include <QObject>
#include "interfacesexport.h"

namespace KDevelop {

class VariableCollection;
class BreakpointModel;
class IDebugSession;

/** Top level debugger object. Exists as long as KDevelop exists
    and holds some global debugger state, like breakpoints.
    Also holds the IDebugSession for the specific application
    that is being debugged.
*/
class KDEVPLATFORMINTERFACES_EXPORT IDebugController : public QObject
{
    Q_OBJECT
public:
    explicit IDebugController(QObject *parent = nullptr);
    ~IDebugController() override;

    /** Each plugin using the debugger actions needs to call this function to setup the gui */
    virtual void initializeUi() = 0;

    /**
     * @return whether a new debug session can be added at this time
     *
     * @param replaceSessionQuestionText the text of the question to ask
     *        the user if an existing debug session is currently running
     */
    [[nodiscard]] virtual bool canAddSession(const QString& replaceSessionQuestionText) const = 0;

    virtual void addSession(IDebugSession* session) = 0;
    
    /** Return the current debug session. At present, only
        one session may be active at a time.  */
    virtual IDebugSession *currentSession() = 0;

    virtual BreakpointModel *breakpointModel() = 0;
    virtual VariableCollection *variableCollection() = 0;

Q_SIGNALS:
    void currentSessionChanged(KDevelop::IDebugSession* session);
    void raiseFramestackViews();
};

}

#endif
