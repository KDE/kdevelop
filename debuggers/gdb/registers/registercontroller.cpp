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

#include <qmath.h>
#include <QRegExp>


#include "../debugsession.h"
#include "../gdbcommand.h"
#include "../mi/gdbmi.h"
#include "../debug.h"
#include "converters.h"

namespace GDBDebugger
{

void IRegisterController::setSession(DebugSession* debugSession)
{
    m_debugSession = debugSession;
}

void IRegisterController::updateRegisters(const GroupsName& group)
{
    if (!m_debugSession || m_debugSession->stateIsOn(s_dbgNotStarted | s_shuttingDown)) {
        return;
    }

    if (m_pendingGroups.contains(group)) {
        qCDebug(DEBUGGERGDB) << "Already updating " << group.name();
        return;
    }

    if (group.name().isEmpty()) {
        foreach (const GroupsName & g, namesOfRegisterGroups()) {
            IRegisterController::updateRegisters(g);
        }
        return;
    } else {
        qCDebug(DEBUGGERGDB) << "Updating: " << group.name();
        m_pendingGroups << group;
    }

    QString registers;
    Format currentFormat = formats(group).first();
    switch (currentFormat) {
    case Binary:
        registers = "t ";
        break;
    case Octal:
        registers = "o ";
        break;
    case Decimal :
        registers = "d ";
        break;
    case Hexadecimal:
        registers = "x ";
        break;
    case Raw:
        registers = "r ";
        break;
    case Unsigned:
        registers = "u ";
        break;
    default:
        break;
    }

    //float point registers have only two reasonable format.
    Mode currentMode = modes(group).first();
    if (((currentMode >= v4_float && currentMode <= v2_double) ||
        (currentMode >= f32 && currentMode <= f64) || group.type() == floatPoint) && currentFormat != Raw) {
        registers = "N ";
    }

    if (group.type() == flag) {
        registers += numberForName(group.flagName());
    } else {
        foreach (const QString & name, registerNamesForGroup(group)) {
            registers += numberForName(name) + ' ';
        }
    }

    //Not initialized yet. They'll be updated afterwards.
    if (registers.contains("-1")) {
        qCDebug(DEBUGGERGDB) << "Will update later";
        m_pendingGroups.clear();
        return;
    }

    void (IRegisterController::* handler)(const GDBMI::ResultRecord&);
    if (group.type() == structured && currentFormat != Raw) {
        handler = &IRegisterController::structuredRegistersHandler;
    } else {
        handler = &IRegisterController::generalRegistersHandler;
    }

    m_debugSession->addCommand(new GDBCommand(GDBMI::DataListRegisterValues, registers, this, handler));
}

void IRegisterController::registerNamesHandler(const GDBMI::ResultRecord& r)
{
    const GDBMI::Value& names = r["register-names"];

    m_rawRegisterNames.clear();
    for (int i = 0; i < names.size(); ++i) {
        const GDBMI::Value& entry = names[i];
        m_rawRegisterNames.push_back(entry.literal());
    }

    //When here probably request for updating registers was sent, but m_rawRegisterNames were not initialized yet, so it wasn't successful. Update everything once again.
    updateRegisters();
}

void IRegisterController::generalRegistersHandler(const GDBMI::ResultRecord& r)
{
    Q_ASSERT(!m_rawRegisterNames.isEmpty());

    QString registerName;

    const GDBMI::Value& values = r["register-values"];
    for (int i = 0; i < values.size(); ++i) {
        const GDBMI::Value& entry = values[i];
        int number = entry["number"].literal().toInt();
        Q_ASSERT(m_rawRegisterNames.size() >  number);

        if (!m_rawRegisterNames[number].isEmpty()) {
            if (registerName.isEmpty()) {
                registerName = m_rawRegisterNames[number];
            }
            const QString value = entry["value"].literal();
            m_registers.insert(m_rawRegisterNames[number], value);
        }
    }

    GroupsName group = groupForRegisterName(registerName);

    if (m_pendingGroups.contains(group)) {
        emit registersChanged(registersFromGroup(group));
        m_pendingGroups.remove(m_pendingGroups.indexOf(group));
    }
}

void IRegisterController::setRegisterValue(const Register& reg)
{
    Q_ASSERT(!m_registers.isEmpty());

    const GroupsName group = groupForRegisterName(reg.name);
    if (!group.name().isEmpty()) {
        setRegisterValueForGroup(group, reg);
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
        if (group.flagName() == name) {
            return group;
        }
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
    Q_ASSERT(!m_registers.isEmpty());

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
        setGeneralRegister(Register(flag.registerName, QString("0x%1").arg(flagsValue, 0, 16)), flag.groupName);
    } else {
        updateRegisters(flag.groupName);
        qCDebug(DEBUGGERGDB) << reg.name << ' ' << reg.value << "is incorrect flag name/value";
    }
}

void IRegisterController::setGeneralRegister(const Register& reg, const GroupsName& group)
{
    if (!m_debugSession || m_debugSession->stateIsOn(s_dbgNotStarted | s_shuttingDown)) {
        return;
    }

    const QString command = QString("set var $%1=%2").arg(reg.name).arg(reg.value);
    qCDebug(DEBUGGERGDB) << "Setting register: " << command;

    m_debugSession->addCommand(new GDBCommand(GDBMI::NonMI, command));
    updateRegisters(group);
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
    return m_formatsModes[idx].formats;
}

GroupsName IRegisterController::createGroupName(const QString& name, int idx, RegisterType t, const QString flag) const
{
    return GroupsName(name, idx, t, flag);
}

void IRegisterController::setFormat(Format f, const GroupsName& group)
{
    foreach (const GroupsName & g, namesOfRegisterGroups()) {
        if (g == group) {
            int i = m_formatsModes[g.index()].formats.indexOf(f);
            if (i != -1) {
                m_formatsModes[g.index()].formats.remove(i);
                m_formatsModes[g.index()].formats.prepend(f);
            }
        }
    }
}

QString IRegisterController::numberForName(const QString& name) const
{
    //Requests for number come in order(if the previous was, let's say 10, then most likely the next one will be 11)
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

void IRegisterController::setStructuredRegister(const Register& reg, const GroupsName& group)
{
    Register r = reg;
    r.value = r.value.trimmed();
    r.value.replace(' ', ',');
    if (r.value.contains(',')) {
        r.value.append('}');
        r.value.prepend('{');
    }

    r.name += '.' + Converters::modeToString(m_formatsModes[group.index()].modes.first());

    setGeneralRegister(r, group);
}

void IRegisterController::structuredRegistersHandler(const GDBMI::ResultRecord& r)
{
    //Parsing records in format like:
    //{u8 = {0, 0, 128, 146, 0, 48, 197, 65}, u16 = {0, 37504, 12288, 16837}, u32 = {2457862144, 1103441920}, u64 = 4739246961893310464, f32 = {-8.07793567e-28, 24.6484375}, f64 = 710934821}
    //{u8 = {0 <repeats 16 times>}, u16 = {0, 0, 0, 0, 0, 0, 0, 0}, u32 = {0, 0, 0, 0}, u64 = {0, 0}, f32 = {0, 0, 0, 0}, f64 = {0, 0}}

    QRegExp rx("^\\s*=\\s*\\{(.*)\\}");
    rx.setMinimal(true);

    QString registerName;
    Mode currentMode = LAST_MODE;
    GroupsName group;
    const GDBMI::Value& values = r["register-values"];

    Q_ASSERT(!m_rawRegisterNames.isEmpty());

    for (int i = 0; i < values.size(); ++i) {
        const GDBMI::Value& entry = values[i];
        int number = entry["number"].literal().toInt();
        registerName = m_rawRegisterNames[number];
        if (currentMode == LAST_MODE) {
            group = groupForRegisterName(registerName);
            currentMode = modes(group).first();
        }

        QString record = entry["value"].literal();
        int start = record.indexOf(Converters::modeToString(currentMode));
        Q_ASSERT(start != -1);
        start += Converters::modeToString(currentMode).size();

        QString value = record.right(record.size() - start);
        int idx = rx.indexIn(value);
        value = rx.cap(1);

        if (idx == -1) {
            //if here then value without braces: u64 = 4739246961893310464, f32 = {-8.07793567e-28, 24.6484375}, f64 = 710934821}
            QRegExp rx2("=\\s+(.*)(\\}|,)");
            rx2.setMinimal(true);
            rx2.indexIn(record, start);
            value = rx2.cap(1);
        }
        value = value.trimmed().remove(',');
        m_registers.insert(registerName, value);
    }

    if (m_pendingGroups.contains(group)) {
        emit registersChanged(registersFromGroup(group));
        m_pendingGroups.remove(m_pendingGroups.indexOf(group));
    }
}

QVector< Mode > IRegisterController::modes(const GroupsName& group)
{
    int idx = -1;
    foreach (const GroupsName & g, namesOfRegisterGroups()) {
        if (g == group) {
            idx = g.index();
        }
    }
    Q_ASSERT(idx != -1);
    return m_formatsModes[idx].modes;
}

void IRegisterController::setMode(Mode m, const GroupsName& group)
{
    foreach (const GroupsName & g, namesOfRegisterGroups()) {
        if (g == group) {
            int i = m_formatsModes[g.index()].modes.indexOf(m);
            if (i != -1) {
                m_formatsModes[g.index()].modes.remove(i);
                m_formatsModes[g.index()].modes.prepend(m);
            }
        }
    }
}

}
