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

#ifndef ISOURCEFORMATTERCONTROLLER_H
#define ISOURCEFORMATTERCONTROLLER_H

#include <QtCore/QHash>
#include <QtCore/QList>
#include <KDE/KMimeType>
#include <KDE/KConfigGroup>

#include "interfacesexport.h"

namespace KDevelop
{
        class ISourceFormatter;
	class IPlugin;

typedef QHash<QString, QList<KDevelop::IPlugin*> > IPluginHash;

/** \short An interface to the controller managing all source formatter plugins
 */
class KDEVPLATFORMINTERFACES_EXPORT ISourceFormatterController : public QObject
{
		Q_OBJECT

	public:
		ISourceFormatterController(QObject *parent = 0);
		virtual ~ISourceFormatterController();

		/** \return the formatter for the active language.
		*/
		virtual ISourceFormatter* activeFormatter() = 0;
		/** \return The formatter corresponding to the language
		* of the document corresponding to the \arg url.
		*/
		virtual ISourceFormatter* formatterForUrl(const KUrl &url) = 0;
		/** Loads and returns a source formatter for this mime type.
		* The language is then activated and the style is loaded.
		* The source formatter is then ready to use on a file.
		*/
		virtual ISourceFormatter* formatterForMimeType(const KMimeType::Ptr &mime) = 0;
		/** \return Whether this mime type is supported by any plugin.
		*/
		virtual bool isMimeTypeSupported(const KMimeType::Ptr &mime) = 0;

		/** A ist of all languages (corresponding to a
		* \ref KDevelop::ILanguageSupport) supported by loaded plugins.
		*/
		virtual QStringList languages() = 0;
		/** \return the language name corresponding to the mime type \arg name.
		*/
		virtual QString languageNameForMimeType(const KMimeType::Ptr &mime) = 0;
		/** \return All loaded plugins for this \arg lang.
		*/
		virtual QList<KDevelop::IPlugin*> pluginListForLanguage(const QString &lang) = 0;
		/** \return One mime type corresponding to the language \arg lang.
		* It is useful because ISourceFormatter always expects a mime type,
		* eg in preview text when the text does not correspond to any file.
		*/
		virtual QString mimeTypeForLanguage(const QString &lang) = 0;
		/** \return The name of an icon for the language.
		*/
		virtual QString iconForLanguage(const QString &lang) = 0;

		/** Sets the active language (and optionnaly plugin). If \arg plugin is not given,
		* the plugin used is the one defined in the config.
		* The current style for this plugin is applied.
		*/
		virtual void setActiveLanguage(const QString &lang, QString plugin = QString()) = 0;
		virtual QString activeLanguage() const = 0;

		/** Reloads the config from the config file. It just clears any stored config
		* and reparse the config file (ie discarding any non saved change).
		*/
		virtual void loadConfig() = 0;
		/** Save the config to disk.
		*/
		virtual void saveConfig() = 0;
		/** \return the config group corresponding to the current language and plugin
		* (where styles are saved).
		*/
		virtual KConfigGroup configGroup() const = 0;

		/** \return the current style for the currently active language and plugin.
		*/
		virtual QString currentStyle() const = 0;
		/** Sets the style used by the current plugin (and language).
		* \arg style can be the name of a predefined style or a custom style.
		* It will be loaded from config file if necessary and applied to the source formatter.
		*/
		virtual void setCurrentStyle(const QString &style) = 0;

		/** Saves a custom style to the config, in the \ref configGroup()
		*/
		virtual void saveStyle(const QString &name, const QString &content) = 0;
		/** Changes the caption of a custom style in the config file.
		*/
		virtual void renameStyle(const QString &name, const QString &caption) = 0;
		/** Deletes a style from the config style.
		*/
		virtual void deleteStyle(const QString &name) = 0;
		/** \return A name for a new style. It will be "User"+ the lowest
		* number available.
		*/
		virtual QString nameForNewStyle() = 0;

		/** \return A modeline string (to add at the end or the beginning of a file)
		* corresponding to the settings of the active language.
		*/
		virtual QString addModelineForCurrentLang(QString input, const KMimeType::Ptr &mime) = 0;
		virtual void setModelinesEnabled(bool enable) = 0;
		virtual bool modelinesEnabled() const = 0;
};

}

#endif // ISOURCEFORMATTERCONTROLLER_H

// kate: indent-mode cstyle; space-indent off; tab-width 4 = 0;
