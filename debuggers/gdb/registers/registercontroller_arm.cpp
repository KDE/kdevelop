/*
 * Class to fetch/change/send registers to the debugger for arm architecture.
 * Copyright 2013  Vlas Puhov <vlas.puhov@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "registercontroller_arm.h"
#include "../debug.h"

#include <KLocalizedString>

namespace GDBDebugger
{

QVector<QStringList> RegisterController_Arm::m_registerNames;
FlagRegister RegisterController_Arm::m_cpsr;

void  RegisterController_Arm::updateValuesForRegisters(RegistersGroup* registers) const
{
    qCDebug(DEBUGGERGDB) << "Updating values for registers: " << registers->groupName.name();
    if (registers->groupName == enumToGroupName(Flags)) {
        updateFlagValues(registers, m_cpsr);
    } else {
        IRegisterController::updateValuesForRegisters(registers);
    }
}

RegistersGroup RegisterController_Arm::registersFromGroup(const GroupsName& group) const
{
    RegistersGroup registers;

    registers.groupName = group;
    registers.format = m_formatsModes[group.index()].formats.first();
    foreach (const QString & name, registerNamesForGroup(group)) {
        registers.registers.append(Register(name, QString()));
    }

    updateValuesForRegisters(&registers);

    return registers;
}

QVector<GroupsName> RegisterController_Arm::namesOfRegisterGroups() const
{
    static const QVector<GroupsName> registerGroups = QVector<GroupsName>() << enumToGroupName(General) << enumToGroupName(Flags) << enumToGroupName(VFP_single) << enumToGroupName(VFP_double) << enumToGroupName(VFP_quad);

    return registerGroups;
}

void RegisterController_Arm::setRegisterValueForGroup(const GroupsName& group, const Register& reg)
{
    if (group == enumToGroupName(General)) {
        setGeneralRegister(reg, group);
    } else if (group == enumToGroupName(Flags)) {
        setFlagRegister(reg, m_cpsr);
    } else if (group == enumToGroupName(VFP_single)) {
        setVFPS_Register(reg);
    } else if (group == enumToGroupName(VFP_double)) {
        setVFPD_Register(reg);
    } else if (group == enumToGroupName(VFP_quad)) {
        setVFPQ_Register(reg);
    }
}

void RegisterController_Arm::setVFPS_Register(const Register& reg)
{
    setGeneralRegister(reg, enumToGroupName(VFP_single));
}

void RegisterController_Arm::setVFPD_Register(const Register& reg)
{
    setStructuredRegister(reg, enumToGroupName(VFP_double));
}

void RegisterController_Arm::setVFPQ_Register(const Register& reg)
{
    setStructuredRegister(reg, enumToGroupName(VFP_quad));
}

void RegisterController_Arm::updateRegisters(const GroupsName& group)
{
    if (!m_registerNamesInitialized) {
        if (initializeRegisters()) {
            m_registerNamesInitialized = true;
        }
    }

    IRegisterController::updateRegisters(group);
}

GroupsName RegisterController_Arm::enumToGroupName(ArmRegisterGroups group) const
{
    static const GroupsName groups[LAST_REGISTER] = { createGroupName(i18n("General"), General) , createGroupName(i18n("Flags"), Flags, flag, m_cpsr.registerName), createGroupName(i18n("VFP single-word"), VFP_single, floatPoint), createGroupName(i18n("VFP double-word"), VFP_double, structured), createGroupName(i18n("VFP quad-word"), VFP_quad, structured)};

    return groups[group];
}

RegisterController_Arm::RegisterController_Arm(DebugSession* debugSession, QObject* parent) : IRegisterController(debugSession, parent), m_registerNamesInitialized(false)
{
    if (m_registerNames.isEmpty()) {
        for (int i = 0; i < static_cast<int>(LAST_REGISTER); i++) {
            m_registerNames.append(QStringList());
        }
        initRegisterNames();
    }

    int n = 0;
    while (n++ < namesOfRegisterGroups().size()) {
        m_formatsModes.append(FormatsModes());
    }

    m_formatsModes[VFP_double].formats.append(Binary);
    m_formatsModes[VFP_double].formats.append(Decimal);
    m_formatsModes[VFP_double].formats.append(Hexadecimal);
    m_formatsModes[VFP_double].formats.append(Octal);
    m_formatsModes[VFP_double].formats.append(Unsigned);
    m_formatsModes[VFP_double].modes.append(u32);
    m_formatsModes[VFP_double].modes.append(u64);
    m_formatsModes[VFP_double].modes.append(f32);
    m_formatsModes[VFP_double].modes.append(f64);

    m_formatsModes[Flags].formats.append(Raw);
    m_formatsModes[Flags].modes.append(natural);

    m_formatsModes[VFP_single].formats.append(Decimal);
    m_formatsModes[VFP_single].modes.append(natural);

    m_formatsModes[VFP_quad] = m_formatsModes[VFP_double];

    m_formatsModes[General].formats.append(Raw);
    m_formatsModes[General].formats << m_formatsModes[VFP_double].formats;
    m_formatsModes[General].modes.append(natural);
}

void RegisterController_Arm::initRegisterNames()
{
    for (int i = 0; i < 32; i++) {
        m_registerNames[VFP_single] << ("s" + QString::number(i));
    }

    m_cpsr.registerName = "cpsr";
    m_cpsr.flags << "Q" << "V" << "C" << "Z" << "N";
    m_cpsr.bits << "27" << "28" << "29" << "30" << "31";
    m_cpsr.groupName = enumToGroupName(Flags);

    m_registerNames[Flags] = m_cpsr.flags;

    for (int i = 0; i < 13; i++) {
        m_registerNames[General] << ("r" + QString::number(i));
    }
    m_registerNames[General] << "sp" << "lr" << "pc";

    for (int i = 0; i < 32; i++) {
        m_registerNames[VFP_double] << ("d" + QString::number(i));
    }

    for (int i = 0; i < 16; i++) {
        m_registerNames[VFP_quad] << ("q" + QString::number(i));
    }
}

QStringList RegisterController_Arm::registerNamesForGroup(const GroupsName& group) const
{

    for (int i = 0; i < static_cast<int>(LAST_REGISTER); i++) {
        if (group == enumToGroupName(static_cast<ArmRegisterGroups>(i))) {
            return m_registerNames[i];
        }
    }

    return QStringList();
}

}
