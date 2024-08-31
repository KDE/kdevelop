/*
    SPDX-FileCopyrightText: 2021 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_SOURCEFORMATTERCONFIG_H
#define KDEVPLATFORM_SOURCEFORMATTERCONFIG_H

#include "debug.h"

#include <KConfigGroup>

#include <QMimeType>
#include <QString>
#include <QStringView>

#include <utility>

namespace KDevelop {
namespace SourceFormatter {

class ConfigForMimeType
{
public:
    explicit ConfigForMimeType(const KConfigGroup& sourceFormatterConfig, const QMimeType& mimeType)
        : ConfigForMimeType(sourceFormatterConfig, mimeType.name())
    {
    }
    explicit ConfigForMimeType(const KConfigGroup& sourceFormatterConfig, const QString& mimeTypeName);

    bool isValid() const
    {
        return m_formatterEndPos != 0;
    }

    QStringView formatterName() const
    {
        Q_ASSERT(isValid());
        return QStringView{m_entry.constData(), m_formatterEndPos};
    }

    QStringView styleName() const
    {
        Q_ASSERT(isValid());
        return QStringView{m_entry}.sliced(stylePos());
    }

    QString takeStyleName() &&
    {
        Q_ASSERT(isValid());
        // the order of the following statements is important
        m_entry.remove(0, stylePos());
        m_formatterEndPos = 0;

        Q_ASSERT(!isValid());
        return std::move(m_entry);
    }

    template<class Style>
    static void writeEntry(KConfigGroup& sourceFormatterConfig, const QMimeType& mimeType, const QString& formatterName,
                           const Style* style)
    {
        if (style) {
            QString entry = formatterName + delimiter + style->name();
            sourceFormatterConfig.writeEntry(mimeType.name(), std::move(entry));
        } else {
            sourceFormatterConfig.deleteEntry(mimeType.name());
        }
    }

private:
    static constexpr QLatin1String delimiter{"||", 2};

    qsizetype stylePos() const
    {
        return m_formatterEndPos + delimiter.size();
    }

    QString m_entry;
    qsizetype m_formatterEndPos = 0;
};

inline ConfigForMimeType::ConfigForMimeType(const KConfigGroup& sourceFormatterConfig, const QString& mimeTypeName)
{
    m_entry = sourceFormatterConfig.readEntry(mimeTypeName, QString{});
    if (m_entry.isEmpty()) {
        Q_ASSERT(!isValid());
        return;
    }

    m_formatterEndPos = m_entry.indexOf(delimiter);
    if (m_formatterEndPos <= 0 || stylePos() >= m_entry.size()) {
        qCWarning(SHELL) << "Broken formatting entry for mime type" << mimeTypeName << ":" << m_entry;
        m_formatterEndPos = 0;
        Q_ASSERT(!isValid());
        return;
    }

    Q_ASSERT(isValid());
}

}
}

#endif // KDEVPLATFORM_SOURCEFORMATTERCONFIG_H
