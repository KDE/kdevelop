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

#include "../debugsession.h"
#include "../gdbcommand.h"
#include "../mi/gdbmi.h"

#include <qmath.h>

#include <KDebug>
#include <KLocalizedString>

namespace GDBDebugger
{

QVector<QStringList> RegisterController_Arm::m_registerNames;
FlagRegister RegisterController_Arm::m_cpsr;

void  RegisterController_Arm::updateValuesForRegisters(RegistersGroup* registers) const
{
    kDebug() << "Updating values for registers: " << registers->groupName.name();
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
    registers.editable = (group == enumToGroupName(VFP_double) || group == enumToGroupName(VFP_quad)) ? false : true;
    registers.flag = (group == enumToGroupName(Flags)) ? true : false;
    registers.format = m_formats[group.index()].first();
    foreach (const QString & name, registerNamesForGroup(group)) {
        registers.registers.append(Register(name, QString()));
    }

    updateValuesForRegisters(&registers);
    convertValuesForGroup(&registers);

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

void RegisterController_Arm::setVFPD_Register(const Register& /*reg*/)
{
    //FIXME: gdb doesn't support setting values for registers with more than 64 bits directly(all that structures stuff should be used.).
    //setGeneralRegister ( reg, enumToString ( VFP_double ) );
}

void RegisterController_Arm::setVFPQ_Register(const Register& /*reg */)
{
    //FIXME:
    // setGeneralRegister ( reg, enumToString ( VFP_quad ) );
}

void RegisterController_Arm::updateRegisters(const GroupsName& group)
{
    if (!m_debugSession || m_debugSession->stateIsOn(s_dbgNotStarted | s_shuttingDown)) {
        return;
    }

    if (!m_registerNamesInitialized) {
        if (initializeRegisters()) {
            m_registerNamesInitialized = true;
        }
    }

    if (group.name() != enumToGroupName(VFP_single).name()) {
        if (group.name().isEmpty()) {
            QVector<GroupsName> groups = namesOfRegisterGroups();
            groups.remove(groups.indexOf(enumToGroupName(VFP_single)));
            foreach (const GroupsName& g, groups) {
                IRegisterController::updateRegisters(g);
            }
        } else {
            IRegisterController::updateRegisters(group);
        }
    }

    if (group == enumToGroupName(VFP_single) || group.name().isEmpty()) {
        QString command = "info all-registers ";
        foreach (const QString & name, registerNamesForGroup(enumToGroupName(VFP_single))) {
            command += "$" + name + ' ';
        }

        if (m_debugSession && !m_debugSession->stateIsOn(s_dbgNotStarted | s_shuttingDown)) {
            //TODO: use mi interface instead.
            m_debugSession->addCommand(
                new CliCommand(GDBMI::NonMI, command, this, &RegisterController_Arm::handleVFPSRegisters));
        }

        if (group == enumToGroupName(VFP_single)) {
            IRegisterController::updateRegisters(enumToGroupName(VFP_double));
            IRegisterController::updateRegisters(enumToGroupName(VFP_quad));
        }
    }
}

void RegisterController_Arm::handleVFPSRegisters(const QStringList& record)
{
    //s0       0
    //s1       -1
    //s2       123456789
    //s3       -12345.6789
    //s10       12.34
    //s15       1.1e+2
    //s30       -23.45e-10
    const QRegExp rx("^(s\\d+)\\s+((?:-?\\d+\\.?\\d+(?:e(\\+|-)\\d+)?)|(?:-?\\d+))$");
    QVector<Register> registers;
    foreach (const QString & s, record) {
        if (rx.exactMatch(s)) {
            registers.push_back(Register(rx.cap(1), rx.cap(2)));
        }
    }

    if (registers.size() != 32) {
        kDebug() << "can't parse VFP single. Wrong format";
        kDebug() << record;
        kDebug() << "Parsed registers: ";
        foreach (const Register & r, registers) {
            kDebug() << r.name << ' ' << r.value;
        }
    } else {
        foreach (const Register & r, registers) {
            if (m_registers.contains(r.name)) {
                m_registers[r.name] = r.value;
            }
        }
        emit registersChanged(registersFromGroup(enumToGroupName(VFP_single)));
    }
}

GroupsName RegisterController_Arm::enumToGroupName(ArmRegisterGroups group) const
{
    static const GroupsName groups[LAST_REGISTER] = { createGroupName(i18n("General"), General) , createGroupName(i18n("Flags"), Flags), createGroupName(i18n("VFP single-word"), VFP_single), createGroupName(i18n("VFP double-word"), VFP_double), createGroupName(i18n("VFP quad-word"), VFP_quad)};

    return groups[group];
}

void RegisterController_Arm::convertValuesForGroup(RegistersGroup* registersGroup) const
{
    if (registersGroup->format != Raw && registersGroup->format != Natural) {
        IRegisterController::convertValuesForGroup(registersGroup);
    }
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
    while (n < namesOfRegisterGroups().size()) {
        m_formats.append(QVector<Format>());
    }

    m_formats[General].append(Raw);
    m_formats[General].append(Binary);
    m_formats[General].append(Decimal);
    m_formats[General].append(Hexadecimal);
    m_formats[General].append(Octal);
    m_formats[General].append(Natural);

    m_formats[Flags].append(Raw);

    m_formats[VFP_single].append(Raw);
    m_formats[VFP_double].append(Raw);
    m_formats[VFP_quad].append(Raw);
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
