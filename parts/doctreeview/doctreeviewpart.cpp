/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "doctreeviewpart.h"

#include <qpopupmenu.h>
#include <qvbox.h>
#include <qwhatsthis.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <kaction.h>

#include "kdevcore.h"
#include "misc.h"
#include "kdevproject.h"
#include "kdevtoplevel.h"
#include <kdevpartcontroller.h>

#include "docsearchdlg.h"
#include "docindexdlg.h"
#include "doctreeviewfactory.h"
#include "doctreeviewwidget.h"
#include "doctreeglobalconfigwidget.h"
#include "doctreeprojectconfigwidget.h"


DocTreeViewPart::DocTreeViewPart( QObject *parent, const char *name, const QStringList & )
    : KDevPlugin(parent, name ? name : "DocTreeViewPart")
{
    setInstance(DocTreeViewFactory::instance());

    setXMLFile("kdevdoctreeview.rc");
    
    connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
    connect( core(), SIGNAL(configWidget(KDialogBase*)),
             this, SLOT(configWidget(KDialogBase*)) );
    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );
    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
             this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
    
    m_widget = new DocTreeViewWidget(this);
    m_widget->setIcon(SmallIcon("contents"));
    m_widget->setCaption(i18n("Documentation Tree"));
    QWhatsThis::add(m_widget, i18n("Documentation Tree\n\n"
                                   "The documentation tree gives access to library "
                                   "documentation and the KDevelop manuals. It can "
                                   "be configured individually."));
    
    topLevel()->embedSelectViewRight(m_widget, i18n("Documentation"), i18n("documentation browser"));

    KAction *action;

//    action = new KAction( i18n("Lookup in &Index..."), 0,
//                          this, SLOT(slotDocumentationIndex()),
//                          actionCollection(), "help_lookupindex" );
    action->setStatusText( i18n("Allows you to lookup a term in the documentation index") );
    action->setWhatsThis( i18n("Lookup in documentation index\n\n"
                               "Opens the documentation index dialog. There you "
                               "can enter a term to lookup in the various indices "
                               "of the installed documentation.") );

    action = new KAction( i18n("Full Text &Search..."), 0,
                          this, SLOT(slotSearchDocumentation()),
                          actionCollection(), "help_fulltextsearch" );
    action->setStatusText( i18n("Performs a full text search in the documentation") );
    action->setWhatsThis( i18n("Search in documentation\n\n"
                               "Opens the Search in documentation dialog. There you "
                               "can enter a search term which will be searched for in "
                               "the documentation. For this to work, you must first "
                               "create a full text index, which can be done in the "
                               "configuration dialog of the documentation tree.") );

    action = new KAction( i18n("Man Page..."), 0,
                          this, SLOT(slotManpage()),
                          actionCollection(), "help_manpage" );
    action->setStatusText( i18n("Show a manpage") );
    
#if 0
    // No longer necessary since the dock widgets are gone :-)
    action = new KAction( i18n("&Documentation Tree"), 0,
                          this, SLOT(slotRaiseWidget()),
                          actionCollection(), "window_doctreeview" );
    action->setStatusText( i18n("Brings the documentation tree to the foreground.") );
#endif
}


DocTreeViewPart::~DocTreeViewPart()
{
    delete (DocTreeViewWidget*) m_widget;
}


void DocTreeViewPart::projectOpened()
{
    m_widget->projectChanged(project());
//    if (m_indexDialog)
//        m_indexDialog->projectChanged();
}


void DocTreeViewPart::projectClosed()
{
    m_widget->projectChanged(0);
}

void DocTreeViewPart::configWidget(KDialogBase *dlg)
{
    QVBox *vbox;

    vbox = dlg->addVBoxPage(i18n("Documentation Tree"));
    DocTreeGlobalConfigWidget *w1 = 
        new DocTreeGlobalConfigWidget( this, m_widget, vbox, "doc tree config widget");

    connect( dlg, SIGNAL(okClicked()), w1, SLOT(accept()) );
}


