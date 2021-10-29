/*
    SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QString>
#include <QStringList>
#include <QVector>
#include <memory>

class QJsonArray;
class QJsonObject;
class QWidget;
class MesonOptionBase;
class MesonOptions;

using MesonOptionPtr = std::shared_ptr<MesonOptionBase>;
using MesonOptsPtr = std::shared_ptr<MesonOptions>;

/*!
 * Base class for a single meson option.
 */
class MesonOptionBase
{
public:
    enum Section { CORE, BACKEND, BASE, COMPILER, DIRECTORY, USER, TEST };
    enum Type { ARRAY, BOOLEAN, COMBO, INTEGER, STRING };

public:
    explicit MesonOptionBase(const QString& name, const QString& description, Section section);
    virtual ~MesonOptionBase();

    virtual Type type() const = 0;
    virtual QString value() const = 0;
    virtual QString initialValue() const = 0;
    virtual void setFromString(const QString& value) = 0;
    virtual void reset() = 0;

    QString name() const;
    QString description() const;
    Section section() const;

    QString mesonArg() const;
    bool isUpdated() const;

    static MesonOptionPtr fromJSON(const QJsonObject& obj);

private:
    QString m_name;
    QString m_description;
    Section m_section;
};

class MesonOptionArray : public MesonOptionBase
{
public:
    MesonOptionArray(const QString& name, const QString& description, Section section, QStringList value);

    MesonOptionBase::Type type() const override;
    QString value() const override;
    QString initialValue() const override;
    void setFromString(const QString& value) override;
    void reset() override;

    QStringList rawValue() const;
    void setValue(const QStringList& val);

private:
    QStringList m_value;
    QStringList m_initialValue;
};

class MesonOptionBool : public MesonOptionBase
{
public:
    MesonOptionBool(const QString& name, const QString& description, Section section, bool value);

    MesonOptionBase::Type type() const override;
    QString value() const override;
    QString initialValue() const override;
    void setFromString(const QString& value) override;
    void reset() override;

    bool rawValue() const;
    void setValue(bool val);

private:
    bool m_value;
    bool m_initialValue;
};

class MesonOptionCombo : public MesonOptionBase
{
public:
    MesonOptionCombo(const QString& name, const QString& description, Section section, QString value,
                     QStringList choices);

    MesonOptionBase::Type type() const override;
    QString value() const override;
    QString initialValue() const override;
    void setFromString(const QString& value) override;
    void reset() override;

    QString rawValue() const;
    void setValue(const QString& val);
    QStringList choices() const;

private:
    QString m_value;
    QString m_initialValue;
    QStringList m_choices;
};

class MesonOptionInteger : public MesonOptionBase
{
public:
    MesonOptionInteger(const QString& name, const QString& description, Section section, int value);

    MesonOptionBase::Type type() const override;
    QString value() const override;
    QString initialValue() const override;
    void setFromString(const QString& value) override;
    void reset() override;

    int rawValue() const;
    void setValue(int val);

private:
    int m_value;
    int m_initialValue;
};

class MesonOptionString : public MesonOptionBase
{
public:
    MesonOptionString(const QString& name, const QString& description, Section section, QString value);

    MesonOptionBase::Type type() const override;
    QString value() const override;
    QString initialValue() const override;
    void setFromString(const QString& value) override;
    void reset() override;

    QString rawValue() const;
    void setValue(const QString& val);

private:
    QString m_value;
    QString m_initialValue;
};

/*!
 * Container class for all meson project options.
 */
class MesonOptions
{
public:
    explicit MesonOptions(const QJsonArray& arr);

    int numChanged() const;
    QStringList getMesonArgs() const;
    void print() const;

    QVector<MesonOptionPtr> options();

private:
    QVector<MesonOptionPtr> m_options;

    void fromJSON(const QJsonArray& arr);
};
