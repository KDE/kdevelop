/*
    SPDX-FileCopyrightText: 2013 Vlas Puhov <vlas.puhov@mail.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REGISTERCONTROLLER_ARM_H
#define REGISTERCONTROLLER_ARM_H


#include "registercontroller.h"

namespace KDevMI {
class MIDebugSession;

class RegisterController_Arm : public IRegisterController
{
    Q_OBJECT

public:
    /**
     * Create an ARM register controller.
     *
     * @copydetails IRegisterController::IRegisterController()
     */
    explicit RegisterController_Arm(MIDebugSession* debugSession, QObject* parent = nullptr);

    QVector<GroupsName> namesOfRegisterGroups() const override;

public Q_SLOTS:
    void updateRegisters(const GroupsName& group = GroupsName()) override;

protected:
    RegistersGroup registersFromGroup(const GroupsName& group) const override;

    QStringList registerNamesForGroup(const GroupsName& group) const override;

    void updateValuesForRegisters(RegistersGroup* registers) const override;

    void setRegisterValueForGroup(const GroupsName& group, const Register& reg) override;

    enum ArmRegisterGroups {General, Flags, VFP_single, VFP_double, VFP_quad, LAST_REGISTER};

    GroupsName enumToGroupName(ArmRegisterGroups group) const;
    //None of functions below checks value for validity, if value is invalid updateRegistres() will restore the previous state.

    void setVFPS_Register(const Register& reg);
    void setVFPD_Register(const Register& reg);
    void setVFPQ_Register(const Register& reg);

    static QVector<QStringList> m_registerNames;

private:
    void initRegisterNames();

    static FlagRegister m_cpsr;

    bool m_registerNamesInitialized = false;
};

} // end of namespace KDevMI

#endif // REGISTERCONTROLLER_ARM_H
