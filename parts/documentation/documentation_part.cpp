/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "documentation_part.h"

#include <qwhatsthis.h>
#include <qlayout.h>
#include <qpopupmenu.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <ktrader.h>
#include <kdebug.h>
#include <kparts/componentfactory.h>
#include <kservice.h>
#include <kdialogbase.h>
#include <kaction.h>
#include <kactionclasses.h>
#include <kbookmark.h>
#include <kbookmarkmenu.h>
#include <kinputdialog.h>
#include <kstringhandler.h>

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevgenericfactory.h"
#include "kdevdocumentationplugin.h"
#include "configwidgetproxy.h"
#include "kdevpartcontroller.h"

#include "documentation_widget.h"
#include "docglobalconfigwidget.h"
#include "contentsview.h"

#define GLOBALDOC_OPTIONS 1
#define PROJECTDOC_OPTIONS 2

static const KAboutData data("kdevdocumentation", I18N_NOOP("Documentation"), "1.0");

typedef KDevGenericFactory<DocumentationPart> DocumentationFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevdocumentation, DocumentationFactory( &data ) );

DocumentationPart::DocumentationPart(QObject *parent, const char *name, const QStringList& )
    :KDevPlugin("Documentation", "doctree", parent, name ? name : "DocumentationPart" )
{
    setInstance(DocumentationFactory::instance());
    setXMLFile("kdevpart_documentation.rc");

    m_configProxy = new ConfigWidgetProxy(core());
    m_configProxy->createGlobalConfigPage(i18n("Documentation"), GLOBALDOC_OPTIONS);
/*    m_configProxy->createProjectConfigPage(i18n("Project Documentation"), PROJECTDOC_OPTIONS);*/
    connect(m_configProxy, SIGNAL(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int )), this, SLOT(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int)));
    connect(core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
        this, SLOT(contextMenu(QPopupMenu *, const Context *)));
    
    m_widget = new DocumentationWidget(this);
    m_widget->setIcon(SmallIcon("contents"));
    m_widget->setCaption(i18n("Documentation"));

    QWhatsThis::add(m_widget, i18n("<b>Documentation browser</b><p>"
        "The documentation browser gives access to various "
        "documentation sources (Qt DCF, Doxygen, KDoc, KDevelopTOC and DevHelp "
        "documentation) and the KDevelop manuals. It also provides documentation index "
        "and full text search capabilities."));

    mainWindow()->embedSelectViewRight(m_widget, i18n("Documentation"),
        i18n("Documentation browser"));
    
    setupActions();
    
    loadDocumentationPlugins();
}

DocumentationPart::~DocumentationPart()
{
    if ( m_widget )
    {
        mainWindow()->removeView( m_widget );
    }
    delete m_widget;
}

void DocumentationPart::loadDocumentationPlugins()
{
    KTrader::OfferList docPluginOffers =
        KTrader::self()->query(QString::fromLatin1("KDevelop/DocumentationPlugins"));

    KTrader::OfferList::ConstIterator serviceIt = docPluginOffers.begin();
    for ( ; serviceIt != docPluginOffers.end(); ++serviceIt )
    {               
        KService::Ptr docPluginService;
        docPluginService = *serviceIt;
        kdDebug() << "DocumentationPart::loadDocumentationPlugins: creating plugin" 
            << docPluginService->name() << endl;
        
        int error;
        DocumentationPlugin *docPlugin = KParts::ComponentFactory
            ::createInstanceFromService<DocumentationPlugin>(docPluginService, 0,
            docPluginService->name().latin1(), QStringList(), &error);
        if (!docPlugin)
            kdDebug() << "    failed to create doc plugin " << docPluginService->name() << endl;
        else
        {
            kdDebug() << "    success" << endl;
            docPlugin->init(m_widget->contents());
            connect(this, SIGNAL(indexSelected(KListBox* )), docPlugin, SLOT(createIndex(KListBox* )));
            m_plugins.append(docPlugin);
        }
    }
}

void DocumentationPart::emitIndexSelected(KListBox *indexBox)
{
    emit indexSelected(indexBox);
}

void DocumentationPart::insertConfigWidget(const KDialogBase *dlg, QWidget *page, unsigned int pageNo)
{
    switch (pageNo)
    {
        case GLOBALDOC_OPTIONS:
        {
            DocGlobalConfigWidget *w1 = new DocGlobalConfigWidget(this, m_widget, page, "doc config widget");
            connect(dlg, SIGNAL(okClicked()), w1, SLOT(accept()));
            break;
        }
        case PROJECTDOC_OPTIONS:
        {
/*            DocProjectConfigWidget *w1 = new DocProjectConfigWidget(m_widget, page, project(), "doc project config");
            connect(dlg, SIGNAL(okClicked()), w1, SLOT(accept()));*/
            break;
        }
    }
}

KConfig *DocumentationPart::config()
{
    return DocumentationFactory::instance()->config();
}

bool DocumentationPart::configure()
{
    KDialogBase dlg(KDialogBase::Plain, i18n("Documentation Settings"),
                    KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, m_widget,
                    "docsettings dialog");

    QVBoxLayout *l = new QVBoxLayout(dlg.plainPage(), 0, 0);
    DocGlobalConfigWidget *w1 = new DocGlobalConfigWidget(this, m_widget, dlg.plainPage());
    l->addWidget(w1);
    connect(&dlg, SIGNAL(okClicked()), w1, SLOT(accept()));
    return (dlg.exec() == QDialog::Accepted);
}

