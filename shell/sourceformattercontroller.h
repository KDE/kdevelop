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

#ifndef SOURCEFORMATTERCONTROLLER_H
#define SOURCEFORMATTERCONTROLLER_H

#include <interfaces/isourceformattercontroller.h>

#include <QtCore/QHash>
#include <QtCore/QList>
#include <KDE/KMimeType>
#include <KDE/KConfigGroup>

#include "shellexport.h"

namespace KDevelop
{
        class ISourceFormatter;
	class IPlugin;

typedef QHash<QString, QList<KDevelop::IPlugin*> > IPluginHash;

/** \short A singleton class managing all source formatter plugins
 */
class KDEVPLATFORMSHELL_EXPORT SourceFormatterController : public ISourceFormatterController
{
		Q_OBJECT

	public:
		SourceFormatterController(QObject *parent = 0);
		virtual ~SourceFormatterController();
		void initialize();
                void cleanup();

		/** \return The instance of this singleton.
		*/
		static SourceFormatterController* self();

		/** \return the formatter for the active language.
		*/
		ISourceFormatter* activeFormatter();
		/** \return The formatter corresponding to the language
		* of the document corresponding to the \arg url.
		*/
		ISourceFormatter* formatterForUrl(const KUrl &url);
		/** Loads and returns a source formatter for this mime type.
		* The language is then activated and the style is loaded.
		* The source formatter is then ready to use on a file.
		*/
		ISourceFormatter* formatterForMimeType(const KMimeType::Ptr &mime);
		/** \return Whether this mime type is supported by any plugin.
		*/
		bool isMimeTypeSupported(const KMimeType::Ptr &mime);

		/** A list of all languages (corresponding to a
		* \ref KDevelop::ILanguageSupport) supported by loaded plugins.
		*/
		QStringList languages();
		/** \return the language name corresponding to the mime type \arg name.
		*/
		QString languageNameForMimeType(const KMimeType::Ptr &mime);
		/** \return All loaded plugins for this \arg lang.
		*/
		QList<KDevelop::IPlugin*> pluginListForLanguage(const QString &lang);
		/** \return One mime type corresponding to the language \arg lang.
		* It is useful because ISourceFormatter always expects a mime type,
		* eg in preview text when the text does not correspond to any file.
		*/
		QString mimeTypeForLanguage(const QString &lang);
		/** \return The name of an icon for the language.
		*/
		QString iconForLanguage(const QString &lang);

		/** Sets the active language (and optionnaly plugin). If \arg plugin is not given,
		* the plugin used is the one defined in the config.
		* The current style for this plugin is applied.
		*/
		void setActiveLanguage(const QString &lang, QString plugin = QString());
		QString activeLanguage() const { return m_currentLang; }

		/** Reloads the config from the config file. It just clears any stored config
		* and reparse the config file (ie discarding any non saved change).
		*/
		void loadConfig();
		/** Save the config to disk.
		*/
		void saveConfig();
		/** \return the config group corresponding to the current language and plugin
		* (where styles are saved).
		*/
		KConfigGroup configGroup() const {
			return m_activeConfigGroup;
		}

		/** \return the current style for the currently active language and plugin.
		*/
		QString currentStyle() const;
		/** Sets the style used by the current plugin (and language).
		* \arg style can be the name of a predefined style or a custom style.
		* It will be loaded from config file if necessary and applied to the source formatter.
		*/
		void setCurrentStyle(const QString &style);

		/** Saves a custom style to the config, in the \ref configGroup()
		*/
		void saveStyle(const QString &name, const QString &content);
		/** Changes the caption of a custom style in the config file.
		*/
		void renameStyle(const QString &name, const QString &caption);
		/** Deletes a style from the config style.
		*/
		void deleteStyle(const QString &name);
		/** \return A name for a new style. It will be "User"+ the lowest
		* number available.
		*/
		QString nameForNewStyle();

		/** \return A modeline string (to add at the end or the beginning of a file)
		* corresponding to the settings of the active language.
		*/
		QString addModelineForCurrentLang(QString input, const KMimeType::Ptr &mime);
		void setModelinesEnabled(bool enable);
		bool modelinesEnabled() const {
			return m_modelinesEnabled;
		}

	protected:
		/** \return the language support plugin corresponding to
		* a mime type name.
		*/
		KDevelop::IPlugin* languageSupportForMimeType(const QString &name);
		/** \return The name of the language corresponding to this mime type name.
		*/
		QString languageNameFromLanguageSupport(const QString &mime);
		void loadPlugins();

		ISourceFormatter* formatterForLanguage(const QString &language);
		/** \return the formatter for the currently avtive language named \arg name.
		*/
		ISourceFormatter* formatterByName(const QString &language, const QString &name);
		/** \return The name of kate indentation mode for the mime type.
		* examples are cstyle, python, etc.
		*/
		QString indentationMode(const KMimeType::Ptr &mime);

	private:
		static SourceFormatterController *m_instance;
		// all availables plugins and languages
		IPluginHash m_plugins;
		QHash<QString, QString> m_languages;
		// currently selected plugins, styles and languages
		QHash<QString, ISourceFormatter*> m_currentPlugins;
		QString m_currentStyle;
		KConfigGroup m_rootConfigGroup;
		KConfigGroup m_activeConfigGroup;
		QString m_currentLang;
		// config
		bool m_modelinesEnabled;
};

}

#endif // SOURCEFORMATTERMANAGER_H

// kate: indent-mode cstyle; space-indent off; tab-width 4;