void DocTreeViewPart::projectConfigWidget(KDialogBase *dlg) {
    QVBox *vbox;

    vbox = dlg->addVBoxPage(i18n("Project Documentation"));
    DocTreeProjectConfigWidget *w1 = 
        new DocTreeProjectConfigWidget(m_widget, vbox, project(), "doc tree project config");

    connect( dlg, SIGNAL(okClicked()), w1, SLOT(accept()) );
    //kdDebug(9002) << "**** ProjectConfigWidget ****" << endl;
}


void DocTreeViewPart::contextMenu(QPopupMenu *popup, const Context *context)
{
    kdDebug(9002) << "context in doctree" << endl;
    if (context->hasType("editor")) {
        const EditorContext *econtext = static_cast<const EditorContext*>(context);
        QString ident = econtext->currentWord();
        if (!ident.isEmpty()) {
            m_popupstr = ident;
            QString squeezed = KStringHandler::csqueeze(m_popupstr, 20);
            popup->insertSeparator();
            popup->insertItem( i18n("Search in documentation: %1").arg(squeezed),
                               this, SLOT(slotContextFulltextSearch()) );
//            popup->insertItem( i18n("Lookup in index: %1").arg(ident),
//                               this, SLOT(slotContextLookupIndex()) );
            popup->insertItem( i18n("Goto manpage: %1").arg(ident),
                               this, SLOT(slotContextGotoManpage()) );
        }
    } else if (context->hasType("documentation")) {
        const DocumentationContext *dcontext = static_cast<const DocumentationContext*>(context);
        kdDebug(9002) << "documentation context in doctree" << endl;
        QString selection = dcontext->selection();
        if (!selection.isEmpty()) {
            m_popupstr = selection;
            QString squeezed = KStringHandler::csqueeze(selection, 20);
            popup->insertSeparator();
//            popup->insertItem( i18n("Lookup in index: %1").arg(squeezed),
//                               this, SLOT(slotContextLookupIndex()) );
            popup->insertItem( i18n("Search in documentation: %1").arg(squeezed),
                               this, SLOT(slotContextFulltextSearch()) );
        }
    }
} 

/*
void DocTreeViewPart::slotDocumentationIndex()
{
    if (!m_indexDialog)
        m_indexDialog = new DocIndexDialog(this, m_widget, "doc index dialog");

    m_indexDialog->show();
}
*/

void DocTreeViewPart::slotSearchDocumentation()
{
    DocSearchDialog dlg(m_widget, "doc search dialog");
    if (dlg.exec()) {
        QString indexdir = kapp->dirs()->saveLocation("data", "kdevdoctreeview/helpindex");
        partController()->showDocument(KURL("file://" + indexdir + "/results.html"));
    }
}


void DocTreeViewPart::slotManpage()
{
    bool ok;
    QString manpage = KLineEditDlg::getText(i18n("Show manpage on:"), "", &ok, 0);
    if (ok && !manpage.isEmpty()) {
        QString url = QString::fromLatin1("man:/%1(3)").arg(manpage);
        partController()->showDocument(KURL(url));
    }
}


void DocTreeViewPart::slotRaiseWidget()
{
    topLevel()->raiseView(m_widget);
}


void DocTreeViewPart::slotContextGotoManpage()
{
    QString url = QString::fromLatin1("man:/%1(3)").arg(m_popupstr);
    partController()->showDocument(KURL(url));
}


/*void DocTreeViewPart::slotContextLookupIndex()
{
    if (!m_indexDialog)
        m_indexDialog = new DocIndexDialog(this, m_widget, "doc index dialog");

    m_indexDialog->lookup(m_popupstr);
    m_indexDialog->show();
}
*/

void DocTreeViewPart::slotContextFulltextSearch()
{
    DocSearchDialog dlg(m_widget, "doc search dialog");
    dlg.setSearchTerm(m_popupstr);
    if (dlg.performSearch()) {
        QString indexdir = kapp->dirs()->saveLocation("data", "kdevdoctreeview/helpindex");
        partController()->showDocument(KURL("file://" + indexdir + "/results.html"));
    }
}

#include "doctreeviewpart.moc"