void DocumentationPart::setupActions()
{
/*    KActionMenu *menu = new KActionMenu(i18n("&Bookmarks"), actionCollection(), "doc_bookmarks");
    KBookmarkMenu *bm = new KBookmarkMenu(new DocBookmarkManager(this),
        new DocBookmarkOwner(this), menu->popupMenu(), actionCollection(), true);*/
    KAction *action;

    action = new KAction(i18n("&Search in Documentation..."), CTRL+ALT+Key_S,
                         this, SLOT(searchInDocumentation()),
                         actionCollection(), "help_search_in_doc" );
    action->setToolTip(i18n("Full text search in the documentation"));
    action->setWhatsThis(i18n("<b>Search in documentation</b><p>"
                              "Opens the Search in documentation tab. It allows "
                              "a search term to be entered which will be searched for in "
                              "the documentation. For this to work, a "
                              "full text index must be created first, which can be done in the "
                              "configuration dialog of the documentation plugin."));    
    action = new KAction(i18n("&Look in Documentation Index..."), CTRL+ALT+Key_I,
                         this, SLOT(lookInDocumentationIndex()),
                         actionCollection(), "help_look_in_index" );
    action->setToolTip(i18n("Look in the documentation index"));
    action->setWhatsThis(i18n("<b>Look in documentation index</b><p>"
                              "Opens the documentation index tab. It allows "
                              "a term to be entered which will be looked for in "
                              "the documentation index."));    
    action = new KAction(i18n("Man Page..."), 0,
                         this, SLOT(manPage()),
                         actionCollection(), "help_manpage" );
    action->setToolTip(i18n("Show a manpage"));
    action->setWhatsThis(i18n("<b>Show a manpage</b><p>Opens a man page using embedded viewer."));
    action = new KAction(i18n("Info Page..."), 0,
                         this, SLOT(infoPage()),
                         actionCollection(), "help_infopage");
    action->setToolTip(i18n("Show an infopage"));
    action->setWhatsThis(i18n("<b>Show an infopage</b><p>Opens an info page using embedded viewer."));
}

void DocumentationPart::emitBookmarkLocation(const QString &title, const KURL &url)
{
    emit bookmarkLocation(title, url);
}

void DocumentationPart::searchInDocumentation()
{
    mainWindow()->raiseView(m_widget);
    m_widget->searchInDocumentation();
}

void DocumentationPart::searchInDocumentation(const QString &term)
{
    mainWindow()->raiseView(m_widget);
    m_widget->searchInDocumentation(term);
}

void DocumentationPart::contextSearchInDocumentation()
{
    mainWindow()->raiseView(m_widget);
    searchInDocumentation(m_contextStr);
}

void DocumentationPart::manPage()
{
    bool ok;
    QString manpage = KInputDialog::getText(i18n("Show manpage on:"), "", "", &ok, 0);
    if (ok && !manpage.isEmpty())
        manPage(manpage);
}

void DocumentationPart::infoPage()
{
    bool ok;
    QString infopage = KInputDialog::getText(i18n("Show infopage on:"), "", "", &ok, 0);
    if (ok && !infopage.isEmpty())
        infoPage(infopage);
}

void DocumentationPart::manPage(const QString &term)
{
    QString url = QString::fromLatin1("man:/%1").arg(term);
    partController()->showDocument(KURL(url));
}

void DocumentationPart::infoPage(const QString &term)
{
    QString url = QString::fromLatin1("info:/%1").arg(term);
    partController()->showDocument(KURL(url));
}

void DocumentationPart::contextManPage()
{
    manPage(m_contextStr);
}

void DocumentationPart::contextInfoPage()
{
    infoPage(m_contextStr);
}

void DocumentationPart::contextMenu(QPopupMenu *popup, const Context *context)
{
    if (context->hasType(Context::EditorContext))
    {
        const EditorContext *econtext = static_cast<const EditorContext*>(context);
        QString ident = econtext->currentWord();
        if (!ident.isEmpty()) 
        {
            m_contextStr = ident;
            QString squeezed = KStringHandler::csqueeze(m_contextStr, 20);            
            int id = popup->insertItem(i18n("Look in Documentation Index: %1").arg(squeezed),
                               this, SLOT(contextLookInDocumentationIndex()));
            popup->setWhatsThis(id, i18n("<b>Look in documentation index</b><p>"
                              "Opens the documentation index tab. It allows "
                              "a term to be entered which will be looked for in "
                              "the documentation index."));
            id = popup->insertItem(i18n("Search in Documentation: %1").arg(squeezed),
                               this, SLOT(contextSearchInDocumentation()));
            popup->setWhatsThis(id, i18n("<b>Search in documentation</b><p>Searches "
                               "for a term under the cursor in "
                               "the documentation. For this to work, "
                               "a full text index must be created first, which can be done in the "
                               "configuration dialog of the documentation plugin."));
            id = popup->insertItem(i18n("Goto Manpage: %1").arg(squeezed),
                               this, SLOT(contextManPage()));
            popup->setWhatsThis(id, i18n("<b>Goto manpage</b><p>Tries to open a man page for the term under the cursor."));
            id = popup->insertItem( i18n("Goto Infopage: %1").arg(squeezed),
                               this, SLOT(contextInfoPage()) );
            popup->setWhatsThis(id, i18n("<b>Goto infopage</b><p>Tries to open an info page for the term under the cursor."));
            popup->insertSeparator();
        }
    }
}

void DocumentationPart::lookInDocumentationIndex()
{
    mainWindow()->raiseView(m_widget);
    m_widget->lookInDocumentationIndex();
}

void DocumentationPart::lookInDocumentationIndex(const QString &term)
{
    mainWindow()->raiseView(m_widget);
    m_widget->lookInDocumentationIndex(term);
}

void DocumentationPart::contextLookInDocumentationIndex()
{
    lookInDocumentationIndex(m_contextStr);
}

#include "documentation_part.moc"
