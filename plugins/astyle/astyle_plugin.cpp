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

#include <QTextStream>
#include <QMenu>
#include <QLineEdit>
#include <QRegExp>
#include <QStringList>
//#include <QProgressBar>

//#include <kdeversion.h>
#include <KDebug>
#include <KDialog>
//#include <KDevgenericfactory.h>
#include <KIconLoader>
#include <KLocale>
#include <kparts/part.h>
#include <kparts/partmanager.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <KApplication>
#include <KConfig>
#include <KFileDialog>
#include <KMessageBox>
#include <KGlobal>
#include <KAction>
#include <KActionCollection>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/idocument.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include "astyle_formatter.h"
#include "astyle_stringiterator.h"
#include "astyle_preferences.h"

using namespace KDevelop;

K_PLUGIN_FACTORY(AStyleFactory, registerPlugin<AStylePlugin>(); )
K_EXPORT_PLUGIN(AStyleFactory("kdevastyle"))

AStylePlugin::AStylePlugin( QObject *parent, const QVariantList& )
: IPlugin( AStyleFactory::componentData(), parent)
{
    setXMLFile("kdevastyle.rc");

    m_formatTextAction = actionCollection()->addAction("edit_astyle_reformat");
    m_formatTextAction->setText(i18n("&Reformat Source"));
    m_formatTextAction->setToolTip(i18n("Reformat source using AStyle"));
    m_formatTextAction->setWhatsThis(i18n("<b>Reformat source</b><p>Source reformatting "
        "functionality using <b>astyle</b> library. "
    /*    "Also available in <b>New Class</b> and <b>Subclassing</b> wizards."*/));
    connect(m_formatTextAction, SIGNAL(triggered()), this, SLOT(beautifySource()));

//     m_formatFileAction = actionCollection()->addAction("tools_astyle");
//     m_formatFileAction->setText(i18n("Formateee files"));
//     m_formatFileAction->setToolTip(i18n("Format files"));
//     m_formatFileAction->setWhatsThis(i18n("<b>Fomat files</b><p>Formatting functionality using <b>astyle</b> library. "
//         /*"Also available in <b>New Class</b> and <b>Subclassing</b> wizards."*/));
//     connect(m_formatFileAction, SIGNAL(triggered()), this, SLOT(formatFilesSelect()));

    m_formatTextAction->setEnabled(false);
//     m_formatFileAction->setEnabled(true);

//     m_defaultFormatExtensions = "*.cpp *.h *.hpp,*.c *.h,*.cxx *.hxx,*.c++ *.h++,*.cc *.hh,*.C *.H,*.diff ,*.inl,*.java,*.moc,*.patch,*.tlh,*.xpm";
//     m_searchExtensions.insert("*", "*");
    m_formatter = new AStyleFormatter();

    connect(core()->partManager(), SIGNAL(activePartChanged(KParts::Part*)),
             this, SLOT(activePartChanged(KParts::Part*)));
}

AStylePlugin::~AStylePlugin()
{
}

void AStylePlugin::beautifySource()
{
    KTextEditor::Document *doc =
        core()->documentController()->activeDocument()->textDocument();
    if(!doc)
        return;

    bool has_selection = false;

    KTextEditor::View *view = doc->views().first();
    if(view && view->selection())
        has_selection = true;

    // put the selection back to the same indent level.
    // taking note of the config options.
    if(has_selection) {
        unsigned int indentCount=0;
        QString indentWith("");
        QString original = view->selectionText();
        
        for (; indentCount < original.length(); indentCount++){
            QChar ch = original[indentCount];
            if(!ch.isSpace())
                break;
                
            if(ch == QChar('\n') || ch == QChar('\r'))
                indentWith="";
            else
                indentWith+=original[indentCount];   
        }
        replaceSpacesWithTab(indentWith);

        QString output = m_formatter->formatSource(view->selectionText(), indentWith);
        
        //remove the final newline character, unless it should be there
        if(!original.endsWith('\n'))
            output.resize(output.length()-1);
        //there was a selection, so only change the part of the text related to it
        view->removeSelectionText();
        doc->insertText(view->cursorPosition(), output);
    } else {
        KTextEditor::Cursor cursor = view->cursorPosition();
        doc->setText(m_formatter->formatSource(doc->text()));
        view->setCursorPosition(cursor);
    }
}

