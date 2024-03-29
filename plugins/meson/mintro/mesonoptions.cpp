/*
    SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mesonoptions.h"

#include <debug.h>

#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QWidget>

#include <algorithm>
#include <vector>

using namespace std;

static const QHash<QString, MesonOptionBase::Section> STRING2SECTION = {
    { QStringLiteral("core"), MesonOptionBase::CORE },
    { QStringLiteral("backend"), MesonOptionBase::BACKEND },
    { QStringLiteral("base"), MesonOptionBase::BASE },
    { QStringLiteral("compiler"), MesonOptionBase::COMPILER },
    { QStringLiteral("directory"), MesonOptionBase::DIRECTORY },
    { QStringLiteral("user"), MesonOptionBase::USER },
    { QStringLiteral("test"), MesonOptionBase::TEST },
};

static const QHash<QString, MesonOptionBase::Type> STRING2TYPE = {
    { QStringLiteral("array"), MesonOptionBase::ARRAY },   { QStringLiteral("boolean"), MesonOptionBase::BOOLEAN },
    { QStringLiteral("combo"), MesonOptionBase::COMBO },   { QStringLiteral("integer"), MesonOptionBase::INTEGER },
    { QStringLiteral("string"), MesonOptionBase::STRING },
};

MesonOptions::MesonOptions(const QJsonArray& arr)
{
    fromJSON(arr);
}

// Option constructors

MesonOptionBase::MesonOptionBase(const QString& name, const QString& description, MesonOptionBase::Section section)
    : m_name(name)
    , m_description(description)
    , m_section(section)
{
}

MesonOptionArray::MesonOptionArray(const QString& name, const QString& description, MesonOptionBase::Section section,
                                   QStringList value)
    : MesonOptionBase(name, description, section)
    , m_value(value)
    , m_initialValue(value)
{
}

MesonOptionBool::MesonOptionBool(const QString& name, const QString& description, MesonOptionBase::Section section,
                                 bool value)
    : MesonOptionBase(name, description, section)
    , m_value(value)
    , m_initialValue(value)
{
}

MesonOptionCombo::MesonOptionCombo(const QString& name, const QString& description, MesonOptionBase::Section section,
                                   QString value, QStringList choices)
    : MesonOptionBase(name, description, section)
    , m_value(value)
    , m_initialValue(value)
    , m_choices(choices)
{
}

MesonOptionInteger::MesonOptionInteger(const QString& name, const QString& description,
                                       MesonOptionBase::Section section, int value)
    : MesonOptionBase(name, description, section)
    , m_value(value)
    , m_initialValue(value)
{
}

MesonOptionString::MesonOptionString(const QString& name, const QString& description, MesonOptionBase::Section section,
                                     QString value)
    : MesonOptionBase(name, description, section)
    , m_value(value)
    , m_initialValue(value)
{
}

QStringList MesonOptionCombo::choices() const
{
    return m_choices;
}

// Type functions

MesonOptionBase::Type MesonOptionArray::type() const
{
    return ARRAY;
}

MesonOptionBase::Type MesonOptionBool::type() const
{
    return BOOLEAN;
}

MesonOptionBase::Type MesonOptionCombo::type() const
{
    return COMBO;
}

MesonOptionBase::Type MesonOptionInteger::type() const
{
    return INTEGER;
}

MesonOptionBase::Type MesonOptionString::type() const
{
    return STRING;
}

// Value functions

QString MesonOptionArray::value() const
{
    QStringList tmp;
    tmp.reserve(m_value.size());
    transform(begin(m_value), end(m_value), back_inserter(tmp),
              [](const QString& str) -> QString { return QStringLiteral("'") + str + QStringLiteral("'"); });
    return QStringLiteral("[") + tmp.join(QStringLiteral(", ")) + QStringLiteral("]");
}

QString MesonOptionBool::value() const
{
    return m_value ? QStringLiteral("true") : QStringLiteral("false");
}

QString MesonOptionCombo::value() const
{
    return m_value;
}

QString MesonOptionInteger::value() const
{
    return QString::number(m_value);
}

QString MesonOptionString::value() const
{
    return m_value;
}

// Initial value functions

QString MesonOptionArray::initialValue() const
{
    QStringList tmp;
    tmp.reserve(m_initialValue.size());
    transform(begin(m_initialValue), end(m_initialValue), back_inserter(tmp),
              [](const QString& str) -> QString { return QStringLiteral("'") + str + QStringLiteral("'"); });
    return QStringLiteral("[") + tmp.join(QStringLiteral(", ")) + QStringLiteral("]");
}

QString MesonOptionBool::initialValue() const
{
    return m_initialValue ? QStringLiteral("true") : QStringLiteral("false");
}

QString MesonOptionCombo::initialValue() const
{
    return m_initialValue;
}

QString MesonOptionInteger::initialValue() const
{
    return QString::number(m_initialValue);
}

QString MesonOptionString::initialValue() const
{
    return m_initialValue;
}

// Reset functions

void MesonOptionArray::reset()
{
    m_value = m_initialValue;
}

void MesonOptionBool::reset()
{
    m_value = m_initialValue;
}

void MesonOptionCombo::reset()
{
    m_value = m_initialValue;
}

void MesonOptionInteger::reset()
{
    m_value = m_initialValue;
}

void MesonOptionString::reset()
{
    m_value = m_initialValue;
}

// Raw value functions

QStringList MesonOptionArray::rawValue() const
{
    return m_value;
}

bool MesonOptionBool::rawValue() const
{
    return m_value;
}

QString MesonOptionCombo::rawValue() const
{
    return m_value;
}

int MesonOptionInteger::rawValue() const
{
    return m_value;
}

QString MesonOptionString::rawValue() const
{
    return m_value;
}

// Set value functions

void MesonOptionArray::setValue(const QStringList& val)
{
    m_value = val;
}

void MesonOptionBool::setValue(bool val)
{
    m_value = val;
}

void MesonOptionCombo::setValue(const QString& val)
{
    m_value = val;
}

void MesonOptionInteger::setValue(int val)
{
    m_value = val;
}

void MesonOptionString::setValue(const QString& val)
{
    m_value = val;
}

// Set value from string

void MesonOptionArray::setFromString(const QString& value)
{
    setValue({ value });
}

void MesonOptionBool::setFromString(const QString& value)
{
    setValue(value.toLower() == QLatin1String("true"));
}

void MesonOptionCombo::setFromString(const QString& value)
{
    setValue(value);
}

void MesonOptionInteger::setFromString(const QString& value)
{
    setValue(value.toInt());
}

void MesonOptionString::setFromString(const QString& value)
{
    setValue(value);
}

// Base option functions

MesonOptionBase::~MesonOptionBase() {}

QString MesonOptionBase::name() const
{
    return m_name;
}

QString MesonOptionBase::description() const
{
    return m_description;
}

MesonOptionBase::Section MesonOptionBase::section() const
{
    return m_section;
}

QString MesonOptionBase::mesonArg() const
{
    return QStringLiteral("-D") + m_name + QStringLiteral("=") + value();
}

bool MesonOptionBase::isUpdated() const
{
    return value() != initialValue();
}

MesonOptionPtr MesonOptionBase::fromJSON(const QJsonObject& obj)
{
    auto nameJV = obj[QStringLiteral("name")];
    auto descriptionJV = obj[QStringLiteral("description")];
    auto sectionJV = obj[QStringLiteral("section")];
    auto typeJV = obj[QStringLiteral("type")];
    auto valueJV = obj[QStringLiteral("value")];

    // Check all values
    for (const auto& i : { nameJV, descriptionJV, sectionJV, typeJV }) {
        if (!i.isString()) {
            qCWarning(KDEV_Meson) << "OPT: Base type validation failed" << typeJV.toString();
            return nullptr;
        }
    }

    // Work around meson bug https://github.com/mesonbuild/meson/pull/5646 by
    // removing the first space and everything after that.
    QString sectionStr = sectionJV.toString();
    int spacePos = sectionStr.indexOf(QLatin1Char(' '));
    if (spacePos > 0) {
        sectionStr = sectionStr.left(spacePos);
    }

    auto sectionIter = STRING2SECTION.find(sectionStr);
    auto typeIter = STRING2TYPE.find(typeJV.toString());

    if (sectionIter == end(STRING2SECTION) || typeIter == end(STRING2TYPE)) {
        qCWarning(KDEV_Meson) << "OPT: Unknown type or section " << typeJV.toString() << " / " << sectionJV.toString();
        return nullptr;
    }

    Section section = *sectionIter;
    Type type = *typeIter;
    QString name = nameJV.toString();
    QString description = descriptionJV.toString();

    switch (type) {
    case ARRAY: {
        if (!valueJV.isArray()) {
            return nullptr;
        }

        QJsonArray raw = valueJV.toArray();
        QStringList values;
        values.reserve(raw.size());
        transform(begin(raw), end(raw), back_inserter(values), [](const QJsonValue& v) { return v.toString(); });
        return make_shared<MesonOptionArray>(name, description, section, values);
    }

    case BOOLEAN:
        if (!valueJV.isBool()) {
            return nullptr;
        }
        return make_shared<MesonOptionBool>(name, description, section, valueJV.toBool());

    case COMBO: {
        auto choicesJV = obj[QStringLiteral("choices")];
        if (!valueJV.isString() || !choicesJV.isArray()) {
            return nullptr;
        }

        QJsonArray raw = choicesJV.toArray();
        QStringList choices;
        choices.reserve(raw.size());
        transform(begin(raw), end(raw), back_inserter(choices), [](const QJsonValue& v) { return v.toString(); });
        return make_shared<MesonOptionCombo>(name, description, section, valueJV.toString(), choices);
    }

    case INTEGER:
        if (!valueJV.isDouble()) {
            return nullptr;
        }
        return make_shared<MesonOptionInteger>(name, description, section, valueJV.toInt());

    case STRING:
        if (!valueJV.isString()) {
            return nullptr;
        }
        return make_shared<MesonOptionString>(name, description, section, valueJV.toString());
    }

    qCWarning(KDEV_Meson) << "OPT: Unknown type " << typeJV.toString();
    return nullptr;
}

int MesonOptions::numChanged() const
{
    int sum = 0;
    for (auto i : m_options) {
        if (i && i->isUpdated()) {
            ++sum;
        }
    }
    return sum;
}

QStringList MesonOptions::getMesonArgs() const
{
    QStringList result;
    result.reserve(m_options.size());

    for (auto i : m_options) {
        if (i && i->isUpdated()) {
            result << i->mesonArg();
        }
    }
    return result;
}

void MesonOptions::fromJSON(const QJsonArray& arr)
{
    m_options.clear();
    m_options.reserve(arr.size());

    for (const QJsonValue& i : arr) {
        if (!i.isObject()) {
            continue;
        }

        auto ptr = MesonOptionBase::fromJSON(i.toObject());
        if (ptr) {
            m_options += ptr;
        } else {
            qCWarning(KDEV_Meson) << "OPT: Failed to parse " << i.toObject();
        }
    }
}

void MesonOptions::print() const
{
    for (const auto& i : m_options) {
        qCDebug(KDEV_Meson) << i->name() << " = " << i->value() << "  [" << i->type() << "] -- " << i->section();
    }
}

QVector<MesonOptionPtr> MesonOptions::options()
{
    return m_options;
}
