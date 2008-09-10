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
#ifndef ISOURCEFORMATTER_H
#define ISOURCEFORMATTER_H

#include <QWidget>
#include <QStringList>
#include <KMimeType>

#include <interfaces/iextension.h>
#include "../utilexport.h"

class KDEVPLATFORMUTIL_EXPORT SettingsWidget : public QWidget
{
		Q_OBJECT

	public:
		SettingsWidget(QWidget *parent = 0);
		virtual ~SettingsWidget();

		virtual void load(const QString &name, const QString &content) = 0;

		virtual QString save() = 0;

	signals:
		void previewTextChanged(const QString &text);
};

/**
 * @short An interface for a source beautifier
 *
 *
 * @author Cédric Pasteur
 */
class KDEVPLATFORMUTIL_EXPORT ISourceFormatter
{
	public:
		virtual ~ISourceFormatter();

		enum IndentationType {
			NoChange,
			IndentWithTabs,
			IndentWithSpaces,
			IndentWithSpacesAndConvertTabs
		};

		virtual QString name() = 0;
		virtual QString caption() = 0;
		virtual QString description() = 0;

		/** \return The highlighting mode for Kate corresponding to this mime.
		*/
		virtual QString highlightModeForMime(const KMimeType::Ptr &mime) = 0;

		/** Formats using the current style.
		*/
		virtual QString formatSource(const QString &text, const KMimeType::Ptr &mime) = 0;

		/** \return A map of predefined styles (a key and a caption for each type)
		*/
		virtual QMap<QString, QString> predefinedStyles(const KMimeType::Ptr &mime) = 0;
		/** Load the predefined type of name \arg name, or if the first arg is empty, the style
		*   defined by the options string \arg content.
		*/
		virtual void setStyle(const QString &name, const QString &content = QString()) = 0;

		/** \return The widget to edit a style.
		*/
		virtual SettingsWidget* editStyleWidget(const KMimeType::Ptr &mime) = 0;

		/** \return The text used in the config dialog to preview the current style.
		*/
		virtual QString previewText(const KMimeType::Ptr &mime) = 0;

		/** \return The indentation type of the currently selected style.
		*/
		virtual IndentationType indentationType() = 0;
		/** \return The number of spaces used for indentation if IndentWithSpaces is used,
		* or the number of spaces per tab if IndentWithTabs is selected.
		*/
		virtual int indentationLength() = 0;

		static QString optionMapToString(const QMap<QString, QVariant> &map);
		static QMap<QString, QVariant> stringToOptionMap(const QString &option);
};

KDEV_DECLARE_EXTENSION_INTERFACE(ISourceFormatter, "org.kdevelop.ISourceFormatter")
Q_DECLARE_INTERFACE(ISourceFormatter, "org.kdevelop.ISourceFormatter")

#endif // ISOURCEFORMATTER_H
// kate: indent-mode cstyle; space-indent off; tab-width 4; 
