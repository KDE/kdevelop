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

#include <KPluginFactory>

#include <interfaces/icore.h>
#include <interfaces/isourceformattercontroller.h>

#include "astyle_formatter.h"
#include "astyle_stringiterator.h"
#include "astyle_preferences.h"
#include <KLocalizedString>
#include <QUrl>

static const char formattingCxxSample[] =
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

static const char formattingObjCSample[] =
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
    "\n"
    "}\n"
    "@implementation someClass \n"
    "+ (someClass*) someClassWithFoo:(int)foo\n"
    "{\n"
    "  someClass *this;\n"
    "  if (foo) {\n"
    "    this = [[someClass alloc] initWith:foo];\n"
    "  } else {\n"
    "    // bla\n"
    "  }\n"
    "  return self;\n"
    "}\n"
    "@end\n";

static const char indentingCxxSample[] =
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
    "void bar(int foo)\n"
    "{\n"
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
    "}\n"
    "int foo()\n"
    "\twhile(isFoo)\n"
    "\t\t{\n"
    "\t\t\t// ...\n"
    "\t\t\tgoto error;\n"
    "\t\t/* .... */\n"
    "\t\terror:\n"
    "\t\t\t//...\n"
    "\t\t}\n"
    "\t}\n"
    "fooArray[]={ red,\n"
    "\tgreen,\n"
    "\tdarkblue};\n"
    "fooFunction(barArg1,\n"
    "\tbarArg2,\n"
    "\tbarArg3);\n"
    "struct foo{ int bar() {} };\n";

static const char indentingObjCSample[] =
    "#import <objc/Object.h>\n"
    "\n"
    "#define foobar(A)\\\n"
    "\t{Foo();Bar();}\n"
    "#define anotherFoo(B)\\\n"
    "\treturn Bar()\n"
    "\n"
    "@interface Foo : Bar {\n"
    "@private\n"
    "\tid Baz;\n"
    "}\n"
    "- (void) init;\n"
    "- (NSString*) description;\n"
    "@property (retain) id Baz;\n"
    "@end\n"
    "\n"
    "@interface Foo (Bar)\n"
    "- (void)bar:(int) foo;\n"
    "@end\n"
    "\n"
    "@implementation Foo (Bar)\n"
    "\n"
    "- (void) bar:(int) foo\n"
    "{\n"
    "\tswitch (foo) {\n"
    "case 1:\n"
    "a += 1;\n"
    "break;\n"
    "case 2: {\n"
    "a += 2;\n"
    "break;\n"
    "}\n"
    "}\n"
    "if (isFoo) {\n"
    "bar();\n"
    "} else {\n"
    "[anotherBar withFoo:self];\n"
    "}\n"
    "}\n"
    "\n"
    "@end\n"
    "int foo()\n"
    "while (isFoo)\n"
    "{\n"
    "\t// ...\n"
    "\tgoto error;\n"
    "\t/* .... */\n"
    "error:\n"
    "\t//...\n"
    "}\n"
    "\n"
    "fooArray[] = { red,\n"
    "\tgreen,\n"
    "\tdarkblue};\n"
    "fooFunction(barArg1,\n"
    "\tbarArg2,\n"
    "\tbarArg3);\n"
    "struct foo { int bar() {} };\n";

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(AStyleFactory, "kdevastyle.json", registerPlugin<AStylePlugin>();)

AStylePlugin::AStylePlugin(QObject *parent, const QVariantList&)
    : IPlugin(QStringLiteral("kdevastyle"), parent)
    , m_formatter(new AStyleFormatter())
{
    currentStyle = predefinedStyles().at(0);
}

AStylePlugin::~AStylePlugin()
{
}

QString AStylePlugin::name() const
{
    // This needs to match the X-KDE-PluginInfo-Name entry from the .desktop file!
    return QStringLiteral("kdevastyle");
}

QString AStylePlugin::caption() const
{
    return QStringLiteral("Artistic Style");
}

QString AStylePlugin::description() const
{
    return i18n("<b>Artistic Style</b> is a source code indenter, formatter,"
        " and beautifier for the C, C++, C# and Java programming languages.<br />"
        "Home Page: <a href=\"http://astyle.sourceforge.net/\">http://astyle.sourceforge.net</a>");
}

