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
#include <kinputdialog.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <kaction.h>
#include <configwidgetproxy.h>

#include "kdevcore.h"
#include "misc.h"
#include "kdevproject.h"
#include "kdevmainwindow.h"
#include "kdevpartcontroller.h"

#include "docsearchdlg.h"
#include "docindexdlg.h"
#include "doctreeviewfactory.h"
#include "doctreeviewwidget.h"
#include "doctreeglobalconfigwidget.h"
#include "doctreeprojectconfigwidget.h"

#define GLOBALDOC_OPTIONS 1
#define PROJECTDOC_OPTIONS 2

DocTreeViewPart::DocTreeViewPart( QObject *parent, const char *name, const QStringList & )
    : KDevPlugin("DocTree", "doctree", parent, name ? name : "DocTreeViewPart")
{
    setInstance(DocTreeViewFactory::instance());

    setXMLFile("kdevdoctreeview.rc");

    connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
//    connect( core(), SIGNAL(configWidget(KDialogBase*)), this, SLOT(configWidget(KDialogBase*)) );
//    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)), this, SLOT(projectConfigWidget(KDialogBase*)) );
    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
             this, SLOT(contextMenu(QPopupMenu *, const Context *)) );

    _configProxy = new ConfigWidgetProxy( core() );
    _configProxy->createGlobalConfigPage( i18n("Documentation Tree"), GLOBALDOC_OPTIONS );
    _configProxy->createProjectConfigPage( i18n("Project Documentation"), PROJECTDOC_OPTIONS );
    connect( _configProxy, SIGNAL(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int )), this, SLOT(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int )) );

    m_widget = new DocTreeViewWidget(this);
    m_widget->setIcon(SmallIcon("contents"));
    m_widget->setCaption(i18n("Documentation Tree"));
    QWhatsThis::add(m_widget, i18n("<b>Documentation tree</b><p>"
                                   "The documentation tree gives access to various "
                                   "documentation sources (Qt DCF, Doxygen, KDoc, KDevelopTOC and DevHelp "
                                   "documentation) and the KDevelop manuals. It also provides documentation index."));

    mainWindow()->embedSelectViewRight(m_widget, i18n("Documentation"), i18n("Documentation browser"));

    KAction *action;

    action = new KAction( i18n("Full Text &Search..."), 0,
                          this, SLOT(slotSearchDocumentation()),
                          actionCollection(), "help_fulltextsearch" );
    action->setToolTip( i18n("Full text search in the documentation") );
    action->setWhatsThis( i18n("<b>Full text search</b><p>"
                               "Opens the Search in documentation dialog. It allows "
                               "a search term to be entered which will be searched for in "
                               "the documentation. For this to work, a "
                               "full text index must be created first, which can be done in the "
                               "configuration dialog of the documentation tree.") );

    action = new KAction( i18n("Man Page..."), 0,
                          this, SLOT(slotManpage()),
                          actionCollection(), "help_manpage" );
    action->setToolTip( i18n("Show a manpage") );
    action->setWhatsThis(i18n("<b>Show a manpage</b><p>Opens a man page using embedded viewer."));
}


DocTreeViewPart::~DocTreeViewPart()
{
    if ( m_widget )
        mainWindow()->removeView( m_widget );
    delete (DocTreeViewWidget*) m_widget;
}


void DocTreeViewPart::projectOpened()
{
    m_widget->projectChanged(project());
}


void DocTreeViewPart::projectClosed()
{
    m_widget->projectChanged(0);
}
/*
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
*/

