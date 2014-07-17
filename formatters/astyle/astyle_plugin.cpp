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

#include <QMimeDatabase>

#include <KPluginLoader>
#include <KPluginFactory>
#include <KAboutData>

#include <interfaces/icore.h>
#include <interfaces/isourceformattercontroller.h>

#include "astyle_formatter.h"
#include "astyle_stringiterator.h"
#include "astyle_preferences.h"
#include <KLocalizedString>
#include <KUrl>

using namespace KDevelop;

K_PLUGIN_FACTORY(AStyleFactory, registerPlugin<AStylePlugin>();)
// K_EXPORT_PLUGIN(AStyleFactory(KAboutData("kdevastyle","kdevformatters", ki18n("Astyle Formatter"), "0.1", ki18n("A formatting tool using astyle"), KAboutData::License_GPL)))

AStylePlugin::AStylePlugin(QObject *parent, const QVariantList&)
    : IPlugin("kdevastyle", parent)
{
    KDEV_USE_EXTENSION_INTERFACE(ISourceFormatter)
    m_formatter = new AStyleFormatter();
    currentStyle = predefinedStyles().at(0);
}

AStylePlugin::~AStylePlugin()
{
    delete m_formatter;
}

QString AStylePlugin::name()
{
    // This needs to match the X-KDE-PluginInfo-Name entry from the .desktop file!
    return "kdevastyle";
}

QString AStylePlugin::caption()
{
    return "Artistic Style";
}

QString AStylePlugin::description()
{
    return i18n("<b>Artistic Style</b> is a source code indenter, formatter,"
        " and beautifier for the C, C++, C# and Java programming languages.<br />"
        "Home Page: <a href=\"http://astyle.sourceforge.net/\">http://astyle.sourceforge.net</a>");
}

QString AStylePlugin::formatSourceWithStyle( SourceFormatterStyle s, const QString& text, const KUrl& /*url*/, const QMimeType& mime, const QString& leftContext, const QString& rightContext )
{
    if(mime.inherits("text/x-java"))
        m_formatter->setJavaStyle();
    else if(mime.inherits("text/x-csharp"))
        m_formatter->setSharpStyle();
    else
        m_formatter->setCStyle();

    if( s.content().isEmpty() )
    {
        m_formatter->predefinedStyle( s.name() );
    } else
    {
        m_formatter->loadStyle( s.content() );
    }
    
    return m_formatter->formatSource(text, leftContext, rightContext);
}

QString AStylePlugin::formatSource(const QString& text, const KUrl& url, const QMimeType& mime, const QString& leftContext, const QString& rightContext)
{
    return formatSourceWithStyle( KDevelop::ICore::self()->sourceFormatterController()->styleForMimeType( mime ), text, url, mime, leftContext, rightContext );
}

static SourceFormatterStyle::MimeList supportedMimeTypes()
{
    using P = SourceFormatterStyle::MimeHighlightPair;
    return SourceFormatterStyle::MimeList() << P{"text/x-c++src", "C++"} << P{"text/x-chdr", "C"}
                                            << P{"text/x-c++hdr", "C++"} << P{"text/x-csrc", "C"}
                                            << P{"text/x-java", "Java"} << P{"text/x-csharp", "C#"};
}

KDevelop::SourceFormatterStyle predefinedStyle(const QString& name, const QString& caption = QString())
{
    KDevelop::SourceFormatterStyle st = KDevelop::SourceFormatterStyle( name );
    st.setCaption( caption.isEmpty() ? name : caption );
    AStyleFormatter fmt;
    fmt.predefinedStyle( name );
    st.setContent( fmt.saveStyle() );
    st.setMimeTypes(supportedMimeTypes());
    return st;
}

QList<KDevelop::SourceFormatterStyle> AStylePlugin::predefinedStyles()
{
    QList<KDevelop::SourceFormatterStyle> styles;

    styles << predefinedStyle("ANSI");
    styles << predefinedStyle("GNU");
    styles << predefinedStyle("Java");
    styles << predefinedStyle("K&R", "Kernighan & Ritchie");
    styles << predefinedStyle("Linux");
    styles << predefinedStyle("Stroustrup");
    styles << predefinedStyle("Horstmann");
    styles << predefinedStyle("Whitesmith");
    styles << predefinedStyle("Banner");
    styles << predefinedStyle("1TBS");
    styles << predefinedStyle("KDELibs");
    styles << predefinedStyle("Qt");

    return styles;
}

