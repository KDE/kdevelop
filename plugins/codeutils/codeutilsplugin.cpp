/*
 * This file is part of KDevelop
 * Copyright 2010 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "codeutilsplugin.h"

#include <QAction>
#include <QVariantList>

#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KAboutData>
#include <KActionCollection>

#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KTextEditor/TemplateInterface>
#include <QApplication>
#include <QStandardPaths>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/context.h>
#include <interfaces/ilanguage.h>
#include <interfaces/ilanguagecontroller.h>

#include <language/duchain/duchainutils.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/abstractfunctiondeclaration.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/types/functiontype.h>
#include <language/codegen/templaterenderer.h>
#include <language/codegen/codedescription.h>
#include <language/codegen/sourcefiletemplate.h>
#include <language/codegen/documentchangeset.h>
#include <project/projectmodel.h>

using namespace KDevelop;
using namespace KTextEditor;

K_PLUGIN_FACTORY_WITH_JSON(CodeUtilsPluginFactory, registerPlugin<CodeUtilsPlugin>(); )
// K_EXPORT_PLUGIN(CodeUtilsPluginFactory(KAboutData("kdevcodeutils","kdevcodeutils", ki18n("Code Utilities"), "0.1", ki18n("Collection of various utilities that increase productivity while programming."), KAboutData::License_GPL)
//     .addAuthor(ki18n("Milian Wolff"), ki18n("Author"), "mail@milianw.de", "http://milianw.de")
// ))

int debugArea() { static int s_area = KDebug::registerArea("kdevcodeutils"); return s_area; }

#define debug() kDebug(debugArea())

CodeUtilsPlugin::CodeUtilsPlugin ( QObject* parent, const QVariantList& )
    : IPlugin ( "kdevcodeutils", parent )
{
    setXMLFile( "kdevcodeutils.rc" );

    QAction* action = actionCollection()->addAction( "document_declaration" );
    // i18n: action name; 'Document' is a verb
    action->setText( i18n( "Document Declaration" ) );
    action->setShortcut( i18n( "Alt+Shift+d" ) );
    connect( action, SIGNAL(triggered(bool)), this, SLOT(documentDeclaration()) );
    action->setToolTip( i18n( "Add Doxygen skeleton for declaration under cursor." ) );
    // i18n: translate title same as the action name
    action->setWhatsThis( i18n( "Adds a basic Doxygen comment skeleton in front of "
                                "the declaration under the cursor, e.g. with all the "
                                "parameter of a function." ) );
    action->setIcon( QIcon::fromTheme( "documentinfo" ) );
}

void CodeUtilsPlugin::documentDeclaration()
{
    View* view =  ICore::self()->documentController()->activeTextDocumentView();
    if ( !view ) {
        return;
    }
    TemplateInterface* tplIface = qobject_cast< TemplateInterface* >( view );
    if ( !tplIface ) {
        return;
    }

    DUChainReadLocker lock;
    TopDUContext* topCtx = DUChainUtils::standardContextForUrl(view->document()->url());
    if ( !topCtx ) {
        return;
    }
    Declaration* dec = DUChainUtils::declarationInLine( KTextEditor::Cursor( view->cursorPosition() ),
                                                        topCtx );
    if ( !dec || dec->isForwardDeclaration() ) {
        return;
    }
    // finally - we found the declaration :)
    int line = dec->range().start.line;
    Cursor insertPos( line, 0 );

#if 0 //FIXME: KF5 porting, template does weird things
    TemplateRenderer renderer;
    renderer.addVariable("brief", i18n( "..." ));

    /*
    QString indentation = textDoc->line( insertPos.line() );
    if (!indentation.isEmpty()) {
        int lastSpace = 0;
        while (indentation.at(lastSpace).isSpace()) {
            ++lastSpace;
        }
        indentation.truncate(lastSpace);
    }
    */

    if (dec->isFunctionDeclaration())
    {
        FunctionDescription description = FunctionDescription(DeclarationPointer(dec));
        renderer.addVariable("function", QVariant::fromValue(description));
        kDebug() << "Found function" << description.name << "with" << description.arguments.size() << "arguments";
    }

    lock.unlock();

    // TODO: Choose the template based on the language
    QString templateName = "doxygen_cpp";
    QList<ILanguage*> languages = core()->languageController()->languagesForUrl(doc->url());
    if (!languages.isEmpty())
    {
        QString languageName = languages.first()->name();
        if (languageName == "Php")
        {
            templateName = "phpdoc_php";
        }
        else if (languageName == "Python")
        {
            templateName = "rest_python";
            // Python docstrings appear inside functions and classes, not above them
            insertPos = Cursor(line+1, 0);
        }
    }

    QString fileName = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kdevcodeutils/templates/" + templateName + ".txt");
    if (fileName.isEmpty())
    {
        kWarning() << "No suitable template found" << fileName;
        return;
    }

    const QString comment = renderer.renderFile(QUrl::fromLocalFile(fileName));
    tplIface->insertTemplateText(insertPos, comment, QMap<QString, QString>());
#endif
}

CodeUtilsPlugin::~CodeUtilsPlugin()
{
}

#include "codeutilsplugin.moc"
