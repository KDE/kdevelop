/*
    SPDX-FileCopyrightText: 2013 Vlas Puhov <vlas.puhov@mail.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REGISTERCONTROLLER_X86_H
#define REGISTERCONTROLLER_X86_H

#include "registercontroller.h"

namespace KDevMI
{
class MIDebugSession;

class RegisterControllerGeneral_x86 : public IRegisterController
{
    Q_OBJECT

public:
    QVector<GroupsName> namesOfRegisterGroups() const override;

protected:
    explicit RegisterControllerGeneral_x86(MIDebugSession* debugSession, const QStringList& debuggerRegisterNames,
                                           QObject* parent = nullptr);

    RegistersGroup registersFromGroup(const GroupsName& group) const override;

    QStringList registerNamesForGroup(const GroupsName& group) const override;

    void updateValuesForRegisters(RegistersGroup* registers) const override;

    void setRegisterValueForGroup(const GroupsName& group, const Register& reg) override;

    enum X86RegisterGroups {General, Flags, FPU, XMM, Segment, LAST_REGISTER};

    GroupsName enumToGroupName(X86RegisterGroups group) const;
    //None of functions below checks value for validity, if value is invalid updateRegistres() will restore the previous state.

    void setFPURegister(const Register& reg);
    void setXMMRegister(const Register& reg);
    void setSegmentRegister(const Register& reg);
    void setEFfagRegister(const Register& reg);
private:
    void initRegisterNames();

protected:
    static QVector<QStringList> m_registerNames;
    static FlagRegister m_eflags;
};

class RegisterController_x86 : public RegisterControllerGeneral_x86
{
    Q_OBJECT

public:
    /**
     * Create an x86 register controller.
     *
     * @copydetails IRegisterController::IRegisterController()
     */
    explicit RegisterController_x86(MIDebugSession* debugSession, const QStringList& debuggerRegisterNames,
                                    QObject* parent = nullptr);

private:
    void initRegisterNames();
};

class RegisterController_x86_64 : public RegisterControllerGeneral_x86
{
    Q_OBJECT

public:
    /**
     * Create an x86-64 register controller.
     *
     * @copydetails IRegisterController::IRegisterController()
     */
    explicit RegisterController_x86_64(MIDebugSession* debugSession, const QStringList& debuggerRegisterNames,
                                       QObject* parent = nullptr);

private:
    void initRegisterNames();
};

} // end of namespace KDevMI

#endif // REGISTERCONTROLLER_X86_H
