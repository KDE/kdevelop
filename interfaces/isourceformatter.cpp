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

#include <KDE/KLocale>

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
