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

#ifndef _REGISTERCONTROLLER_H_
#define _REGISTERCONTROLLER_H_

#include <QHash>
#include <QVector>
#include <QObject>
#include <QStringList>
#include <QString>

namespace GDBMI
{
struct ResultRecord;
}

namespace GDBDebugger
{

class DebugSession;

enum RegistersFormat {
    Binary = 2,
    Octal = 8,
    Decimal = 10,
    Hexadecimal = 16,
    Raw,
    Natural
};
///Register in format: @p name, @p value
struct Register {
    Register() {}
    Register(const QString& _name, const QString& _value): name(_name), value(_value) {}
    QString name;
    QString value;
};
///List of @p registers for @p groupName in @p format
struct RegistersGroup {
    RegistersGroup(): flag(false), editable(true) {}
    QString groupName;
    QVector<Register> registers;
    RegistersFormat format; ///<Current format
    bool flag; ///<true if this group is flags group.
    bool editable; ///<indicates if registers can be edited.
};

struct FlagRegister {
    QStringList flags;
    QStringList bits;
    QString registerName;
    QString groupName;
};

/** @brief Class for managing registers: it can retrieve, change and send registers back to the debugger.*/
class IRegisterController : public QObject
{
    Q_OBJECT

public:
    ///Sets session @p debugSession to send commands to.
    void setSession(DebugSession* debugSession);

    ///There'll be at least 2 groups: "General" and "Flags", also "XMM", "FPU", "Segment" for x86, x86_64 architectures.
    virtual QStringList namesOfRegisterGroups() const = 0;

signals:
    ///Emits @p group with updated registers.
    void registersChanged(const RegistersGroup& g);

public slots:
    ///Updates registers in @p group. If @p group is empty - updates all registers.
    virtual void updateRegisters(const QString& group = QString());

    ///Sends updated register's @p reg value to the debugger.
    virtual void setRegisterValue(const Register& reg);

protected:
    IRegisterController(DebugSession* debugSession = 0, QObject* parent = 0);

    ///Returns registers from the @p group, or empty registers group if @p group is invalid.
    virtual RegistersGroup registersFromGroup(const QString& group, RegistersFormat format = Raw) const = 0;

    ///Sets value for @p register from @p group.
    virtual void setRegisterValueForGroup(const QString& group, const Register& reg) = 0;

    ///Return names of all registers for @p group.
    virtual QStringList registerNamesForGroup(const QString& group) const = 0;

    /**Updates value for each register in the group.
     * @param [out] registers Registers which values should be updated.
     */
    virtual void updateValuesForRegisters(RegistersGroup* registers) const;

    ///Sets new value for register @p reg, from group @p group.
    virtual void setGeneralRegister(const Register& reg, const QString& group);

    /**Converts values for each register in the group.
    * @param [out] registers Registers which values should be converted.
    * @param format Format used for conversion.
    */
    virtual void convertValuesForGroup(RegistersGroup* registersGroup, RegistersFormat format = Raw) const;

    ///Returns value for the given @p name, empty string if the name is incorrect or there is no registers yet.
    QString registerValue(const QString& name) const;

    /** Sets a flag register.
     * @param reg register to set
     * @param flag flag register @p reg belongs to.
     */
    void setFlagRegister(const Register& reg, const FlagRegister& flag);

    ///Updates values in @p flagsGroup for @p flagRegister.
    void updateFlagValues(RegistersGroup* flagsGroup, const FlagRegister& flagRegister) const;

    ///Returns group that given register belongs to.
    QString groupForRegisterName(const QString& name) const;

    ///Initializes registers, that is gets names of all available registers. Returns true is succeed.
    bool initializeRegisters();

public:
    virtual ~IRegisterController();

private :
    ///Handles initialization of register's names.
    void registerNamesHandler(const GDBMI::ResultRecord& r);

    ///Handles updated values for registers.
    void updateRegisterValuesHandler(const GDBMI::ResultRecord& r);

private:
    ///Register names as it sees debugger (in format: number, name).
    QVector<QString > m_rawRegisterNames;

    ///Groups that should be updated(emitted @p registersInGroupChanged signal), if empty - all.
    QStringList m_pendingGroups;

protected:
    ///Registers in format: name, value
    QHash<QString, QString > m_registers;

    ///Current debug session;
    DebugSession* m_debugSession;
};

}

Q_DECLARE_TYPEINFO(GDBDebugger::Register, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(GDBDebugger::RegistersGroup, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(GDBDebugger::FlagRegister, Q_MOVABLE_TYPE);

#endif
