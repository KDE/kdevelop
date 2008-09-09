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

#ifndef SOURCEFORMATTERMANAGER_H
#define SOURCEFORMATTERMANAGER_H

#include <QHash>
#include <QList>
#include <KMimeType>
#include <KConfigGroup>

#include "utilexport.h"

class ISourceFormatter;
namespace KDevelop
{
	class IPlugin;
}

/**
 */
class KDEVPLATFORMUTIL_EXPORT SourceFormatterManager : public QObject
{
		Q_OBJECT

	public:
		SourceFormatterManager(QObject *parent = 0);
		virtual ~SourceFormatterManager();

		static SourceFormatterManager* self();

		/** \return the active formatter for this language.
		*/
		ISourceFormatter* activeFormatter();
		ISourceFormatter* formatterForUrl(const KUrl &url);
		/** Loads and returns a source formatter for this mime type.
		* The language is then activated and the style is loaded.
		* The source formatter is then ready to use on a file.
		*/
		ISourceFormatter* formatterForMimeType(const KMimeType::Ptr &mime);
		bool isMimeTypeSupported(const KMimeType::Ptr &mime);

		QStringList languages();
		/** \return the language name corresponding to the mime type \arg name.
		*/
		QString languageNameForMimeType(const KMimeType::Ptr &mime);
		QList<KDevelop::IPlugin*> pluginListForLanguage(const QString &lang);
		/** \return One mime type corresponding to the language \arg lang.
		* It is useful because ISourceFormatter always expects a mime type,
		* eg in preview text when the text does not correspond to any file.
		*/
		QString mimeTypeForLanguage(const QString &lang);
		QString iconForLanguage(const QString &lang);

		/** Sets the active language (and optionnaly plugin). If \arg plugin is not given,
		* the plugin used is the one defined in the config.
		* The current style for this plugin is applied.
		*/
		void setActiveLanguage(const QString &lang, QString plugin = QString());

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
		KConfigGroup configGroup() {
			return m_activeConfigGroup;
		}

		/** \return the current style for the currently active language and plugin.
		*/
		QString currentStyle();
		/** Sets the style used by the current plugin (and language).
		* \arg style can be the name of a predefined style or a custom style.
		* It will be loaded from config file if necessary and applied to the source formatter.
		*/
		void setCurrentStyle(const QString &style);

		void saveStyle(const QString &name, const QString &content);
		void renameStyle(const QString &name, const QString &caption);
		void deleteStyle(const QString &name);
		QString nameForNewStyle();

	protected:
		KDevelop::IPlugin* languageSupportForMimeType(const QString &name);
		QString languageNameFromLanguageSupport(const QString &mime);
		void loadPlugins();

		ISourceFormatter* formatterForLanguage(const QString &language);
		ISourceFormatter* formatterByName(const QString &language, const QString &name);

	private:
		static SourceFormatterManager *m_instance;
		// all availables plugins and languages
		QHash<QString, QList<KDevelop::IPlugin*> > m_plugins;
		QHash<QString, QString> m_languages;
		// currently selected plugins, styles and languages
		QHash<QString, ISourceFormatter*> m_currentPlugins;
		KConfigGroup m_rootConfigGroup;
		KConfigGroup m_activeConfigGroup;
		QString m_currentLang;
};

#endif // SOURCEFORMATTERMANAGER_H