void AStylePlugin::replaceSpacesWithTab(QString &input)
{
    int wsCount = m_formatter->option("FillCount").toInt();
    if(m_formatter->option("Fill").toString() == "Tabs") {
        // tabs and wsCount spaces to be a tab
        QString replace;
        for (int i =0; i < wsCount;i++)
            replace+=' ';

        input = input.replace(replace, QChar('\t'));
        input = input.remove(' ');
    } else {
        if(m_formatter->option("FillForce").toBool()){
            //convert tabs to spaces
            QString replace;
            for (int i =0;i < wsCount;i++)
                replace+=' ';

            input = input.replace(QChar('\t'), replace);
        }
    }
}

void AStylePlugin::activePartChanged ( KParts::Part *part )
{
    bool enabled = false;
    KParts::ReadWritePart *rw_part = dynamic_cast<KParts::ReadWritePart*> ( part );

    if(rw_part) {
        KTextEditor::Document *doc = dynamic_cast<KTextEditor::Document*>(rw_part);

        if(doc) {
            // check for the everything case..
//             if(!m_formatter->hasExtension("*")) {
//                 QString extension = rw_part->url().path();
//                 int pos = extension.lastIndexOf( '.' );
//                 if(pos >= 0) {
//                     extension = extension.mid(pos);
//                     enabled = m_formatter->hasExtension(extension);
//                 }
//             } else {
//                 enabled = true;
//             }
            enabled = true;
        }
    }

    m_formatTextAction->setEnabled(enabled);
}

KDevelop::ContextMenuExtension 
AStylePlugin::contextMenuExtension(KDevelop::Context* context)
{
//     QList<QAction*> actionlist;
//     if(context->hasType(Context::EditorContext))
//         actionlist << m_formatTextAction;
//     else if(context->hasType(Context::FileContext))
//         actionlist << m_formatTextAction;
//     return qMakePair(i18n("Source formatter"), actionlist);
    ContextMenuExtension ext;
    ext.addAction(ContextMenuExtension::EditGroup, m_formatTextAction);
    return ext;
}

/*void AStylePlugin::contextMenu(QMenu *popup, const Context *context)
{
    if(context->hasType( Context::EditorContext )) {
        popup->addSeparator();
        popup->addAction(m_formatTextAction);
    }
    else if ( context->hasType( Context::FileContext )){
        const FileContext *ctx = static_cast<const FileContext*>(context);
        m_urls = ctx->urls();

        popup->addSeparator();
        popup->addAction(m_formatFileAction);
    }
}*/

// void AStylePlugin::formatFilesSelect()
// {
//     KUrl::List list = KFileDialog::getOpenUrls(KUrl(),
//         /*getProjectExtensions()*/"*", 0, "Select files to format" );
// 
//     formatFiles(list);
// }

// void AStylePlugin::formatFiles(KUrl::List &list)
// {
//     KUrl::List::iterator it = list.begin();
//     while(it != list.end()) {
//         kDebug ( 9009 ) << "Formatting " << (*it).pathOrUrl() << endl;
//         ++it;
//     }
// 
//     uint processed = 0;
//     for(uint fileCount = 0; fileCount < m_urls.size(); fileCount++) {
//         QString fileName = m_urls[fileCount].pathOrUrl();
// 
//         bool found = false;
//         for ( QMap<QString, QString>::Iterator it = m_searchExtensions.begin();
//             it != m_searchExtensions.end(); ++it ) {
//             QRegExp re(it.value(), Qt::CaseSensitive, QRegExp::Wildcard);
//             if( (re.indexIn(fileName) == 0)
//                 && ((uint) re.matchedLength() == fileName.length()) ) {
//                 found = true;
//                 break;
//             }
//         }
// 
//         if ( found )
//         {
//             QString backup = fileName + "#";
//             QFile fin ( fileName );
//             QFile fout ( backup );
//             if ( fin.open ( QFile::ReadOnly ) ) {
//                 if ( fout.open ( QFile::WriteOnly ) ) {
//                     QString fileContents ( fin.readAll() );
//                     fin.close();
//                     QTextStream outstream ( &fout );
//                     outstream << formatSource ( fileContents );
//                     fout.close();
//                     QDir().rename ( backup, fileName );
//                     processed++;
//                 } else {
//                     KMessageBox::sorry ( 0, i18n ( "Not able to write %1" ).arg ( backup ) );
//                 }
//             } else {
//                 KMessageBox::sorry(0, i18n ( "Not able to read %1" ).arg ( fileName ) );
//             }
//         }
//     }
// 
//     if ( processed != 0 ) {
//         KMessageBox::information ( 0, i18n ( "Processed %1 files ending with extensions %2" ).arg ( processed ).arg(m_formatter.extensions()) );
//     }
// }


#include "astyle_plugin.moc"
