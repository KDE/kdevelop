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

#include "sourceformattercontroller.h"

#include <QVariant>
#include <QStringList>
#include <QRegExp>
#include <KPluginInfo>
#include <KDebug>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/interfaces/ilanguagesupport.h>
#include <interfaces/isourceformatter.h>

namespace KDevelop
{

SourceFormatterController::SourceFormatterController(QObject *parent)
		: ISourceFormatterController(parent), m_modelinesEnabled(false)
{
}

void SourceFormatterController::initialize()
{
	loadPlugins();
	m_rootConfigGroup = KGlobal::config()->group("SourceFormatter");
}

SourceFormatterController::~SourceFormatterController()
{
}

void SourceFormatterController::loadPlugins()
{
	KDevelop::IPluginController *controller = KDevelop::ICore::self()->pluginController();

	foreach(KDevelop::IPlugin *p,
	        controller->allPluginsForExtension("org.kdevelop.ISourceFormatter")) {
		KPluginInfo info = controller->pluginInfo(p);
		QVariant mimes = info.property("X-KDevelop-SupportedMimeTypes");
		kDebug() << "Found plugin " << info.name() << " for mimes " << mimes << endl;

		QStringList mimeTypes = mimes.toStringList();
		foreach(const QString &s, mimeTypes) {
			QString lang = languageNameFromLanguageSupport(s);
			QList<KDevelop::IPlugin*> &list = m_plugins[lang];
			if (!list.contains(p))
				list.append(p);
			kDebug() << "Loading plugin " << info.name() << " for type " << s
			<< " and lang " << lang << endl;
		}
	}
}

KDevelop::IPlugin*
SourceFormatterController::languageSupportForMimeType(const QString &name)
{
	KDevelop::IPluginController *controller = KDevelop::ICore::self()->pluginController();
	QStringList constraints;
	constraints << QString("'%1' in [X-KDevelop-SupportedMimeTypes]").arg(name);

	QList<KDevelop::IPlugin*> list = controller->allPluginsForExtension("ILanguageSupport", constraints);
	foreach(KDevelop::IPlugin *p, list)
		return p;

	return 0;
}

QString SourceFormatterController::languageNameFromLanguageSupport(const QString &name)
{
	if (m_languages.contains(name))
		return m_languages[name];

	// we're loading the plugin, find the name from the language support plugin
	KDevelop::IPlugin *p = languageSupportForMimeType(name);
	if (p) {
		KDevelop::ILanguageSupport *languageSupport = p->extension<KDevelop::ILanguageSupport>();
		if (languageSupport) {
			m_languages.insert(name, languageSupport->name());
			return languageSupport->name();
		}
	}
	// temp to support languages with no language support
	m_languages.insert(name, name);
	return name;
}

QString SourceFormatterController::languageNameForMimeType(const KMimeType::Ptr &mime)
{
	if (m_languages.contains(mime->name()))
		return m_languages[mime->name()];
	return QString();
}

QString SourceFormatterController::iconForLanguage(const QString &lang)
{
	//  QString mime = mimeTypeForLanguage(lang);
	KDevelop::IPlugin *p = KDevelop::ICore::self()
	        ->pluginController()->pluginForExtension("org.kdevelop.ILanguageSupport", lang);//languageForMimeType(mime);
	if (p) {
		KPluginInfo info = KDevelop::ICore::self()->pluginController()->pluginInfo(p);
		return info.icon();
	}
	return lang;
}

QList<KDevelop::IPlugin*> SourceFormatterController::pluginListForLanguage(const QString &lang)
{
	if (m_plugins.contains(lang))
		return m_plugins[lang];
	return QList<KDevelop::IPlugin*>();
}

ISourceFormatter* SourceFormatterController::activeFormatter()
{
	return m_currentPlugins[m_currentLang];
}

ISourceFormatter* SourceFormatterController::formatterForUrl(const KUrl &url)
{
	KMimeType::Ptr mime = KMimeType::findByUrl(url);
	return formatterForMimeType(mime);
}

ISourceFormatter* SourceFormatterController::formatterForMimeType(const KMimeType::Ptr &mime)
{
	if (!m_languages.contains(mime->name())) //unknown mime type
		return 0;

	setActiveLanguage(languageNameForMimeType(mime));
	kDebug() << "About to format file " << mime->name() << m_currentLang << endl;
	return formatterForLanguage(m_languages[mime->name()]);
}

bool SourceFormatterController::isMimeTypeSupported(const KMimeType::Ptr &mime)
{
	return m_languages.contains(mime->name());
}

ISourceFormatter* SourceFormatterController::formatterForLanguage(const QString &language)
{
	if (m_currentPlugins.contains(language))
		return m_currentPlugins[language];

	// else load the current plugin from config
	KConfigGroup langGroup = m_rootConfigGroup.group(language);
	QString formatterName = langGroup.readEntry("Plugin", "");
	if (formatterName.isEmpty()) { // no formatters defined yet, load the first one
		KDevelop::IPlugin *p = m_plugins[language].first();
		if (!p)
			return 0;
		m_currentPlugins[language] = p->extension<ISourceFormatter>();
	} else
		m_currentPlugins[language] = formatterByName(language, formatterName);

	return m_currentPlugins[language];
}

ISourceFormatter* SourceFormatterController::formatterByName(const QString &language, const QString &name)
{
	QList<KDevelop::IPlugin*> list = m_plugins[language];
	foreach(KDevelop::IPlugin *p, list) {
		if (p) {
			ISourceFormatter *f = p->extension<ISourceFormatter>();
			if (f && (f->name() == name))
				return f;
		}
	}
	return 0;
}

// void SourceFormatterController::setFormatterForLanguage(const QString &lang, ISourceFormatter *formatter)
// {
//     m_currentPlugins[lang] = formatter;
// }

void SourceFormatterController::setActiveLanguage(const QString &lang, QString plugin)
{
	// find the plugin for this language
	if (plugin.isEmpty()) {
		if (m_currentLang == lang)
			return; // no change
		ISourceFormatter *f = formatterForLanguage(lang);
		if (f)
			plugin = f->name();
	} else
		m_currentPlugins[lang] = formatterByName(lang, plugin);
	kDebug() << "Activating language " << lang << " with plugin " << plugin << " == " << m_currentPlugins[lang] << endl;

	if (plugin.isEmpty())
		kDebug() << "Cannot find a suitable plugin for language " << lang << endl;

	// update the plugin entry in config
// 	m_rootConfigGroup.group(lang).writeEntry("Plugin", plugin);
	m_activeConfigGroup = m_rootConfigGroup.group(lang).group(plugin);
	m_currentLang = lang;

	// load the current style for this language
	QString styleName = m_activeConfigGroup.readEntry("Style", "");
	if (!styleName.isEmpty() && m_currentPlugins[lang])
		setCurrentStyle(styleName);
}

QStringList SourceFormatterController::languages()
{
	QStringList list;
	QHash<QString, QString>::const_iterator it = m_languages.constBegin();
	for (; it != m_languages.constEnd(); ++it) {
		if (!list.contains(it.value()))
			list.append(it.value());
	}
	return list;
}

QString SourceFormatterController::mimeTypeForLanguage(const QString &lang)
{
	QHash<QString, QString>::const_iterator it = m_languages.constBegin();
	for (; it != m_languages.constEnd(); ++it) {
		if (it.value() == lang)
			return it.key();
	}
	return QString();
}

void SourceFormatterController::loadConfig()
{
	// reload config that may have been modified by config dialog
	m_currentPlugins.clear();
	m_currentLang.clear();

	m_modelinesEnabled = m_rootConfigGroup.readEntry("ModelinesEnabled",
	        QVariant(false)).toBool();
//     // load current plugins and styles
//     foreach(QString l, languages()) {
//         KConfigGroup langGroup = m_configGroup.group(l);
//         QString plugin = langGroup.readEntry("Plugin", "");
//         if(!plugin.isEmpty()) {
//             KConfigGroup pluginGroup = langGroup.group(plugin);
//             m_currentPlugins[l] = formatterByName(l, plugin);
// //             m_currentStyles[l] = pluginGroup.readEntry("Style","");
//         }
//     }
}

void SourceFormatterController::saveConfig()
{
	// save current plugins
	QHash<QString, ISourceFormatter*>::const_iterator it = m_currentPlugins.constBegin();
	for (; it != m_currentPlugins.constEnd(); ++it) {
		ISourceFormatter *f = it.value();
		if(f) {
			QString pluginName = f->name();
			m_rootConfigGroup.group(it.key()).writeEntry("Plugin", pluginName);
		}
	}

	m_rootConfigGroup.writeEntry("ModelinesEnabled", m_modelinesEnabled);
	m_rootConfigGroup.sync();
}

QString SourceFormatterController::currentStyle() const
{
    return m_currentStyle;
// 	return m_activeConfigGroup.readEntry("Style", "");
}

void SourceFormatterController::setCurrentStyle(const QString &style)
{
	if (!m_currentPlugins[m_currentLang]) {
		kDebug() << "currrent plugin is null for style " << m_currentLang << style << endl;
		return;
	}
	kDebug() << "style is " << style << endl;

	if (m_activeConfigGroup.hasKey(style)) { // custom style
		QString content = m_activeConfigGroup.readEntry(style);
		m_currentPlugins[m_currentLang]->setStyle(QString(), content);
	} else // predefined style
		m_currentPlugins[m_currentLang]->setStyle(style);

	m_currentStyle = style;
// 	m_activeConfigGroup.writeEntry("Style", style);
}

void SourceFormatterController::saveStyle(const QString &name, const QString &content)
{
	m_activeConfigGroup.writeEntry(name, content);
}

void SourceFormatterController::renameStyle(const QString &name, const QString &caption)
{
	m_activeConfigGroup.writeEntry("Caption" + name.mid(4), caption);
}

void SourceFormatterController::deleteStyle(const QString &name)
{
	m_activeConfigGroup.deleteEntry(name);
}

QString SourceFormatterController::nameForNewStyle()
{
	//find available number
	int idx = 1;
	QString s = "User" + QString::number(idx);
	while (m_activeConfigGroup.hasKey(s)) {
		++idx;
		s = "User" + QString::number(idx);
	}

	return s;
}

QString SourceFormatterController::indentationMode(const KMimeType::Ptr &mime)
{
	if (mime->is("text/x-c++src") || mime->is("text/x-chdr") ||
	        mime->is("text/x-c++hdr") || mime->is("text/x-csrc") ||
	        mime->is("text/x-java") || mime->is("text/x-csharp"))
		return "cstyle";
	return "none";
}

QString SourceFormatterController::addModelineForCurrentLang(QString input, const KMimeType::Ptr &mime)
{
	if (!m_currentPlugins[m_currentLang] || !m_modelinesEnabled)
		return input;

	QString output;
	QTextStream os(&output, QIODevice::WriteOnly);
	QTextStream is(&input, QIODevice::ReadOnly);

    QString modeline("// kate: ");
	QString length = QString::number(m_currentPlugins[m_currentLang]->indentationLength());
	// add indentation style
	modeline.append("indent-mode ").append(indentationMode(mime)).append("; ");

	ISourceFormatter::IndentationType type = m_currentPlugins[m_currentLang]->indentationType();
	if (type == ISourceFormatter::IndentWithTabs) {
		modeline.append("replace-tabs off; ");
		modeline.append("tab-width ").append(length).append("; ");
	} else {
		modeline.append("space-indent on; ");
		modeline.append("indent-width ").append(length).append("; ");
		if (type == ISourceFormatter::IndentWithSpacesAndConvertTabs)
			modeline.append("replace-tabs on; ");
	}

	kDebug() << "created modeline: " << modeline << endl;

	bool modelinefound = false;
	QRegExp kateModeline("^\\s*//\\s*kate:(.*)$");
	QRegExp knownOptions("\\s*(indent-width|space-indent|tab-width|indent-mode)");
	while (!is.atEnd()) {
		QString line = is.readLine();
		// replace only the options we care about
		if (kateModeline.indexIn(line) >= 0) { // match
			kDebug() << "Found a kate modeline: " << line << endl;
			modelinefound = true;
			QString options = kateModeline.cap(1);
			QStringList optionList = options.split(';', QString::SkipEmptyParts);

			os <<  modeline;
			foreach(const QString &s, optionList) {
				if (knownOptions.indexIn(s) < 0) { // unknown option, add it
					os << s << ";";
					kDebug() << "Found unknown option: " << s << endl;
				}
			}
			os << endl;
		} else
			os << line << endl;
	}

	if (!modelinefound)
		os << modeline << endl;
	return output;
}

void SourceFormatterController::setModelinesEnabled(bool enable)
{
	m_modelinesEnabled = enable;
}

void SourceFormatterController::cleanup()
{
}

}

#include "sourceformattercontroller.moc"

// kate: indent-mode cstyle; space-indent off; tab-width 4;

