/* This file is part of KDevelop
   Copyright (C) 2008 Cédric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2011 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include <KPluginLoader>
#include <KPluginFactory>
#include <KAboutData>
#include <QTextStream>
#include <QTemporaryFile>
#include <KDebug>
#include <KProcess>
#include <interfaces/icore.h>
#include <interfaces/isourceformattercontroller.h>
#include <memory>
#include <QDir>
#include <util/formattinghelpers.h>

using namespace KDevelop;

static QWeakPointer<IndentPlugin> indentPluginSingleton;

K_PLUGIN_FACTORY(IndentFactory, registerPlugin<IndentPlugin>();)
K_EXPORT_PLUGIN(IndentFactory(KAboutData("kdevindent","kdevformatters", ki18n("Custom Script Formatter"), "0.2", ki18n("A formatter using custom scripts"), KAboutData::License_GPL)))

IndentPlugin::IndentPlugin(QObject *parent, const QVariantList&)
		: IPlugin(IndentFactory::componentData(), parent)
{
	KDEV_USE_EXTENSION_INTERFACE(ISourceFormatter)
        m_currentStyle = predefinedStyles().at(0);
	indentPluginSingleton = this;
}

IndentPlugin::~IndentPlugin()
{
}

QString IndentPlugin::name()
{
	// This needs to match the X-KDE-PluginInfo-Name entry from the .desktop file!
	return "kdevindent";
}

QString IndentPlugin::caption()
{
	return "Custom Script Formatter";
}

QString IndentPlugin::description()
{
	return i18n("<b>Indent and Format Source Code.</b><br />"
				"This plugin allows using powerful external formatting tools "
				"that can be invoked through the command-line.<br />"
				"For example, the <b>uncrustify</b>, <b>astyle</b> or <b>indent</b> "
				"formatters can be used.<br />"
				"The advantage of command-line formatters is that formatting configurations "
				"can be easily shared by all team members, independent of their preferred IDE.");
}

QString IndentPlugin::highlightModeForMime(const KMimeType::Ptr &mime)
{
	Q_UNUSED(mime);
	return "C++";
}

QString IndentPlugin::formatSourceWithStyle(SourceFormatterStyle style, const QString& text, const KUrl& url, const KMimeType::Ptr& /*mime*/, const QString& leftContext, const QString& rightContext)
{
	KProcess proc;
	QTextStream ios(&proc);
	
	std::auto_ptr<QTemporaryFile> tmpFile;

	if (style.content().isEmpty())
	{
		style = predefinedStyle(style.name());
		if (style.content().isEmpty())
		{
			kWarning() << "Empty contents for style" << style.name() << "for indent plugin";
			return text;
		}
	}
	
	QString useText = text;
	useText = leftContext + useText + rightContext;
	
	QString command = style.content();
	if(command.contains("$TMPFILE"))
	{
		tmpFile = std::auto_ptr<QTemporaryFile>(new QTemporaryFile(QDir::tempPath() + "/code"));
		tmpFile->setAutoRemove(false);
		if(tmpFile->open())
		{
			kDebug() << "using temporary file" << tmpFile->fileName();
			command.replace("$TMPFILE", tmpFile->fileName());
			command.replace("$FILE", url.toLocalFile());
			QByteArray useTextArray = useText.toLocal8Bit();
			if( tmpFile->write(useTextArray) != useTextArray.size() )
			{
				kWarning() << "failed to write text to temporary file";
				return text;
			}
			
		}else{
			kWarning() << "Failed to create a temporary file";
			return text;
		}
		tmpFile->close();
	}
	
	kDebug() << "using shell command for indentation: " << command;
	proc.setShellCommand(command);
	proc.setOutputChannelMode(KProcess::OnlyStdoutChannel);
	
	proc.start();
	if(!proc.waitForStarted()) {
		kDebug() << "Unable to start indent" << endl;
		return text;
	}
	
	if(!tmpFile.get())
		proc.write(useText.toLocal8Bit());
	
	proc.closeWriteChannel();
	if(!proc.waitForFinished()) {
		kDebug() << "Process doesn't finish" << endl;
		return text;
	}
	
	QString output;
	
	if(tmpFile.get())
	{
		QFile f(tmpFile->fileName());
		if( f.open(QIODevice::ReadOnly) )
		{
			output = QString::fromLocal8Bit(f.readAll());
		}else{
			kWarning() << "Failed opening the temporary file for reading";
			return text;
		}
	}else{
		output = ios.readAll();
	}
	if (output.isEmpty())
	{
		kWarning() << "indent returned empty text for style" << style.name() << style.content();
		return text;
	}

    return KDevelop::extractFormattedTextFromContext(output, useText, text, leftContext, rightContext);
}

QString IndentPlugin::formatSource(const QString& text, const KUrl& url, const KMimeType::Ptr& mime, const QString& leftContext, const QString& rightContext)
{
	return formatSourceWithStyle( KDevelop::ICore::self()->sourceFormatterController()->styleForMimeType( mime ), text, url, mime, leftContext, rightContext );
}

