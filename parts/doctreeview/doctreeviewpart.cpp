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
#include "kdevtoplevel.h"
#include <kdevpartcontroller.h>

#include "docsearchdlg.h"
#include "docindexdlg.h"
#include "doctreeviewpart.h"
#include "doctreeviewfactory.h"
#include "doctreeviewwidget.h"
#include "doctreeconfigwidget.h"


DocTreeViewPart::DocTreeViewPart( QObject *parent, const char *name, const QStringList & )
    : KDevPlugin(parent, name)
{
    setInstance(DocTreeViewFactory::instance());

    setXMLFile("kdevdoctreeview.rc");
    
    connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
    connect( core(), SIGNAL(configWidget(KDialogBase*)),
             this, SLOT(configWidget(KDialogBase*)) );
    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
             this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
    
    m_widget = new DocTreeViewWidget(this);
    m_widget->setIcon(SmallIcon("contents"));
    m_widget->setCaption(i18n("Documentation Tree"));
    QWhatsThis::add(m_widget, i18n("Documentation Tree\n\n"
                                   "The documentation tree gives access to library "
                                   "documentation and the KDevelop manuals. It can "
                                   "be configured individually."));
    
    topLevel()->embedSelectView(m_widget, i18n("Books"));

    KAction *action;

    action = new KAction( i18n("Lookup in &Index..."), 0,
                          this, SLOT(slotDocumentationIndex()),
                          actionCollection(), "help_lookupindex" );
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
                               "can enter a search term which will be searched in "
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
    topLevel()->removeView(m_widget);
    delete m_widget;
}


void DocTreeViewPart::projectOpened()
{
    m_widget->projectChanged(project());
    if (m_indexDialog)
        m_indexDialog->projectChanged();
}


void DocTreeViewPart::projectClosed()
{
    m_widget->projectChanged(0);
}


void DocTreeViewPart::configWidget(KDialogBase *dlg)
{
    QVBox *vbox = dlg->addVBoxPage(i18n("Documentation tree"));
    DocTreeConfigWidget *w = new DocTreeConfigWidget(m_widget, vbox, "documentation tree config widget");
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}


void DocTreeViewPart::contextMenu(QPopupMenu *popup, const Context *context)
{
    kdDebug(9002) << "context in doctree" << endl;
    if (context->hasType("editor")) {
        const EditorContext *econtext = static_cast<const EditorContext*>(context);
        QString str = econtext->linestr();
        int col = econtext->col();
        int startPos = QMAX(QMIN(col, (int)str.length()-1), 0);
        int endPos = startPos;
        while (startPos > 0 && str[startPos].isLetter())
            startPos--;
        while (endPos < (int)str.length() && str[endPos].isLetter())
            endPos++;
        QString ident = str.mid(startPos+1, endPos-startPos-1);
        kdDebug(9002) << "Identifier:" << ident << ":" << endl;
        
        kdDebug(9002) << "editor context in doctree" << endl;
        if (!ident.isEmpty())
            popupstr = ident;
            popup->insertItem( i18n("Goto manpage: %1").arg(popupstr),
                               this, SLOT(slotGotoManpage()) );
    }
    else if (context->hasType("documentation")) {
        const DocumentationContext *dcontext = static_cast<const DocumentationContext*>(context);
        kdDebug(9002) << "documentation context in doctree" << endl;
        if (!dcontext->selection().isEmpty()) {
            popupstr = dcontext->selection();
            popup->insertItem( i18n("Search in documentation: %1").arg(KStringHandler::csqueeze(popupstr, 20)),
                               this, SLOT(slotSearchDocumentationTerm()) );
        }
    }
} 


void DocTreeViewPart::slotDocumentationIndex()
{
    if (!m_indexDialog)
        m_indexDialog = new DocIndexDialog(this, m_widget, "doc index dialog");

    m_indexDialog->show();
}


void DocTreeViewPart::slotSearchDocumentation()
{
    DocSearchDialog dlg(m_widget, "doc search dialog");
    if (dlg.exec()) {
        QString indexdir = kapp->dirs()->saveLocation("data", "gideon/helpindex");
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


void DocTreeViewPart::slotGotoManpage()
{
    QString url = QString::fromLatin1("man:/%1(3)").arg(popupstr);
    partController()->showDocument(KURL(url));
}


void DocTreeViewPart::slotSearchDocumentationTerm()
{
    // We use the dialog for the interaction with htdig,
    // but do not actually show the dialog. Looks like
    // a hack, eh?
    DocSearchDialog dlg(m_widget, "doc search dialog");
    dlg.setSearchTerm(popupstr);
    if (dlg.performSearch()) {
        QString indexdir = kapp->dirs()->saveLocation("data", "gideon/helpindex");
        partController()->showDocument(KURL("file://" + indexdir + "/results.html"));
    }
}

#include "doctreeviewpart.moc"
