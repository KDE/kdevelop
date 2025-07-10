/*
    SPDX-FileCopyrightText: 2013 Vlas Puhov <vlas.puhov@mail.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "registercontroller_x86.h"
#include "debuglog.h"

#include <KLocalizedString>

using namespace KDevMI;

QVector<QStringList> RegisterControllerGeneral_x86::m_registerNames;
FlagRegister RegisterControllerGeneral_x86::m_eflags;

void RegisterControllerGeneral_x86::updateValuesForRegisters(RegistersGroup* registers) const
{
    qCDebug(DEBUGGERCOMMON) << "Updating values for registers: " << registers->groupName.name();
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
    registers.format = m_formatsModes[group.index()].formats.first();
    const auto registerNames = registerNamesForGroup(group);
    registers.registers.reserve(registerNames.size());
    for (const auto& name : registerNames) {
        registers.registers.append(Register(name, QString()));
    }

    updateValuesForRegisters(&registers);

    return registers;
}

QVector<GroupsName> RegisterControllerGeneral_x86::namesOfRegisterGroups() const
{
    static const QVector<GroupsName> registerGroups =  QVector<GroupsName>{
        enumToGroupName(General),
        enumToGroupName(Flags),
        enumToGroupName(FPU),
        enumToGroupName(XMM),
        enumToGroupName(Segment),
    };

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

GroupsName RegisterControllerGeneral_x86::enumToGroupName(X86RegisterGroups group) const
{
    static const GroupsName groups[LAST_REGISTER] = { createGroupName(i18n("General"), General), createGroupName(i18n("Flags"), Flags, flag, m_eflags.registerName), createGroupName(i18n("FPU"), FPU, floatPoint), createGroupName(i18n("XMM"), XMM, structured), createGroupName(i18n("Segment"), Segment)};

    return groups[group];
}

RegisterController_x86::RegisterController_x86(MIDebugSession* debugSession, const QStringList& debuggerRegisterNames,
                                               QObject* parent)
    : RegisterControllerGeneral_x86(debugSession, debuggerRegisterNames, parent)
{
    initRegisterNames();
}

RegisterController_x86_64::RegisterController_x86_64(MIDebugSession* debugSession,
                                                     const QStringList& debuggerRegisterNames, QObject* parent)
    : RegisterControllerGeneral_x86(debugSession, debuggerRegisterNames, parent)
{
    initRegisterNames();
}

RegisterControllerGeneral_x86::RegisterControllerGeneral_x86(MIDebugSession* debugSession,
                                                             const QStringList& debuggerRegisterNames, QObject* parent)
    : IRegisterController(debugSession, debuggerRegisterNames, parent)
{
    if (m_registerNames.isEmpty()) {
        const int registerCount = static_cast<int>(LAST_REGISTER);
        m_registerNames.resize(registerCount);
        initRegisterNames();
    }

    m_formatsModes.resize(namesOfRegisterGroups().size());

    m_formatsModes[XMM].formats = {Binary, Decimal, Hexadecimal, Octal, Unsigned};
    m_formatsModes[XMM].modes = {v4_float, v2_double, v4_int32, v2_int64};

    m_formatsModes[Flags].formats.append(Raw);
    m_formatsModes[Flags].modes.append(natural);

    m_formatsModes[FPU].formats.append(Decimal);
    m_formatsModes[FPU].modes.append(natural);

    m_formatsModes[General].modes.append(natural);
    m_formatsModes[General].formats.append(Raw);
    m_formatsModes[General].formats << m_formatsModes[XMM].formats;

    m_formatsModes[Segment] = m_formatsModes[General];
}

void RegisterControllerGeneral_x86::initRegisterNames()
{
    for (int i = 0; i < 8; i++) {
        m_registerNames[FPU] << (QLatin1String("st") + QString::number(i));
    }

    m_registerNames[Flags] = QStringList{
        QStringLiteral("C"),
        QStringLiteral("P"),
        QStringLiteral("A"),
        QStringLiteral("Z"),
        QStringLiteral("S"),
        QStringLiteral("T"),
        QStringLiteral("D"),
        QStringLiteral("O")
    };

    m_registerNames[Segment] = QStringList{
        QStringLiteral("cs"),
        QStringLiteral("ss"),
        QStringLiteral("ds"),
        QStringLiteral("es"),
        QStringLiteral("fs"),
        QStringLiteral("gs"),
    };

    m_eflags.flags = m_registerNames[Flags];
    m_eflags.bits = QStringList{
        QStringLiteral("0"),
        QStringLiteral("2"),
        QStringLiteral("4"),
        QStringLiteral("6"),
        QStringLiteral("7"),
        QStringLiteral("8"),
        QStringLiteral("10"),
        QStringLiteral("11"),
    };
    m_eflags.registerName = QStringLiteral("eflags");
    m_eflags.groupName = enumToGroupName(Flags);
}

void RegisterController_x86::initRegisterNames()
{
    m_registerNames[General] = QStringList{
        QStringLiteral("eax"),
        QStringLiteral("ebx"),
        QStringLiteral("ecx"),
        QStringLiteral("edx"),
        QStringLiteral("esi"),
        QStringLiteral("edi"),
        QStringLiteral("ebp"),
        QStringLiteral("esp"),
        QStringLiteral("eip")
    };

    m_registerNames[XMM].clear();
    for (int i = 0; i < 8; i++) {
        m_registerNames[XMM] << (QLatin1String("xmm") + QString::number(i));
    }
}

void RegisterController_x86_64::initRegisterNames()
{
    m_registerNames[General] = QStringList{
        QStringLiteral("rax"),
        QStringLiteral("rbx"),
        QStringLiteral("rcx"),
        QStringLiteral("rdx"),
        QStringLiteral("rsi"),
        QStringLiteral("rdi"),
        QStringLiteral("rbp"),
        QStringLiteral("rsp"),
        QStringLiteral("r8"),
        QStringLiteral("r9"),
        QStringLiteral("r10"),
        QStringLiteral("r11"),
        QStringLiteral("r12"),
        QStringLiteral("r13"),
        QStringLiteral("r14"),
        QStringLiteral("r15"),
        QStringLiteral("rip")
    };

    m_registerNames[XMM].clear();
    for (int i = 0; i < 16; i++) {
        m_registerNames[XMM] << (QLatin1String("xmm") + QString::number(i));
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

#include "moc_registercontroller_x86.cpp"
