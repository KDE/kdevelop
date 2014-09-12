/* This file is part of KDevelop
   Copyright (C) 2008 Cédric Pasteur <cedric.pasteur@free.fr>

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
#include "isourceformatter.h"

#include <KLocalizedString>
#include <QVariant>

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

SourceFormatterStyle::SourceFormatterStyle()
	: m_usePreview(false)
{
};

SourceFormatterStyle::SourceFormatterStyle(const QString &name)
	: m_name(name)
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

QString SourceFormatterStyle::caption() const
{
	return m_caption;
}

QString SourceFormatterStyle::name() const
{
	return m_name;
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
	for ( auto t: types ) {
		auto items = t.split('|');
		if ( items.size() != 2 ) {
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
	for ( const auto& item: m_mimeTypes ) {
		result << item.mimeType + "|" + item.highlightMode;
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
	for (const auto& item : mimeTypes()) {
		if (mime.inherits(item.mimeType)) {
			return item.highlightMode;
		}
	}
	return QString();
}

void SourceFormatterStyle::copyDataFrom(SourceFormatterStyle *other)
{
	m_content = other->content();
	m_mimeTypes = other->mimeTypes();
	m_overrideSample = other->overrideSample();
}

QString ISourceFormatter::optionMapToString(const QMap<QString, QVariant> &map)
{
	QString options;
	QMap<QString, QVariant>::const_iterator it = map.constBegin();
	for (; it != map.constEnd(); ++it) {
		options += it.key();
		options += '=';
		options += it.value().toString();
		options += ',';
	}
	return options;
}

QMap<QString, QVariant> ISourceFormatter::stringToOptionMap(const QString &options)
{
	QMap<QString, QVariant> map;
	QStringList pairs = options.split(',', QString::SkipEmptyParts);
	QStringList::const_iterator it;
	for (it = pairs.constBegin(); it != pairs.constEnd(); ++it) {
		QStringList bits = (*it).split('=');
		map[bits[0]] = bits[1];
	}
	return map;
}

QString ISourceFormatter::missingExecutableMessage(const QString &name)
{
	return i18n("The executable %1 cannot be found. Please make sure"
	" it is installed and can be executed. <br />"
	"The plugin will not work until you fix this problem.", "<b>" + name + "</b>");
}

}

// kate: indent-mode cstyle; space-indent off; tab-width 4; 
