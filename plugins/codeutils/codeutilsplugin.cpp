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
#include <KActionCollection>

#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <QStandardPaths>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/context.h>
#include <interfaces/ilanguagecontroller.h>

#include <language/duchain/duchainutils.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/abstractfunctiondeclaration.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/types/functiontype.h>
#include <language/codegen/templaterenderer.h>
#include <language/codegen/codedescription.h>
#include <language/interfaces/ilanguagesupport.h>
#include <debug.h>


using namespace KDevelop;
using namespace KTextEditor;

K_PLUGIN_FACTORY_WITH_JSON(CodeUtilsPluginFactory, "kdevcodeutils.json", registerPlugin<CodeUtilsPlugin>(); )

CodeUtilsPlugin::CodeUtilsPlugin ( QObject* parent, const QVariantList& )
    : IPlugin ( QStringLiteral("kdevcodeutils"), parent )
{
    setXMLFile( QStringLiteral("kdevcodeutils.rc") );

    QAction* action = actionCollection()->addAction( QStringLiteral("document_declaration") );
    // i18n: action name; 'Document' is a verb
    action->setText( i18n( "Document Declaration" ) );
    actionCollection()->setDefaultShortcut(action, i18n( "Alt+Shift+d" ));
    connect( action, &QAction::triggered, this, &CodeUtilsPlugin::documentDeclaration );
    action->setToolTip( i18n( "Add Doxygen skeleton for declaration under cursor." ) );
    // i18n: translate title same as the action name
    action->setWhatsThis( i18n( "Adds a basic Doxygen comment skeleton in front of "
                                "the declaration under the cursor, e.g. with all the "
                                "parameter of a function." ) );
    action->setIcon( QIcon::fromTheme( QStringLiteral("documentinfo") ) );
}

void CodeUtilsPlugin::documentDeclaration()
{
    View* view =  ICore::self()->documentController()->activeTextDocumentView();
    if ( !view ) {
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

    TemplateRenderer renderer;
    renderer.setEmptyLinesPolicy(TemplateRenderer::TrimEmptyLines);
    renderer.addVariable(QStringLiteral("brief"), i18n( "..." ));

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
        renderer.addVariable(QStringLiteral("function"), QVariant::fromValue(description));
        qCDebug(PLUGIN_CODEUTILS) << "Found function" << description.name << "with" << description.arguments.size() << "arguments";
    }

    lock.unlock();

    // TODO: Choose the template based on the language
    QString templateName = QStringLiteral("doxygen_cpp");
    auto languages = core()->languageController()->languagesForUrl(view->document()->url());
    if (!languages.isEmpty())
    {
        QString languageName = languages.first()->name();
        if (languageName == QLatin1String("Php"))
        {
            templateName = QStringLiteral("phpdoc_php");
        }
        else if (languageName == QLatin1String("Python"))
        {
            templateName = QStringLiteral("rest_python");
            // Python docstrings appear inside functions and classes, not above them
            insertPos = Cursor(line+1, 0);
        }
    }

    const QString fileName = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("kdevcodeutils/templates/") + templateName + QLatin1String(".txt"));
    if (fileName.isEmpty())
    {
        qCWarning(PLUGIN_CODEUTILS) << "No suitable template found" << fileName;
        return;
    }

    const QString comment = renderer.renderFile(QUrl::fromLocalFile(fileName));
    view->insertTemplate(insertPos, comment);
}

CodeUtilsPlugin::~CodeUtilsPlugin()
{
}

#include "codeutilsplugin.moc"
