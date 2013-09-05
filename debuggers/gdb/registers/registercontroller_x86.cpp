/*
 * Class to fetch/change/send registers to the debugger for x86, x86_64 architectures.
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

#include "registercontroller_x86.h"

#include "../debugsession.h"
#include "converters.h"

#include <KDebug>
#include <KLocalizedString>

namespace GDBDebugger
{

QVector<QStringList> RegisterControllerGeneral_x86::m_registerNames;
FlagRegister RegisterControllerGeneral_x86::m_eflags;

void RegisterControllerGeneral_x86::updateValuesForRegisters(RegistersGroup* registers) const
{
    kDebug() << "Updating values for registers: " << registers->groupName.name();
    if (registers->groupName == enumToGroupName(Flags)) {
        updateFlagValues(registers, m_eflags);
    } else {
        IRegisterController::updateValuesForRegisters(registers);
    }
}

RegistersGroup RegisterControllerGeneral_x86::registersFromGroup(const GroupsName& group) const
{
    RegistersGroup registers;

    registers.groupName = group;
    registers.flag = (group == enumToGroupName(Flags)) ? true : false;
    registers.format = m_formats[group.index()].first();
    foreach (const QString & name, registerNamesForGroup(group)) {
        registers.registers.append(Register(name, QString()));
    }

    updateValuesForRegisters(&registers);
    convertValuesForGroup(&registers);

    return registers;
}

QVector<GroupsName> RegisterControllerGeneral_x86::namesOfRegisterGroups() const
{
    static const QVector<GroupsName> registerGroups =  QVector<GroupsName>() << enumToGroupName(General) << enumToGroupName(Flags) << enumToGroupName(FPU) << enumToGroupName(XMM) << enumToGroupName(Segment);

    return registerGroups;
}

void RegisterControllerGeneral_x86::setRegisterValueForGroup(const GroupsName& group, const Register& reg)
{
    if (group == enumToGroupName(General)) {
        setGeneralRegister(reg, group);
    } else if (group == enumToGroupName(Flags)) {
        setFlagRegister(reg, m_eflags);
    } else if (group == enumToGroupName(FPU)) {
        setFPURegister(reg);
    } else if (group == enumToGroupName(XMM)) {
        setXMMRegister(reg);
    } else if (group == enumToGroupName(Segment)) {
        setSegmentRegister(reg);
    }
}

void RegisterControllerGeneral_x86::setFPURegister(const Register& reg)
{
    setGeneralRegister(reg, enumToGroupName(FPU));
}

void RegisterControllerGeneral_x86::setXMMRegister(const Register& reg)
{
    setStructuredRegister(reg, enumToGroupName(XMM));
}

void RegisterControllerGeneral_x86::setSegmentRegister(const Register& reg)
{
    setGeneralRegister(reg, enumToGroupName(Segment));
}

void RegisterControllerGeneral_x86::updateRegisters(const GroupsName& group)
{
    if (!m_registerNamesInitialized) {
        if (initializeRegisters()) {
            m_registerNamesInitialized = true;
        }
    }

    IRegisterController::updateRegisters(group);
}

GroupsName RegisterControllerGeneral_x86::enumToGroupName(X86RegisterGroups group) const
{
    static const GroupsName groups[LAST_REGISTER] = { createGroupName(i18n("General"), General), createGroupName(i18n("Flags"), Flags, flag, m_eflags.registerName), createGroupName(i18n("FPU"), FPU, floatPoint), createGroupName(i18n("XMM"), XMM, structured), createGroupName(i18n("Segment"), Segment)};

    return groups[group];
}

void RegisterControllerGeneral_x86::convertValuesForGroup(RegistersGroup* registersGroup) const
{
    if (registersGroup->format != Raw) {
        IRegisterController::convertValuesForGroup(registersGroup);
    }
}

RegisterController_x86::RegisterController_x86(DebugSession* debugSession, QObject* parent)
    : RegisterControllerGeneral_x86(debugSession, parent)
{
    initRegisterNames();
}

RegisterController_x86_64::RegisterController_x86_64(DebugSession* debugSession, QObject* parent)
    : RegisterControllerGeneral_x86(debugSession, parent)
{
    initRegisterNames();
}

RegisterControllerGeneral_x86::RegisterControllerGeneral_x86(DebugSession* debugSession, QObject* parent)
    : IRegisterController(debugSession, parent), m_registerNamesInitialized(false)
{
    if (m_registerNames.isEmpty()) {
        for (int i = 0; i < static_cast<int>(LAST_REGISTER); i++) {
            m_registerNames.append(QStringList());
        }
        initRegisterNames();
    }

    int n = 0;
    while (n++ < namesOfRegisterGroups().size()) {
        m_formats.append(QVector<Format>());
    }

    m_formats[General].append(Raw);
    m_formats[General].append(Binary);
    m_formats[General].append(Decimal);
    m_formats[General].append(Hexadecimal);
    m_formats[General].append(Octal);

    m_formats[Flags].append(Raw);

    m_formats[FPU].append(Raw);

    m_formats[Segment] = m_formats[General];

    m_formats[XMM].append(v4_float);
    m_formats[XMM].append(v2_double);
    m_formats[XMM].append(v4_int32);
    m_formats[XMM].append(v2_int64);
}

void RegisterControllerGeneral_x86::initRegisterNames()
{
    for (int i = 0; i < 8; i++) {
        m_registerNames[FPU] << ("st" + QString::number(i));
    }

    m_registerNames[Flags] << "C" << "P" << "A" << "Z" << "S" << "T" << "D" << "O";

    m_registerNames[Segment] << "cs" << "ss" << "ds" << "es" << "fs" << "gs";

    m_eflags.flags = m_registerNames[Flags];
    m_eflags.bits << "0" << "2" << "4" << "6" << "7" << "8" << "10" << "11";
    m_eflags.registerName = "eflags";
    m_eflags.groupName = enumToGroupName(Flags);
}

void RegisterController_x86::initRegisterNames()
{
    m_registerNames[General] = QStringList() << "eax" << "ebx" << "ecx" << "edx" << "esi" << "edi" << "ebp" << "esp" << "eip";

    m_registerNames[XMM].clear();
    for (int i = 0; i < 8; i++) {
        m_registerNames[XMM] << ("xmm" + QString::number(i));
    }
}

void RegisterController_x86_64::initRegisterNames()
{
    m_registerNames[General] = QStringList() << "rax" << "rbx" << "rcx" << "rdx" << "rsi" << "rdi" << "rbp" << "rsp" << "r8" << "r9" << "r10" << "r11" << "r12" << "r13" << "r14" << "r15" << "rip";

    m_registerNames[XMM].clear();
    for (int i = 0; i < 16; i++) {
        m_registerNames[XMM] << ("xmm" + QString::number(i));
    }
}

QStringList RegisterControllerGeneral_x86::registerNamesForGroup(const GroupsName& group) const
{
    for (int i = 0; i < static_cast<int>(LAST_REGISTER); i++) {
        if (group == enumToGroupName(static_cast<X86RegisterGroups>(i))) {
            return m_registerNames[i];
        }
    }

    return QStringList();
}

}
