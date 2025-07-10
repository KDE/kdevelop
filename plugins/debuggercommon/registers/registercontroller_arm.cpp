/*
    SPDX-FileCopyrightText: 2013 Vlas Puhov <vlas.puhov@mail.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "registercontroller_arm.h"
#include "debuglog.h"

#include <KLocalizedString>

using namespace KDevMI;

QVector<QStringList> RegisterController_Arm::m_registerNames;
FlagRegister RegisterController_Arm::m_cpsr;

void  RegisterController_Arm::updateValuesForRegisters(RegistersGroup* registers) const
{
    qCDebug(DEBUGGERCOMMON) << "Updating values for registers: " << registers->groupName.name();
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
    const auto registerNames = registerNamesForGroup(group);
    registers.registers.reserve(registerNames.size());
    for (const auto& name : registerNames) {
        registers.registers.append(Register(name, QString()));
    }

    updateValuesForRegisters(&registers);

    return registers;
}

QVector<GroupsName> RegisterController_Arm::namesOfRegisterGroups() const
{
    static const QVector<GroupsName> registerGroups = QVector<GroupsName>{
        enumToGroupName(General),
        enumToGroupName(Flags),
        enumToGroupName(VFP_single),
        enumToGroupName(VFP_double),
        enumToGroupName(VFP_quad),
    };

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

GroupsName RegisterController_Arm::enumToGroupName(ArmRegisterGroups group) const
{
    static const GroupsName groups[LAST_REGISTER] = { createGroupName(i18n("General"), General) , createGroupName(i18n("Flags"), Flags, flag, m_cpsr.registerName), createGroupName(i18n("VFP single-word"), VFP_single, floatPoint), createGroupName(i18n("VFP double-word"), VFP_double, structured), createGroupName(i18n("VFP quad-word"), VFP_quad, structured)};

    return groups[group];
}

RegisterController_Arm::RegisterController_Arm(MIDebugSession* debugSession, const QStringList& debuggerRegisterNames,
                                               QObject* parent)
    : IRegisterController(debugSession, debuggerRegisterNames, parent)
{
    if (m_registerNames.isEmpty()) {
        const int registerCount = static_cast<int>(LAST_REGISTER);
        m_registerNames.resize(registerCount);
        initRegisterNames();
    }

    m_formatsModes.resize(namesOfRegisterGroups().size());

    m_formatsModes[VFP_double].formats = {Binary, Decimal, Hexadecimal, Octal, Unsigned};
    m_formatsModes[VFP_double].modes = {u32, u64, f32, f64};

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
        m_registerNames[VFP_single] << (QLatin1Char('s') + QString::number(i));
    }

    m_cpsr.registerName = QStringLiteral("cpsr");
    m_cpsr.flags = QStringList{
        QStringLiteral("Q"),
        QStringLiteral("V"),
        QStringLiteral("C"),
        QStringLiteral("Z"),
        QStringLiteral("N"),
    };
    m_cpsr.bits = QStringList{
        QStringLiteral("27"),
        QStringLiteral("28"),
        QStringLiteral("29"),
        QStringLiteral("30"),
        QStringLiteral("31"),
    };
    m_cpsr.groupName = enumToGroupName(Flags);

    m_registerNames[Flags] = m_cpsr.flags;

    for (int i = 0; i < 13; i++) {
        m_registerNames[General] << (QLatin1Char('r') + QString::number(i));
    }
    m_registerNames[General] << QStringLiteral("sp") << QStringLiteral("lr") << QStringLiteral("pc");

    for (int i = 0; i < 32; i++) {
        m_registerNames[VFP_double] << (QLatin1Char('d') + QString::number(i));
    }

    for (int i = 0; i < 16; i++) {
        m_registerNames[VFP_quad] << (QLatin1Char('q') + QString::number(i));
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

#include "moc_registercontroller_arm.cpp"
