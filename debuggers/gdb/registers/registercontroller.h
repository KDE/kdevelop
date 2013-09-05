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

enum RegisterType {general, structured, flag, floatPoint};

class GroupsName
{
public:
    QString name() const { return _name;}
    int index() const {return _index;}
    RegisterType type() const{return _type; }
    QString flagName() const{return _flagName;}

    bool operator==(const GroupsName& g) const {return _name == g.name();}

    GroupsName(): _index(-1), _type(general) {}

private:
    GroupsName(const QString& name, int idx, RegisterType type = general, const QString flag = QString()): _name(name), _index(idx), _type(type), _flagName(flag) {}

private:
    QString _name;
    int _index; ///Should be unique for each group for current architecture (0, 1...n).
    RegisterType _type;
    QString _flagName; ///Used only for flag registers.

    friend class IRegisterController;
    friend struct RegistersGroup;
};

enum Format {
    Binary = 2,
    Octal = 8,
    Decimal = 10,
    Hexadecimal = 16,
    Raw,

    v4_float,
    v2_double,
    v4_int32,
    v2_int64,

    u32,
    u64,
    f32,
    f64,
    LAST_FORMAT
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
    RegistersGroup(): flag(false) {}
    GroupsName groupName;
    QVector<Register> registers;
    Format format; ///<Current format
    bool flag; ///<true if this group is flags group.
};

struct FlagRegister {
    QStringList flags;
    QStringList bits;
    QString registerName;
    GroupsName groupName;
};

/** @brief Class for managing registers: it can retrieve, change and send registers back to the debugger.*/
class IRegisterController : public QObject
{
    Q_OBJECT

public:
    ///Sets session @p debugSession to send commands to.
    void setSession(DebugSession* debugSession);

    ///There'll be at least 2 groups: "General" and "Flags", also "XMM", "FPU", "Segment" for x86, x86_64 architectures.
    virtual QVector<GroupsName> namesOfRegisterGroups() const = 0;

    ///Returns all supported formats for @p group
    QVector<Format> formats(const GroupsName& group);

    ///Sets current format for the @p group, if format is supported. Does nothing otherwise.
    void setFormat(Format f, const GroupsName& group);

signals:
    ///Emits @p group with updated registers.
    void registersChanged(const RegistersGroup& g);

public slots:
    ///Updates registers in @p group. If @p group is empty - updates all registers.
    virtual void updateRegisters(const GroupsName& group = GroupsName());

    ///Sends updated register's @p reg value to the debugger.
    virtual void setRegisterValue(const Register& reg);

protected:
    IRegisterController(DebugSession* debugSession = 0, QObject* parent = 0);

    ///Returns registers from the @p group, or empty registers group if @p group is invalid.
    virtual RegistersGroup registersFromGroup(const GroupsName& group) const = 0;

    ///Sets value for @p register from @p group.
    virtual void setRegisterValueForGroup(const GroupsName& group, const Register& reg) = 0;

    ///Returns names of all registers for @p group.
    virtual QStringList registerNamesForGroup(const GroupsName& group) const = 0;

    /**Updates value for each register in the group.
     * @param [out] registers Registers which values should be updated.
     */
    virtual void updateValuesForRegisters(RegistersGroup* registers) const;

    /**Converts values for each register in the group.
    * @param [out] registers Registers which values should be converted.
    */
    virtual void convertValuesForGroup(RegistersGroup* registersGroup) const;

    ///Returns value for the given @p name, empty string if the name is incorrect or there is no registers yet.
    QString registerValue(const QString& name) const;

    /** Sets a flag register.
     * @param reg register to set
     * @param flag flag register @p reg belongs to.
     */
    void setFlagRegister(const Register& reg, const FlagRegister& flag);

    ///Sets new value for register @p reg, from group @p group.
    void setGeneralRegister(const Register& reg, const GroupsName& group);

    ///Sets new value for structured register(XMM, VFP quad and other) @p reg, from group @p group.
    void setStructuredRegister(const Register& reg, const GroupsName& group);

    ///Updates values in @p flagsGroup for @p flagRegister.
    void updateFlagValues(RegistersGroup* flagsGroup, const FlagRegister& flagRegister) const;

    ///Returns group that given register belongs to.
    GroupsName groupForRegisterName(const QString& name) const;

    ///Initializes registers, that is gets names of all available registers. Returns true is succeed.
    bool initializeRegisters();

    GroupsName createGroupName(const QString& name, int idx, RegisterType t = general, const QString flag = QString()) const;

    ///Returns register's number for @p name.
    QString numberForName(const QString& name) const;

public:
    virtual ~IRegisterController();

private :
    ///Handles initialization of register's names.
    void registerNamesHandler(const GDBMI::ResultRecord& r);

    ///Parses new values for general registers from @p r and updates it in m_registers.
    ///Emits registersChanged signal.
    void generalRegistersHandler(const GDBMI::ResultRecord& r);

    ///Parses new values for structured registers from @p r and updates it in m_registers.
    ///Emits registersChanged signal.
    virtual void structuredRegistersHandler(const GDBMI::ResultRecord& r);

private:

    ///Groups that should be updated(emitted @p registersInGroupChanged signal), if empty - all.
    QVector<GroupsName> m_pendingGroups;

protected:
    ///Register names as it sees debugger (in format: number, name).
    QVector<QString > m_rawRegisterNames;

    ///Registers in format: name, value
    QHash<QString, QString > m_registers;

    ///Supported formats for each register's group. First format is current.
    QVector<QVector<Format> > m_formats;

    ///Current debug session;
    DebugSession* m_debugSession;
};

}

Q_DECLARE_TYPEINFO(GDBDebugger::Register, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(GDBDebugger::RegistersGroup, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(GDBDebugger::FlagRegister, Q_MOVABLE_TYPE);

#endif
