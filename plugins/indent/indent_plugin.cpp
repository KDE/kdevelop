/* This file is part of KDevelop
*  Copyright (C) 2008 Cédric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2001 Matthias Hölzer-Klüpfel <mhk@caldera.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#include "indent_plugin.h"

#include <KGenericFactory>

using namespace KDevelop;

K_PLUGIN_FACTORY(IndentFactory, registerPlugin<IndentPlugin>();)
K_EXPORT_PLUGIN(IndentFactory("kdevindent"))

IndentPlugin::IndentPlugin(QObject *parent, const QVariantList&)
: IPlugin(IndentFactory::componentData(), parent)
{
    KDEV_USE_EXTENSION_INTERFACE(ISourceFormatter)
}

IndentPlugin::~IndentPlugin()
{
}

QString IndentPlugin::name()
{
    return "astyle";
}

QString IndentPlugin::caption()
{
    return "Artistic Style";
}

QString IndentPlugin::description()
{
    return i18n("<b>Artistic Style</b> is a source code indenter, formatter,"
        " and beautifier for the C, C++, C# and Java programming languages.\n"
        "Home Page: <a href=\"http://astyle.sourceforge.net/\">http://astyle.sourceforge.net</a>");
}

QString IndentPlugin::highlightModeForMime(const KMimeType::Ptr &mime)
{
    if(mime->is("text/x-java-source"))
        return "Java";
    else if(mime->is("text/x-csharp"))
        return "C#";
    return "C++";
}

QString IndentPlugin::formatSource(const QString &text, const KMimeType::Ptr &mime)
{
    if(mime->is("text/x-java-source"))
        m_formatter->setSharpStyle();
    else if(mime->is("text/x-csharp"))
        m_formatter->setJavaStyle();
    else
        m_formatter->setCStyle();
    
    return m_formatter->formatSource(text);
}

QMap<QString, QString> IndentPlugin::predefinedStyles(const KMimeType::Ptr &)
{
    QMap<QString, QString> styles;
    styles.insert("ANSI", "ANSI");
    styles.insert("GNU", "GNU");
    styles.insert("Java", "Java");
    styles.insert("KR", "Kernighan & Ritchie");
    styles.insert("Linux", "Linux");

    return styles;
}

void IndentPlugin::setStyle(const QString &name, const QString &content)
{
    if(!name.isEmpty())
        m_formatter->predefinedStyle(name);
    else
        m_formatter->loadStyle(content);
}

SettingsWidget* IndentPlugin::editStyleWidget(const KMimeType::Ptr &mime)
{
    AStylePreferences::Language lang = AStylePreferences::CPP;
    if(mime->is("text/x-java-source"))
        lang = AStylePreferences::Java;
    else if(mime->is("text/x-csharp"))
        lang = AStylePreferences::CSharp;
    return new AStylePreferences(lang);
}

QString IndentPlugin::previewText(const KMimeType::Ptr &)
{
    return "// Indentation\n" + indentingSample() + "\t// Formatting\n"
        + formattingSample();
}

ISourceFormatter::IndentationType IndentPlugin::indentationType()
{
    QString s = m_formatter->option("Fill").toString();
    if(s == "Tabs")
        return ISourceFormatter::IndentWithTabs;
    else if(m_formatter->option("FillForce").toBool())
        return ISourceFormatter::IndentWithSpacesAndConvertTabs;
    else
        return ISourceFormatter::IndentWithSpaces;
}

int IndentPlugin::indentationLength()
{
    return m_formatter->option("FillCount").toInt();
}

#include "indent_plugin.moc"