KDevelop::SettingsWidget* AStylePlugin::editStyleWidget(const QMimeType& mime)
{
    AStylePreferences::Language lang = AStylePreferences::CPP;
    if(mime.inherits("text/x-java"))
        lang = AStylePreferences::Java;
    else if(mime.inherits("text/x-csharp"))
        lang = AStylePreferences::CSharp;
    return new AStylePreferences(lang);
}

QString AStylePlugin::previewText(const SourceFormatterStyle& style, const QMimeType& mime)
{
    return "// Indentation\n" + indentingSample() + "\t// Formatting\n"
        + formattingSample();
}

AStylePlugin::Indentation AStylePlugin::indentation( const KUrl& url )
{
    // Call formatSource first, to initialize the m_formatter data structures according to the URL
    formatSource( "", url, QMimeDatabase().mimeTypeForUrl(url), QString(), QString() );

    Indentation ret;

    ret.indentWidth = m_formatter->option("FillCount").toInt();
    
    QString s = m_formatter->option("Fill").toString();
    if(s == "Tabs")
    {
        // Do tabs-only indentation
        ret.indentationTabWidth = ret.indentWidth;
    }else{
        // Don't use tabs at all
        ret.indentationTabWidth = -1;
    }
    
    return ret;
}

QString AStylePlugin::formattingSample()
{
    return 
    "void func(){\n"
    "\tif(isFoo(a,b))\n"
    "\tbar(a,b);\n"
    "if(isFoo)\n"
    "\ta=bar((b-c)*a,*d--);\n"
    "if(  isFoo( a,b ) )\n"
    "\tbar(a, b);\n"
    "if (isFoo) {isFoo=false;cat << isFoo <<endl;}\n"
    "if(isFoo)DoBar();if (isFoo){\n"
    "\tbar();\n"
    "}\n"
    "\telse if(isBar()){\n"
    "\tannotherBar();\n"
    "}\n"
    "int var = 1;\n"
    "int *ptr = &var;\n"
    "int& ref = i;\n"
    "\n"
    "QList<int>::const_iterator it = list.begin();\n"
    "}\n"
    "namespace A {\n"
    "namespace B {\n"
    "class someClass {\n"
    "void foo() {\n"
    "  if (true) {\n"
    "    func();\n"
    "  } else {\n"
    "    // bla\n"
    "  }\n"
    "}\n"
    "};\n"
    "}\n"
    "}\n";
}

QString AStylePlugin::indentingSample()
{
    return
    "#define foobar(A)\\\n"
    "{Foo();Bar();}\n"
    "#define anotherFoo(B)\\\n"
    "return Bar()\n"
    "\n"
    "namespace Bar\n"
    "{\n"
    "class Foo\n"
    "{public:\n"
    "Foo();\n"
    "virtual ~Foo();\n"
    "};\n"
    "switch (foo)\n"
    "{\n"
    "case 1:\n"
    "a+=1;\n"
    "break;\n"
    "case 2:\n"
    "{\n"
    "a += 2;\n"
    " break;\n"
    "}\n"
    "}\n"
    "if (isFoo)\n"
    "{\n"
    "bar();\n"
    "}\n"
    "else\n"
    "{\n"
    "anotherBar();\n"
    "}\n"
    "int foo()\n"
    "\twhile(isFoo)\n"
    "\t\t{\n"
    "\t\t\t...\n"
    "\t\t\tgoto error;\n"
    "\t\t....\n"
    "\t\terror:\n"
    "\t\t\t...\n"
    "\t\t}\n"
    "\t}\n"
    "fooArray[]={ red,\n"
    "\tgreen,\n"
    "\tdarkblue};\n"
    "fooFunction(barArg1,\n"
    "\tbarArg2,\n"
    "\tbarArg3);\n"
    "struct foo{ int bar() {} };\n";
}

#include "astyle_plugin.moc"