void DocTreeViewPart::contextMenu(QPopupMenu */*popup*/, const Context */*context*/)
{
/*    kdDebug(9002) << "context in doctree" << endl;
    if (context->hasType( Context::EditorContext )) {
        const EditorContext *econtext = static_cast<const EditorContext*>(context);
        QString ident = econtext->currentWord();
        if (!ident.isEmpty()) {
            m_popupstr = ident;
            QString squeezed = KStringHandler::csqueeze(m_popupstr, 20);
            popup->insertSeparator();
            int id = popup->insertItem( i18n("Search in Documentation: %1").arg(squeezed),
                               this, SLOT(slotContextFulltextSearch()) );
            popup->setWhatsThis(id, i18n("<b>Search in documentation</b><p>Searches "
                               "for a term under the cursor in "
                               "the documentation. For this to work, "
                               "a full text index must be created first, which can be done in the "
                               "configuration dialog of the documentation tree."));
            id = popup->insertItem( i18n("Goto Manpage: %1").arg(ident),
                               this, SLOT(slotContextGotoManpage()) );
            popup->setWhatsThis(id, i18n("<b>Goto manpage</b><p>Tries to open a man page for the term under the cursor."));
        }
    } else if (context->hasType( Context::DocumentationContext )) {
        const DocumentationContext *dcontext = static_cast<const DocumentationContext*>(context);
        kdDebug(9002) << "documentation context in doctree" << endl;
        QString selection = dcontext->selection();
        if (!selection.isEmpty()) {
            m_popupstr = selection;
            QString squeezed = KStringHandler::csqueeze(selection, 20);
            popup->insertSeparator();
            int id = popup->insertItem( i18n("Search in Documentation: %1").arg(squeezed),
                               this, SLOT(slotContextFulltextSearch()) );
            popup->setWhatsThis(id, i18n("<b>Search in documentation</b><p>Searches "
                               "for a text of currently selected documentation item in "
                               "the documentation. For this to work, "
                               "a full text index must be created first, which can be done in the "
                               "configuration dialog of the documentation tree."));
        }
    }*/
}

void DocTreeViewPart::slotSearchDocumentation()
{
    kdDebug(9002) << "Full text search requested" << endl;
    DocSearchDialog dlg(m_widget, "doc search dialog");
    if (dlg.exec()) {
        QString indexdir = kapp->dirs()->saveLocation("data", "kdevdoctreeview/helpindex");
        partController()->showDocument(KURL("file://" + indexdir + "/results.html"));
    }
}

void DocTreeViewPart::slotManpage()
{
    bool ok;
    QString manpage = KInputDialog::getText(i18n("Show Manpage"), i18n("Show manpage on:"), "", &ok, 0);
    if (ok && !manpage.isEmpty()) {
        QString url = QString::fromLatin1("man:/%1").arg(manpage);
        partController()->showDocument(KURL(url));
    }
}

void DocTreeViewPart::slotRaiseWidget()
{
    mainWindow()->raiseView(m_widget);
}


void DocTreeViewPart::slotContextGotoManpage()
{
    QString url = QString::fromLatin1("man:/%1").arg(m_popupstr);
    partController()->showDocument(KURL(url));
}

void DocTreeViewPart::slotContextFulltextSearch()
{
    DocSearchDialog dlg(m_widget, "doc search dialog");
    dlg.setSearchTerm(m_popupstr);
    if (dlg.performSearch()) {
        QString indexdir = kapp->dirs()->saveLocation("data", "kdevdoctreeview/helpindex");
        partController()->showDocument(KURL("file://" + indexdir + "/results.html") );
    }
}

void DocTreeViewPart::insertConfigWidget( const KDialogBase * dlg, QWidget * page, unsigned int pagenumber )
{
	switch ( pagenumber )
	{
		case GLOBALDOC_OPTIONS:
		{
			DocTreeGlobalConfigWidget *w1 = new DocTreeGlobalConfigWidget( this, m_widget, page, "doc tree config widget");
			connect( dlg, SIGNAL(okClicked()), w1, SLOT(accept()) );

		break;
		}
		case PROJECTDOC_OPTIONS:
		{
			DocTreeProjectConfigWidget *w1 = new DocTreeProjectConfigWidget(m_widget, page, project(), "doc tree project config");
			connect( dlg, SIGNAL(okClicked()), w1, SLOT(accept()) );

		break;
		}
	}
}

#include "doctreeviewpart.moc"
