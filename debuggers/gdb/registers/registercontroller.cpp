/*
 * Class to fetch/change/send registers to the debugger.
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

#include "registercontroller.h"

#include "../debugsession.h"

#include <qmath.h>

#include <KDebug>

#include "../gdbcommand.h"
#include "../mi/gdbmi.h"

namespace GDBDebugger
{

void IRegisterController::setSession(DebugSession* debugSession)
{
    m_debugSession = debugSession;
}

void IRegisterController::updateRegisters(const GroupsName& group)
{
    if (m_pendingGroups.contains(group)) {
        kDebug() << "Already updating " << group.name();
        return;
    }

    if (group.name().isEmpty()) {
        kDebug() << "Update all";
        m_pendingGroups.clear();
    } else {
        kDebug() << "Update: " << group.name();
        m_pendingGroups << group;
        if (m_pendingGroups.size() != 1) {
            return;
        }
    }

    if (m_debugSession && !m_debugSession->stateIsOn(s_dbgNotStarted | s_shuttingDown)) {
        m_debugSession->addCommand(
            new GDBCommand(GDBMI::DataListRegisterValues, "r", this, &IRegisterController::updateRegisterValuesHandler));
    }
}

void IRegisterController::registerNamesHandler(const GDBMI::ResultRecord& r)
{
    const GDBMI::Value& names = r["register-names"];

    m_rawRegisterNames.clear();
    for (int i = 0; i < names.size(); ++i) {
        const GDBMI::Value& entry = names[i];
        m_rawRegisterNames.push_back(entry.literal());
    }

    //When here probably request for updating XMM registers was sent, but m_rawRegisterNames were not initialized yet, so request was wrong. So update everything once again.
    updateRegisters();
}

void IRegisterController::updateRegisterValuesHandler(const GDBMI::ResultRecord& r)
{
    if (m_rawRegisterNames.isEmpty()) {
        kDebug() << "Registers not initialized yet!";
        return;
    }

    const GDBMI::Value& values = r["register-values"];
    for (int i = 0; i < values.size(); ++i) {
        const GDBMI::Value& entry = values[i];
        int number = entry["number"].literal().toInt();
        Q_ASSERT(m_rawRegisterNames.size() >  number);
        if (!m_rawRegisterNames[number].isEmpty()) {
            const QString value = entry["value"].literal();
            m_registers.insert(m_rawRegisterNames[number], value);
        }
    }

    kDebug() << "groups to change registers: ";
    foreach (const GroupsName & g, m_pendingGroups) {
        kDebug() << g.name();
    }

    foreach (const GroupsName & group, namesOfRegisterGroups()) {
        if (m_pendingGroups.isEmpty() || m_pendingGroups.contains(group)) {
            emit registersChanged(registersFromGroup(group));
        }
    }
    m_pendingGroups.clear();
}

void IRegisterController::setRegisterValue(const Register& reg)
{
    if (!m_registers.isEmpty()) {
        const GroupsName group = groupForRegisterName(reg.name);
        if (!group.name().isEmpty()) {
            setRegisterValueForGroup(group, reg);
        }
    } else {
        kDebug() << "Registers not initialized yet!";
    }
}

QString IRegisterController::registerValue(const QString& name) const
{
    QString value;
    if (!name.isEmpty()) {
        if (m_registers.contains(name)) {
            value = m_registers.value(name);
        }
    }
    return value;
}

bool IRegisterController::initializeRegisters()
{
    if (!m_debugSession || m_debugSession->stateIsOn(s_dbgNotStarted | s_shuttingDown)) {
        return false;
    }

    m_debugSession->addCommand(
        new GDBCommand(GDBMI::DataListRegisterNames, "", this, &IRegisterController::registerNamesHandler));
    return true;
}

GroupsName IRegisterController::groupForRegisterName(const QString& name) const
{
    foreach (const GroupsName & group, namesOfRegisterGroups()) {
        const QStringList registersInGroup = registerNamesForGroup(group);
        foreach (const QString & n, registersInGroup) {
            if (n == name) {
                return group;
            }
        }
    }
    return GroupsName();
}

void IRegisterController::updateValuesForRegisters(RegistersGroup* registers) const
{
    if (m_registers.isEmpty()) {
        kDebug() << "Registers not initialized yet";
        return;
    }

    for (int i = 0; i < registers->registers.size(); i++) {
        if (m_registers.contains(registers->registers[i].name)) {
            registers->registers[i].value = m_registers.value(registers->registers[i].name);
        }
    }
}

void IRegisterController::setFlagRegister(const Register& reg, const FlagRegister& flag)
{
    quint32 flagsValue = registerValue(flag.registerName).toUInt(0, 16);

    const int idx = flag.flags.indexOf(reg.name);

    if (idx != -1) {
        flagsValue ^= static_cast<int>(qPow(2, flag.bits[idx].toUInt()));
        setGeneralRegister(Register(flag.registerName, QString("%1").arg(flagsValue)), flag.groupName);
    } else {
        updateRegisters(flag.groupName);
        kDebug() << reg.name << ' ' << reg.value << "is incorrect flag name/value";
    }
}

void IRegisterController::setGeneralRegister(const Register& reg, const GroupsName& group)
{
    const QString command = QString("set var $%1=%2").arg(reg.name).arg(reg.value);
    kDebug() << "Setting register: " << command;

    if (m_debugSession && !m_debugSession->stateIsOn(s_dbgNotStarted | s_shuttingDown)) {
        m_debugSession->addCommand(new GDBCommand(GDBMI::NonMI, command));
        updateRegisters(group);
    } else {
        kDebug() << "Session has ended";
    }
}

void IRegisterController::convertValuesForGroup(RegistersGroup* registersGroup) const
{
    if (registersGroup->format > Hexadecimal) {
        return;
    }

    bool ok;
    for (int i = 0; i < registersGroup->registers.size(); i++) {
        const QString converted = QString::number(registersGroup->registers[i].value.toULongLong(&ok, 16), static_cast<int> (registersGroup->format));
        if (ok) {
            registersGroup->registers[i].value = converted;
        } else {
            kDebug() << "Can't convert register value to format: " << registersGroup->format << ' ' << registersGroup->registers[i].name << ' ' << registersGroup->registers[i].value;
        }
    }
}

IRegisterController::IRegisterController(DebugSession* debugSession, QObject* parent)
: QObject(parent), m_debugSession(debugSession) {}

IRegisterController::~IRegisterController() {}

void IRegisterController::updateFlagValues(RegistersGroup* flagsGroup, const FlagRegister& flagRegister) const
{
    const quint32 flagsValue = registerValue(flagRegister.registerName).toUInt(0, 16);

    for (int idx = 0; idx < flagRegister.flags.count(); idx++) {
        flagsGroup->registers[idx].value = ((flagsValue >> flagRegister.bits[idx].toInt()) & 1) ? "1" : "0";
    }
}

QVector<Format> IRegisterController::formats(const GroupsName& group)
{
    int idx = -1;
    foreach (const GroupsName & g, namesOfRegisterGroups()) {
        if (g == group) {
            idx = g.index();
        }
    }
    Q_ASSERT(idx != -1);
    return m_formats[idx];
}

GroupsName IRegisterController::createGroupName(const QString& name, int idx) const
{
    return GroupsName(name, idx);
}

void IRegisterController::setFormat(Format f, const GroupsName& group)
{
    foreach (const GroupsName & g, namesOfRegisterGroups()) {
        if (g == group) {
            int i = m_formats[g.index()].indexOf(f);
            if (i != -1) {
                m_formats[g.index()].remove(i);
                m_formats[g.index()].prepend(f);
            }
        }
    }
}

QString IRegisterController::numberForName(const QString& name) const
{
    //Requests for number come in order(if the previous was, let's say 10, then most likely the next one'll be 11)
    static int previousNumber = -1;
    if (m_rawRegisterNames.isEmpty()) {
        previousNumber = -1;
        return QString::number(previousNumber);
    }

    if (previousNumber != -1 && m_rawRegisterNames.size() > ++previousNumber) {
        if (m_rawRegisterNames[previousNumber] == name) {
            return QString::number(previousNumber);
        }
    }

    for (int number = 0; number < m_rawRegisterNames.size(); number++) {
        if (name == m_rawRegisterNames[number]) {
            previousNumber = number;
            return QString::number(number);
        }
    }

    previousNumber = -1;
    return QString::number(previousNumber);
}

}
