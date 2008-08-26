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
#include <QFile>

#include <KDebug>
#include <KDialog>
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
#include <kio/netaccess.h>
#include <KMimeType>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/idocument.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <project/projectmodel.h>
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

    m_formatFilesAction = actionCollection()->addAction("tools_astyle");
    m_formatFilesAction->setText(i18n("Format files"));
    m_formatFilesAction->setToolTip(i18n("Format file(s) using the current theme"));
    m_formatFilesAction->setWhatsThis(i18n("<b>Format files</b><p>Formatting functionality using <b>astyle</b> library. "
        /*"Also available in <b>New Class</b> and <b>Subclassing</b> wizards."*/));
    connect(m_formatFilesAction, SIGNAL(triggered()), this, SLOT(formatItem()));

    m_formatTextAction->setEnabled(false);
    m_formatFilesAction->setEnabled(true);

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
    m_formatter->loadConfig(KGlobal::config());
    
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
    ContextMenuExtension ext;
    m_urls.clear();
    m_prjItems.clear();
    
    if(context->hasType(Context::EditorContext))
        ext.addAction(ContextMenuExtension::EditGroup, m_formatTextAction);
     else if(context->hasType(Context::FileContext)) {
        FileContext* filectx = dynamic_cast<FileContext*>(context);
        m_urls = filectx->urls();
        ext.addAction(ContextMenuExtension::EditGroup, m_formatFilesAction);
     } else if(context->hasType(Context::CodeContext)) {
     } else if(context->hasType(Context::ProjectItemContext)) {
         ProjectItemContext* prjctx = dynamic_cast<ProjectItemContext*>(context);
         m_prjItems = prjctx->items();
         ext.addAction(ContextMenuExtension::ExtensionGroup, m_formatFilesAction);
     }    
    return ext;
}

void AStylePlugin::formatItem()
{
    if(m_prjItems.isEmpty())
        return;
    
    //get a list of all files in this folder recursively
    QList<ProjectFolderItem*> folders;
    foreach (ProjectBaseItem *item, m_prjItems) {
        if(!item)
            continue;
        if(item->folder())
            folders.append(item->folder());
        else if(item->file())
            m_urls.append(item->file()->url());
        else if(item->target()) {
            foreach (ProjectFileItem *f, item->fileList())
                 m_urls.append(f->url());
        }
    }
    
    while(!folders.isEmpty()) {
        ProjectFolderItem *item = folders.takeFirst();
        foreach (ProjectFolderItem *f, item->folderList())
            folders.append(f);
        foreach (ProjectTargetItem *f, item->targetList()) {
            foreach (ProjectFileItem *child, f->fileList())
                m_urls.append(child->url());  
        }
        foreach (ProjectFileItem *f, item->fileList())
            m_urls.append(f->url());        
   }
   
   formatFiles(m_urls);
}

void AStylePlugin::formatFiles(KUrl::List &list)
{
    m_formatter->loadConfig(KGlobal::config());

    for(int fileCount = 0; fileCount < list.size(); fileCount++) {
        // check mimetype
        KMimeType::Ptr mime = KMimeType::findByUrl(list[fileCount]);
        kDebug() << "Checking file " << list[fileCount].pathOrUrl() << " of mime type " << mime->name() << endl;
        if(!mime->is("text/x-chdr") && !mime->is("text/x-c++src"))
            continue;
        
        // if the file is opened in the editor, format the text in the editor without saving it
        IDocumentController *docController = ICore::self()->documentController();
        IDocument *doc = docController->documentForUrl(list[fileCount]);
        if(doc) {
            kDebug() << "Processing file " << list[fileCount].pathOrUrl() << "opened in editor" << endl;
            KTextEditor::Cursor cursor = doc->cursorPosition();
            doc->textDocument()->setText(m_formatter->formatSource(doc->textDocument()->text()));
            doc->setCursorPosition(cursor);
            return;
        }
        
        kDebug() << "Processing file " << list[fileCount].pathOrUrl() << endl;
        QString tmpFile, output;
        if(KIO::NetAccess::download(list[fileCount], tmpFile, 0)) {
            QFile file(tmpFile);
            // read file
            if(file.open(QFile::ReadOnly)) {
                QTextStream is(&file);
                output = m_formatter->formatSource(is.readAll());
                file.close();
            } else
                KMessageBox::error(0, i18n("Unable to read %1").arg(list[fileCount].prettyUrl()));
            
            //write new content
            if(file.open(QFile::WriteOnly|QIODevice::Truncate)) {
                QTextStream os(&file);
                os << output;
                file.close();
            } else
                KMessageBox::error(0, i18n("Unable to write to %1").arg(list[fileCount].prettyUrl()));
            
            if(!KIO::NetAccess::upload(tmpFile, list[fileCount], 0))
                KMessageBox::error(0, KIO::NetAccess::lastErrorString());
            
            KIO::NetAccess::removeTempFile(tmpFile);
        } else
            KMessageBox::error(0, KIO::NetAccess::lastErrorString());
    }
}


#include "astyle_plugin.moc"