QString AStylePlugin::formatSourceWithStyle( SourceFormatterStyle s, const QString& text, const QUrl& /*url*/, const QMimeType& mime, const QString& leftContext, const QString& rightContext ) const
{
    if(mime.inherits(QStringLiteral("text/x-java")))
        m_formatter->setJavaStyle();
    else if(mime.inherits(QStringLiteral("text/x-csharp")))
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

QString AStylePlugin::formatSource(const QString& text, const QUrl &url, const QMimeType& mime, const QString& leftContext, const QString& rightContext) const
{
    auto style = ICore::self()->sourceFormatterController()->styleForUrl(url, mime);
    return formatSourceWithStyle(style, text, url, mime, leftContext, rightContext);
}

static SourceFormatterStyle::MimeList supportedMimeTypes()
{
    return {
        {QStringLiteral("text/x-c++src"), QStringLiteral("C++")},
        {QStringLiteral("text/x-csrc"), QStringLiteral("C")},
        {QStringLiteral("text/x-chdr"), QStringLiteral("C")},
        {QStringLiteral("text/x-c++hdr"), QStringLiteral("C++")},
        {QStringLiteral("text/x-java"), QStringLiteral("Java")},
        {QStringLiteral("text/x-csharp"), QStringLiteral("C#")},
        {QStringLiteral("text/x-objcsrc"), QStringLiteral("Objective-C")},
        {QStringLiteral("text/x-objc++src"), QStringLiteral("Objective-C++")},
        {QStringLiteral("text/x-objchdr"), QStringLiteral("Objective-C")},
    };
}

SourceFormatterStyle predefinedStyle(const QString& name, const QString& caption = QString())
{
    SourceFormatterStyle st = SourceFormatterStyle( name );
    st.setCaption( caption.isEmpty() ? name : caption );
    AStyleFormatter fmt;
    fmt.predefinedStyle( name );
    st.setContent( fmt.saveStyle() );
    st.setMimeTypes(supportedMimeTypes());
    st.setUsePreview(true);
    return st;
}

QVector<SourceFormatterStyle> AStylePlugin::predefinedStyles() const
{
    return {
        predefinedStyle(QStringLiteral("ANSI")),
        predefinedStyle(QStringLiteral("GNU")),
        predefinedStyle(QStringLiteral("Java")),
        predefinedStyle(QStringLiteral("K&R"), QStringLiteral("Kernighan & Ritchie")),
        predefinedStyle(QStringLiteral("Linux")),
        predefinedStyle(QStringLiteral("Stroustrup")),
        predefinedStyle(QStringLiteral("Horstmann")),
        predefinedStyle(QStringLiteral("Whitesmith")),
        predefinedStyle(QStringLiteral("Banner")),
        predefinedStyle(QStringLiteral("1TBS")),
        predefinedStyle(QStringLiteral("KDELibs")),
        predefinedStyle(QStringLiteral("Qt")),
    };
}

SettingsWidget* AStylePlugin::editStyleWidget(const QMimeType& mime) const
{
    AStylePreferences::Language lang = AStylePreferences::CPP;
    if (mime.inherits(QStringLiteral("text/x-java")))
        lang = AStylePreferences::Java;
    else if (mime.inherits(QStringLiteral("text/x-csharp")))
        lang = AStylePreferences::CSharp;
    else if (mime.inherits(QStringLiteral("text/x-objcsrc")) || mime.inherits(QStringLiteral("text/x-objc++src"))) {
        // x-objc++src *should* inherit x-objcsrc but that is not always the case in practice
        lang = AStylePreferences::ObjC;
    }
    return new AStylePreferences(lang);
}

QString AStylePlugin::previewText(const SourceFormatterStyle& /*style*/, const QMimeType& mime) const
{
    AStylePreferences::Language lang;
    if (mime.inherits(QStringLiteral("text/x-objcsrc")) || mime.inherits(QStringLiteral("text/x-objc++src"))) {
       lang = AStylePreferences::ObjC;
    } else {
       lang = AStylePreferences::CPP;
    }
    // TODO: add previews for the other supported languages
    return
      QLatin1String("// Indentation\n") +
      indentingSample(lang) +
      QLatin1String("\t// Formatting\n") +
      formattingSample(lang);
}

AStylePlugin::Indentation AStylePlugin::indentation(const QUrl& url) const
{
    // Call formatSource first, to initialize the m_formatter data structures according to the URL
    formatSource(QString(), url, QMimeDatabase().mimeTypeForUrl(url), QString(), QString());

    Indentation ret;

    ret.indentWidth = m_formatter->option(QStringLiteral("FillCount")).toInt();
    
    QString s = m_formatter->option(QStringLiteral("Fill")).toString();
    if(s == QLatin1String("Tabs"))
    {
        // Do tabs-only indentation
        ret.indentationTabWidth = ret.indentWidth;
    }else{
        // Don't use tabs at all
        ret.indentationTabWidth = -1;
    }
    
    return ret;
}

QString AStylePlugin::formattingSample(AStylePreferences::Language lang)
{
   switch (lang) {
      case AStylePreferences::ObjC:
         return QLatin1String(formattingObjCSample);
      default:
         return QLatin1String(formattingCxxSample);
   }
   Q_UNREACHABLE();
}

QString AStylePlugin::indentingSample(AStylePreferences::Language lang)
{
   switch (lang) {
      case AStylePreferences::ObjC:
         return QLatin1String(indentingObjCSample);
      default:
         return QLatin1String(indentingCxxSample);
   }
   Q_UNREACHABLE();
}

#include "astyle_plugin.moc"
