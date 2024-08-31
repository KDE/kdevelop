/*
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "isourceformatter.h"

#include <KLocalizedString>

#include <QDebug>
#include <QDebugStateSaver>
#include <QVariant>
#include <QMimeType>

#include <algorithm>

namespace KDevelop
{

SettingsWidget::SettingsWidget(QWidget *parent)
		: QWidget(parent)
{
}

SettingsWidget::~SettingsWidget()
{
}

ISourceFormatter::~ISourceFormatter()
{
}

QString ISourceFormatter::usageHint() const
{
    return QString();
}

SourceFormatterStyle::SourceFormatterStyle()
{
}

SourceFormatterStyle::SourceFormatterStyle(const QString &name)
	: m_usePreview(false)
	, m_name(name)
{
}

void SourceFormatterStyle::setContent(const QString &content)
{
	m_content = content;
}

void SourceFormatterStyle::setCaption(const QString &caption)
{
	m_caption = caption;
}

QString SourceFormatterStyle::content() const
{
	return m_content;
}

QString SourceFormatterStyle::description() const
{
	return m_description;
}

void SourceFormatterStyle::setDescription(const QString &desc)
{
	m_description = desc;
}

bool SourceFormatterStyle::usePreview() const
{
	return m_usePreview;
}

void SourceFormatterStyle::setUsePreview(bool use)
{
	m_usePreview = use;
}

void SourceFormatterStyle::setMimeTypes(const SourceFormatterStyle::MimeList& types)
{
	m_mimeTypes = types;
}

void SourceFormatterStyle::setMimeTypes(const QStringList& types)
{
	for (auto& t : types) {
		auto items = t.split(QLatin1Char('|'));
		if (items.size() != 2 || items.at(0).isEmpty() || items.at(1).isEmpty()) {
			// An empty MIME type name is invalid and not useful.
			// Language names are displayed in a combobox. So an empty language name is unacceptable.
			qWarning() << "Skipping invalid Mime/Highlight pair in MimeTypes config entry for"
			           << *this << ':' << t;
			continue;
		}
		m_mimeTypes << MimeHighlightPair{items.at(0), items.at(1)};
	}
}

void SourceFormatterStyle::setOverrideSample(const QString &sample)
{
	m_overrideSample = sample;
}

QString SourceFormatterStyle::overrideSample() const
{
	return m_overrideSample;
}

SourceFormatterStyle::MimeList SourceFormatterStyle::mimeTypes() const
{
	return m_mimeTypes;
}

QVariant SourceFormatterStyle::mimeTypesVariant() const
{
	QStringList result;
	result.reserve(m_mimeTypes.size());
	for ( const auto& item: m_mimeTypes ) {
		result << item.mimeType + QLatin1Char('|') + item.highlightMode;
	}
	return QVariant::fromValue(result);
}

bool SourceFormatterStyle::supportsLanguage(const QString &language) const
{
	for ( const auto& item: m_mimeTypes ) {
		if ( item.highlightMode == language ) {
			return true;
		}
	}
	return false;
}

QString SourceFormatterStyle::modeForMimetype(const QMimeType& mime) const
{
	const auto mimeTypes = this->mimeTypes();
	for (const auto& item : mimeTypes) {
		if (mime.inherits(item.mimeType)) {
			return item.highlightMode;
		}
	}
	return QString();
}

void SourceFormatterStyle::copyDataFrom(const SourceFormatterStyle& other)
{
    m_usePreview = other.m_usePreview;
    m_content = other.m_content;
    m_mimeTypes = other.m_mimeTypes;
    m_overrideSample = other.m_overrideSample;
}

QDebug operator<<(QDebug dbg, const SourceFormatterStyle& style)
{
    QDebugStateSaver saver(dbg);
    // For a given formatter, a style is uniquely identified by its name. But style names never appear in
    // the UI, only in config files. Style captions feature prominently in the UI. However, multiple styles
    // can have equal captions. Print both the name and the caption to allow quick and easy style
    // recognition by caption and reliable identification by name.
    dbg.nospace() << "SourceFormatterStyle{name=" << style.name() << ", caption=" << style.caption() << '}';
    return dbg;
}

SourceFormatterStyle ISourceFormatter::predefinedStyle(const QString& name) const
{
    const auto styles = predefinedStyles();
    const auto it = std::find_if(styles.cbegin(), styles.cend(), [&name](const SourceFormatterStyle& style) {
        return style.name() == name;
    });
    return it == styles.cend() ? SourceFormatterStyle{name} : *it;
}

QString ISourceFormatter::optionMapToString(const QMap<QString, QVariant> &map)
{
	QString options;
	QMap<QString, QVariant>::const_iterator it = map.constBegin();
	for (; it != map.constEnd(); ++it) {
		options += it.key() + QLatin1Char('=') + it.value().toString() + QLatin1Char(',');
	}
	return options;
}

QMap<QString, QVariant> ISourceFormatter::stringToOptionMap(const QString &options)
{
    QMap<QString, QVariant> map;
    const auto pairs = QStringView{options}.split(QLatin1Char(','), Qt::SkipEmptyParts);
    for (const auto pair : pairs) {
        const int pos = pair.indexOf(QLatin1Char('='));
        if (pos == -1) {
            qWarning() << "Skipping invalid option without '=' in a source formatter style's Content config entry:"
                       << pair;
            continue;
        }
        map.insert(pair.first(pos).toString(), pair.sliced(pos + 1).toString());
    }
    return map;
}

SourceFormatterStyle::MimeList ISourceFormatter::mimeTypesSupportedByBuiltInStyles()
{
    static const SourceFormatterStyle::MimeList list = {
        {QStringLiteral("text/x-c++src"), QStringLiteral("C++")},
        {QStringLiteral("text/x-csrc"), QStringLiteral("C")},
        {QStringLiteral("text/x-chdr"), QStringLiteral("C")},
        {QStringLiteral("text/x-c++hdr"), QStringLiteral("C++")},
        {QStringLiteral("text/x-java"), QStringLiteral("Java")},
        {QStringLiteral("text/x-csharp"), QStringLiteral("C#")},
        {QStringLiteral("text/x-objcsrc"), QStringLiteral("Objective-C")},
        {QStringLiteral("text/x-objc++src"), QStringLiteral("Objective-C++")},
    };
    return list;
}

QString ISourceFormatter::missingExecutableMessage(const QString &name)
{
	return i18n("The executable %1 cannot be found. Please make sure"
	" it is installed and can be executed. <br />"
	"The plugin will not work until you fix this problem.", QLatin1String("<b>") + name + QLatin1String("</b>"));
}

}

#include "moc_isourceformatter.cpp"
