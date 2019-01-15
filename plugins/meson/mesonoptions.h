/* This file is part of KDevelop
    Copyright 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#pragma once

#include <QString>
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
    explicit MesonOptionBase(QString name, QString description, Section section);
    virtual ~MesonOptionBase();

    virtual Type type() const = 0;
    virtual QString value() const = 0;
    virtual QString initialValue() const = 0;
    virtual void reset() = 0;

    QString name() const;
    QString description() const;
    Section section() const;

    QString mesonArg() const;
    bool isUpdated() const;

    static MesonOptionPtr fromJSON(QJsonObject const& obj);

private:
    QString m_name;
    QString m_description;
    Section m_section;
};

class MesonOptionArray : public MesonOptionBase
{
public:
    MesonOptionArray(QString name, QString description, Section section, QStringList value);

    MesonOptionBase::Type type() const override;
    QString value() const override;
    QString initialValue() const override;
    void reset() override;

    QStringList rawValue() const;
    void setValue(QStringList val);

private:
    QStringList m_value;
    QStringList m_initialValue;
};

class MesonOptionBool : public MesonOptionBase
{
public:
    MesonOptionBool(QString name, QString description, Section section, bool value);

    MesonOptionBase::Type type() const override;
    QString value() const override;
    QString initialValue() const override;
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
    MesonOptionCombo(QString name, QString description, Section section, QString value, QStringList choices);

    MesonOptionBase::Type type() const override;
    QString value() const override;
    QString initialValue() const override;
    void reset() override;

    QString rawValue() const;
    void setValue(QString val);
    QStringList choices() const;

private:
    QString m_value;
    QString m_initialValue;
    QStringList m_choices;
};

class MesonOptionInteger : public MesonOptionBase
{
public:
    MesonOptionInteger(QString name, QString description, Section section, int value);

    MesonOptionBase::Type type() const override;
    QString value() const override;
    QString initialValue() const override;
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
    MesonOptionString(QString name, QString description, Section section, QString value);

    MesonOptionBase::Type type() const override;
    QString value() const override;
    QString initialValue() const override;
    void reset() override;

    QString rawValue() const;
    void setValue(QString val);

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
    explicit MesonOptions(QJsonArray const& arr);

    int numChanged() const;
    QStringList getMesonArgs() const;
    void print() const;

    QVector<MesonOptionPtr> options();

private:
    QVector<MesonOptionPtr> m_options;

    void fromJSON(QJsonArray const& arr);
};
