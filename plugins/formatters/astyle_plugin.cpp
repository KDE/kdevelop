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

#include "astyle_plugin.h"

#include <KGenericFactory>

#include "astyle_formatter.h"
#include "astyle_stringiterator.h"
#include "astyle_preferences.h"

using namespace KDevelop;

K_PLUGIN_FACTORY(AStyleFactory, registerPlugin<AStylePlugin>();)
K_EXPORT_PLUGIN(AStyleFactory("kdevastyle"))

AStylePlugin::AStylePlugin(QObject *parent, const QVariantList&)
		: IPlugin(AStyleFactory::componentData(), parent)
{
    KDEV_USE_EXTENSION_INTERFACE(ISourceFormatter)
	m_formatter = new AStyleFormatter();
}

AStylePlugin::~AStylePlugin()
{
}

QString AStylePlugin::name()
{
    return "astyle";
}

QString AStylePlugin::caption()
{
    return "Artistic Style";
}

QString AStylePlugin::description()
{
    return i18n("<b>Artistic Style</b> is a source code indenter, formatter,"
        " and beautifier for the C, C++, C# and Java programming languages.\n"
        "Home Page: <a href=\"http://astyle.sourceforge.net/\">http://astyle.sourceforge.net</a>");
}

QString AStylePlugin::highlightModeForMime(const KMimeType::Ptr &mime)
{
    if(mime->is("text/x-java-source"))
        return "Java";
    else if(mime->is("text/x-csharp"))
        return "C#";
    return "C++";
}

QString AStylePlugin::formatSource(const QString &text, const KMimeType::Ptr &mime)
{
    if(mime->is("text/x-java-source"))
        m_formatter->setSharpStyle();
    else if(mime->is("text/x-csharp"))
        m_formatter->setJavaStyle();
    else
        m_formatter->setCStyle();
    
    return m_formatter->formatSource(text);
}

QMap<QString, QString> AStylePlugin::predefinedStyles(const KMimeType::Ptr &)
{
    QMap<QString, QString> styles;
    styles.insert("ANSI", "ANSI");
    styles.insert("GNU", "GNU");
    styles.insert("Java", "Java");
    styles.insert("KR", "Kernighan & Ritchie");
    styles.insert("Linux", "Linux");

    return styles;
}

void AStylePlugin::setStyle(const QString &name, const QString &content)
{
    if(!name.isEmpty())
        m_formatter->predefinedStyle(name);
    else
        m_formatter->loadStyle(content);
}

SettingsWidget* AStylePlugin::editStyleWidget(const KMimeType::Ptr &mime)
{
    AStylePreferences::Language lang = AStylePreferences::CPP;
    if(mime->is("text/x-java-source"))
        lang = AStylePreferences::Java;
    else if(mime->is("text/x-csharp"))
        lang = AStylePreferences::CSharp;
    return new AStylePreferences(lang);
}

QString AStylePlugin::previewText(const KMimeType::Ptr &)
{
    return "// Indentation\n" + indentingSample() + "\t// Formatting\n"
        + formattingSample();
}

ISourceFormatter::IndentationType AStylePlugin::indentationType()
{
    QString s = m_formatter->option("Fill").toString();
    if(s == "Tabs")
        return ISourceFormatter::IndentWithTabs;
    else if(m_formatter->option("FillForce").toBool())
        return ISourceFormatter::IndentWithSpacesAndConvertTabs;
    else
        return ISourceFormatter::IndentWithSpaces;
}

int AStylePlugin::indentationLength()
{
    return m_formatter->option("FillCount").toInt();
}

QString AStylePlugin::formattingSample()
{
    return "void func(){\n\tif(isFoo(a,b))\n\tbar(a,b);\nif(isFoo)"
    "\n\ta=bar((b-c)*a,*d--);\nif(  isFoo( a,b ) )\n\tbar(a, b);"
    "\nif (isFoo) {isFoo=false;cat << isFoo <<endl;}\nif(isFoo)DoBar();"
    "if (isFoo){\n\tbar();\n}\n\telse if(isBar())"
    "{\n\tannotherBar();\n}\n}\n";
}

QString AStylePlugin::indentingSample()
{
    return "#define foobar(A)\\\n{Foo();Bar();}\n#define"
    "anotherFoo(B)\\\nreturn Bar()\n\nnamespace Bar\n{\nclass Foo"
    "\n{public:\nFoo();\nvirtual ~Foo();\n};\nswitch (foo)\n"
    "{\ncase 1:\na+=1;\nbreak;\ncase 2:\n{\na += 2;\n break;\n}"
    "\n}\nif (isFoo)\n{\nbar();\n}\nelse\n{\nanotherBar();\n}"
    "\nint foo()\n\twhile(isFoo)\n\t\t{\n\t\t\t...\n\t\t\tgoto error;"
    "\n\t\t....\n\t\terror:\n\t\t\t...\n\t\t}\n\t}\nfooArray[]"
    "={ red,\n\tgreen,\n\tdarkblue};\nfooFunction(barArg1,"
    "\n\tbarArg2,\n\tbarArg3);\n";
}

#include "astyle_plugin.moc"
