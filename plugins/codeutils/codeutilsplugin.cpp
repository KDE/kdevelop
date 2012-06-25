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

#include <QVariantList>

#include <KLocale>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KAboutData>
#include <KAction>
#include <KActionCollection>

#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KTextEditor/TemplateInterface>
#include <KStandardDirs>
#include <QApplication>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/context.h>
#include <interfaces/ilanguage.h>
#include <interfaces/ilanguagecontroller.h>

#include <language/duchain/duchainutils.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/abstractfunctiondeclaration.h>
#include <language/duchain/types/functiontype.h>
#include <language/codegen/templateclassassistant.h>
#include <language/codegen/templaterenderer.h>
#include <language/codegen/codedescription.h>
#include <project/projectmodel.h>

using namespace KDevelop;
using namespace KTextEditor;

K_PLUGIN_FACTORY(CodeUtilsPluginFactory, registerPlugin<CodeUtilsPlugin>(); )
K_EXPORT_PLUGIN(CodeUtilsPluginFactory(KAboutData("kdevcodeutils","kdevcodeutils", ki18n("Code Utilities"), "0.1", ki18n("Various utilities that help you code faster."), KAboutData::License_GPL)
    .addAuthor(ki18n("Milian Wolff"), ki18n("Author"), "mail@milianw.de", "http://milianw.de")
))

int debugArea() { static int s_area = KDebug::registerArea("kdevcodeutils"); return s_area; }

#define debug() kDebug(debugArea())

CodeUtilsPlugin::CodeUtilsPlugin ( QObject* parent, const QVariantList& )
    : IPlugin ( CodeUtilsPluginFactory::componentData(), parent )
{
    setXMLFile( "kdevcodeutils.rc" );

    KAction *action = actionCollection()->addAction( "document_declaration" );
    // i18n: action name; 'Document' is a verb
    action->setText( i18n( "Document Declaration" ) );
    action->setShortcut( i18n( "Alt+Shift+d" ) );
    connect( action, SIGNAL(triggered(bool)), this, SLOT(documentDeclaration()) );
    action->setToolTip( i18n( "Add Doxygen skeleton for declaration under cursor." ) );
    // i18n: translate title same as the action name
    action->setWhatsThis( i18n( "<b>Document Declaration</b><p>"
                                "Adds a basic Doxygen comment skeleton in front of "
                                "the declaration under the cursor, e.g. with all the "
                                "parameter of a function."
                                "</p>" ) );
    action->setIcon( KIcon( "documentinfo" ) );

    action = actionCollection()->addAction( "class_from_template" );
    action->setText( i18n( "Create Class from &Template" ) );
    action->setShortcut( i18n( "Alt+Shift+t" ) );
    connect( action, SIGNAL(triggered(bool)), this, SLOT(createClass()));
    action->setIcon( KIcon( "code-class" ) );
    action->setToolTip( i18n( "Create a new class from a template" ) );
}

void CodeUtilsPlugin::documentDeclaration()
{
    IDocument* doc = ICore::self()->documentController()->activeDocument();
    if ( !doc ) {
        return;
    }
    Document* textDoc = doc->textDocument();
    if ( !textDoc ) {
        return;
    }
    View* view =  textDoc->activeView();
    if ( !view ) {
        return;
    }
    TemplateInterface* tplIface = qobject_cast< TemplateInterface* >( view );
    if ( !tplIface ) {
        return;
    }

    DUChainReadLocker lock;
    TopDUContext* topCtx = DUChainUtils::standardContextForUrl(doc->url());
    if ( !topCtx ) {
        return;
    }
    Declaration* dec = DUChainUtils::declarationInLine( SimpleCursor( view->cursorPosition() ),
                                                        topCtx );
    if ( !dec || dec->isForwardDeclaration() ) {
        return;
    }
    // finally - we found the declaration :)
    const Cursor insertPos( dec->range().start.line, 0 );

    QList<ILanguage*> languages = core()->languageController()->languagesForUrl(doc->url());
    if (!languages.isEmpty())
    {
        languages.first()->languageSupport();
    }

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
    QString fileName = core()->componentData().dirs()->findResource("data", "kdevcodeutils/templates/doxygen_cpp.txt");
    Q_ASSERT(!fileName.isEmpty());
    kDebug() << "Rendering template from file" << fileName;

    const QString comment = renderer.renderFile(KUrl(fileName));
    kDebug() << comment;
    tplIface->insertTemplateText(insertPos, comment, QMap<QString, QString>());
}

void CodeUtilsPlugin::createClass()
{
    KUrl url;
    if (QAction* action = qobject_cast<QAction*>(sender()))
    {
        url = action->data().value<KUrl>();
    }
    TemplateClassAssistant assistant(QApplication::activeWindow(), url);
    assistant.exec();
}

CodeUtilsPlugin::~CodeUtilsPlugin()
{
}

ContextMenuExtension CodeUtilsPlugin::contextMenuExtension (Context* context)
{
    ContextMenuExtension ext;

    if (context->hasType(Context::ProjectItemContext))
    {
        if (ProjectItemContext* projectContext = dynamic_cast<ProjectItemContext*>(context))
        {
            if (projectContext->items().size() == 1)
            {
                KAction* action = new KAction(KIcon("code-class"), i18n("Create Class from &Template"), actionCollection() );
                action->setData(projectContext->items().first()->url());

                connect( action, SIGNAL(triggered(bool)), this, SLOT(createClass()));
                ext.addAction(ContextMenuExtension::FileGroup, action);
            }
        }
    }
    return ext;
}
