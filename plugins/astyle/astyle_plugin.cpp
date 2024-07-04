/*
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>
    SPDX-FileCopyrightText: 2001 Matthias Hölzer-Klüpfel <mhk@caldera.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "astyle_plugin.h"

#include <KPluginFactory>

#include "astyle_formatter.h"
#include "astyle_stringiterator.h"
#include "astyle_preferences.h"
#include <KLocalizedString>

#include <QMimeType>
#include <QUrl>

namespace {

QString formattingCppSample()
{
    return QStringLiteral(
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
        "}\n");
}

QString formattingObjCSample()
{
    return QStringLiteral(
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
        "@end\n");
}

QString indentingCppSample()
{
    return QStringLiteral(
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
        "struct foo{ int bar() {} };\n");
}

QString indentingObjCSample()
{
    return QStringLiteral(
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
        "struct foo { int bar() {} };\n");
}

QString previewText(const QString& indentingSample, const QString& formattingSample)
{
    return QLatin1String("// Indentation\n") + indentingSample + QLatin1String("\t// Formatting\n") + formattingSample;
}

} // unnamed namespace

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(AStyleFactory, "kdevastyle.json", registerPlugin<AStylePlugin>();)

AStylePlugin::AStylePlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : IPlugin(QStringLiteral("kdevastyle"), parent, metaData)
    , m_formatter(new AStyleFormatter())
{
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
    return i18n(
        "<b>Artistic Style</b> is a source code indenter, formatter,"
        " and beautifier for the C, C++, C# and Java programming languages."
        // Use <p> to prevent a line break within the link in QWhatsThis. Apparently the following excerpt
        // from QToolTip class documentation applies to QWhatsThis too: "Rich text displayed in a tool tip
        // is implicitly word-wrapped unless specified differently with <p style='white-space:pre'>."
        "<p style='margin-top:0;white-space:pre'>"
        "Home Page: <a href=\"http://astyle.sourceforge.net/\">http://astyle.sourceforge.net</a></p>");
}

QString AStylePlugin::formatSourceWithStyle(const SourceFormatterStyle& style,
                                            const QString& text,
                                            const QUrl& /*url*/,
                                            const QMimeType& mime,
                                            const QString& leftContext,
                                            const QString& rightContext) const
{
    if(mime.inherits(QStringLiteral("text/x-java")))
        m_formatter->setJavaStyle();
    else if(mime.inherits(QStringLiteral("text/x-csharp")))
        m_formatter->setSharpStyle();
    else
        m_formatter->setCStyle();

    if (style.content().isEmpty()) {
        m_formatter->predefinedStyle(style.name());
    } else {
        m_formatter->loadStyle(style.content());
    }

    return m_formatter->formatSource(text, leftContext, rightContext);
}

static SourceFormatterStyle createPredefinedStyle(const QString& name, const QString& caption = QString())
{
    SourceFormatterStyle st = SourceFormatterStyle( name );
    st.setCaption( caption.isEmpty() ? name : caption );
    AStyleFormatter fmt;
    fmt.predefinedStyle( name );
    st.setContent( fmt.saveStyle() );
    st.setMimeTypes(ISourceFormatter::mimeTypesSupportedByBuiltInStyles());
    st.setUsePreview(true);
    return st;
}

QVector<SourceFormatterStyle> AStylePlugin::predefinedStyles() const
{
    static const QVector<SourceFormatterStyle> list = {
        createPredefinedStyle(QStringLiteral("ANSI")),
        createPredefinedStyle(QStringLiteral("GNU")),
        createPredefinedStyle(QStringLiteral("Java")),
        createPredefinedStyle(QStringLiteral("K&R"), QStringLiteral("Kernighan & Ritchie")),
        createPredefinedStyle(QStringLiteral("Linux")),
        createPredefinedStyle(QStringLiteral("Stroustrup")),
        createPredefinedStyle(QStringLiteral("Horstmann")),
        createPredefinedStyle(QStringLiteral("Whitesmith")),
        createPredefinedStyle(QStringLiteral("Banner")),
        createPredefinedStyle(QStringLiteral("1TBS")),
        createPredefinedStyle(QStringLiteral("KDELibs"), QStringLiteral("KDE Frameworks")),
        createPredefinedStyle(QStringLiteral("Qt")),
    };
    return list;
}

bool AStylePlugin::hasEditStyleWidget() const
{
    return true;
}

SettingsWidgetPtr AStylePlugin::editStyleWidget(const QMimeType& mime) const
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
    return SettingsWidgetPtr{new AStylePreferences(lang)};
}

QString AStylePlugin::previewText(const SourceFormatterStyle& /*style*/, const QMimeType& mime) const
{
    if (mime.inherits(QStringLiteral("text/x-objcsrc")) || mime.inherits(QStringLiteral("text/x-objc++src"))) {
        static const QString text = ::previewText(indentingObjCSample(), formattingObjCSample());
        return text;
    } else {
        static const QString text = ::previewText(indentingCppSample(), formattingCppSample());
        return text;
    }
    // TODO: add previews for the other supported languages
}

AStylePlugin::Indentation AStylePlugin::indentation(const SourceFormatterStyle& style, const QUrl& url,
                                                    const QMimeType& mime) const
{
    // Call formatSourceWithStyle() first to initialize the m_formatter data structures according to the arguments.
    formatSourceWithStyle(style, QString(), url, mime, QString(), QString());

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
        return formattingObjCSample();
      default:
        return formattingCppSample();
   }
   Q_UNREACHABLE();
}

QString AStylePlugin::indentingSample(AStylePreferences::Language lang)
{
   switch (lang) {
      case AStylePreferences::ObjC:
        return indentingObjCSample();
      default:
        return indentingCppSample();
   }
   Q_UNREACHABLE();
}

#include "astyle_plugin.moc"
#include "moc_astyle_plugin.cpp"