KDevelop::SourceFormatterStyle IndentPlugin::predefinedStyle(const QString& name)
{
	SourceFormatterStyle result(name);
	if (name == "GNU_indent_GNU")
	{
		result.setCaption(i18n("Gnu Indent: GNU"));
		result.setContent("indent");
	} else if (name == "GNU_indent_KR") {
		result.setCaption(i18n("Gnu Indent: Kernighan & Ritchie"));
		result.setContent("indent -kr");
	} else if (name == "GNU_indent_orig") {
		result.setCaption(i18n("Gnu Indent: Original Berkeley indent style"));
		result.setContent("indent -orig");
	} else if(name == "kdev_format_source.sh") {
		result.setCaption("KDevelop: kdev_format_source.sh");
		result.setContent("kdev_format_source.sh $FILE $TMPFILE");
	}
	return result;
}

QList<KDevelop::SourceFormatterStyle> IndentPlugin::predefinedStyles()
{
    QList<KDevelop::SourceFormatterStyle> styles;
	styles << predefinedStyle("kdev_format_source.sh");
	styles << predefinedStyle("GNU_indent_GNU");
	styles << predefinedStyle("GNU_indent_KR");
	styles << predefinedStyle("GNU_indent_orig");
	return styles;
}

KDevelop::SettingsWidget* IndentPlugin::editStyleWidget(const KMimeType::Ptr &mime)
{
	Q_UNUSED(mime);
	return new IndentPreferences();
}

static QString formattingSample()
{
    return 
    "// Formatting\n"
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
    "int &ref = i;\n"
    "\n"
    "QList<int>::const_iterator it = list.begin();\n"
    "}\n"
    "namespace A {\n"
    "namespace B {\n"
    "void foo() {\n"
    "  if (true) {\n"
    "    func();\n"
    "  } else {\n"
    "    // bla\n"
    "  }\n"
    "}\n"
    "}\n"
    "}\n";
}

static QString indentingSample()
{
    return
    "// Indentation\n"
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
    "\tbarArg3);\n";
}

QString IndentPlugin::previewText(const KMimeType::Ptr &)
{
	return formattingSample() + "\n\n" + indentingSample();
}

ISourceFormatter::IndentationType IndentPlugin::indentationType()
{
	///@todo Format a sample, and extract the indentation type
	return ISourceFormatter::IndentWithSpaces;
}

int IndentPlugin::indentationLength()
{
	///@todo Format a sample, and extract the indentation length
	return 4;
}

void IndentPreferences::updateTimeout()
{
    QString formatted = indentPluginSingleton.data()->formatSourceWithStyle ( m_style, indentPluginSingleton.data()->previewText ( KMimeType::Ptr() ), KUrl(), KMimeType::Ptr() );
    emit previewTextChanged ( formatted );
}

IndentPreferences::IndentPreferences()
{
    m_updateTimer = new QTimer ( this );
    m_updateTimer->setSingleShot ( true );
    connect ( m_updateTimer, SIGNAL (timeout()), SLOT (updateTimeout()) );
    m_vLayout = new QVBoxLayout ( this );
    m_captionLabel = new QLabel;
    m_vLayout->addWidget ( m_captionLabel );
    m_vLayout->addSpacing ( 10 );
    m_hLayout = new QHBoxLayout;
    m_vLayout->addLayout ( m_hLayout );
    m_commandLabel = new QLabel;
    m_hLayout->addWidget ( m_commandLabel );
    m_commandEdit = new QLineEdit;
    m_hLayout->addWidget ( m_commandEdit );
    m_commandLabel->setText ( i18n("Command: ") );
    m_vLayout->addSpacing ( 10 );
    m_bottomLabel = new QLabel;
    m_vLayout->addWidget ( m_bottomLabel );
    m_bottomLabel->setTextFormat ( Qt::RichText );
    m_bottomLabel->setText (
        i18n ( "<i>You can enter an arbitrary shell command.</i><br />"
               "Normally, the source-code to format will be reached <br />"
               "to the command through the standard-input, and the <br />"
               "result will be read from its standard-output.<br /><br />"
               "If you add <b>$TMPFILE</b> into the command, then <br />"
               "the code will be written into a temporary file, the temporary <br />"
               "file will be substituted into that position, and the result <br />"
               "will be read out of that file instead." ) );
    connect ( m_commandEdit, SIGNAL (textEdited(QString)), SLOT (textEdited(QString)) );
}

void IndentPreferences::load ( const KDevelop::SourceFormatterStyle& style )
{
    m_style = style;
    m_commandEdit->setText ( style.content() );
    m_captionLabel->setText ( i18n ( "Style: %1", style.caption() ) );

    updateTimeout();
}

QString IndentPreferences::save()
{
    return m_commandEdit->text();
}

#include "indent_plugin.moc"

// kate: indent-mode cstyle; space-indent off; tab-width 4;
