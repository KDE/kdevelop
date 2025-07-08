/*
    SPDX-FileCopyrightText: 2013 Vlas Puhov <vlas.puhov@mail.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _REGISTERCONTROLLER_H_
#define _REGISTERCONTROLLER_H_

#include <QHash>
#include <QVector>
#include <QObject>
#include <QStringList>
#include <QString>

namespace KDevMI {
namespace MI
{
struct ResultRecord;
}

class MIDebugSession;

enum RegisterType {general, structured, flag, floatPoint};

class GroupsName
{
public:
    QString name() const { return _name;}
    int index() const {return _index;}
    RegisterType type() const{return _type; }
    QString flagName() const{return _flagName;}

    bool operator==(const GroupsName& g) const {return _name == g.name();}

    GroupsName() {}

private:
    GroupsName(const QString& name, int idx, RegisterType type = general, const QString& flag = QString()): _name(name), _index(idx), _type(type), _flagName(flag) {}

private:
    QString _name;
    int _index = -1; ///Should be unique for each group for current architecture (0, 1...n).
    RegisterType _type = general;
    QString _flagName; ///Used only for flag registers.

    friend class IRegisterController;
    friend struct RegistersGroup;
};

enum Format {
    Binary,
    Octal,
    Decimal,
    Hexadecimal,
    Raw,
    Unsigned,

    LAST_FORMAT
};

enum Mode {
    natural,

    v4_float,
    v2_double,
    v4_int32,
    v2_int64,

    u32,
    u64,
    f32,
    f64,

    LAST_MODE
};

struct FormatsModes {
    QVector<Format> formats;
    QVector<Mode> modes;
};

///Register in format: @p name, @p value - space separated list of values
struct Register {
    Register() {}
    Register(const QString& _name, const QString& _value): name(_name), value(_value) {}
    QString name;
    QString value;
};
///List of @p registers for @p groupName in @p format
struct RegistersGroup {
    RegistersGroup()

    {}

    GroupsName groupName;
    QVector<Register> registers;
    Format format = Binary; ///<Current format
    bool flag = false; ///<true if this group is flags group.
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
    ///There'll be at least 2 groups: "General" and "Flags", also "XMM", "FPU", "Segment" for x86, x86_64 architectures.
    virtual QVector<GroupsName> namesOfRegisterGroups() const = 0;

    ///Returns all supported formats for @p group (bin, dec, hex ...)
    QVector<Format> formats(const GroupsName& group);

    ///Sets current format for the @p group, if format is supported. Does nothing otherwise.
    void setFormat(Format f, const GroupsName& group);

    ///Returns all supported modes for @p group (i.e. how to display group: 2 int, 4 float or other number of columns)
    QVector<Mode> modes(const GroupsName& group);

    ///Sets current mode for the @p group, if mode is supported. Does nothing otherwise.
    void setMode(Mode m, const GroupsName& group);

Q_SIGNALS:
    ///Emits @p group with updated registers.
    void registersChanged(const RegistersGroup& g);

public Q_SLOTS:
    ///Updates registers in @p group. If @p group is empty - updates all registers.
    virtual void updateRegisters(const GroupsName& group = GroupsName());

    ///Sends updated register's @p reg value to the debugger.
    virtual void setRegisterValue(const Register& reg);

protected:
    /**
     * Create a register controller.
     *
     * @param debugSession a non-null debug session that must remain valid throughout this controller's lifetime
     */
    explicit IRegisterController(MIDebugSession* debugSession, QObject* parent = nullptr);

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

    GroupsName createGroupName(const QString& name, int idx, RegisterType t = general, const QString& flag = QString()) const;

    ///Returns register's number for @p name.
    QString numberForName(const QString& name) const;

public:
    ~IRegisterController() override;

private :
    ///Handles initialization of register's names.
    void registerNamesHandler(const MI::ResultRecord& r);

    ///Parses new values for general registers from @p r and updates it in m_registers.
    ///Emits registersChanged signal.
    void generalRegistersHandler(const MI::ResultRecord& r);

    ///Parses new values for structured registers from @p r and updates it in m_registers.
    ///Emits registersChanged signal.
    virtual void structuredRegistersHandler(const MI::ResultRecord& r);

private:

    ///Groups that should be updated(emitted @p registersInGroupChanged signal), if empty - all.
    QVector<GroupsName> m_pendingGroups;

protected:
    ///Register names as it sees debugger (in format: number, name).
    QVector<QString > m_rawRegisterNames;

    ///Registers in format: name, value
    QHash<QString, QString > m_registers;

    ///Supported formats and modes for each register's group. First format/mode is current.
    QVector<FormatsModes > m_formatsModes;

    ///Current debug session;
    MIDebugSession* const m_debugSession;
};

} // end of namespace KDevMI

Q_DECLARE_TYPEINFO(KDevMI::Register, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(KDevMI::RegistersGroup, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(KDevMI::FlagRegister, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(KDevMI::GroupsName, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(KDevMI::FormatsModes, Q_MOVABLE_TYPE);

#endif
