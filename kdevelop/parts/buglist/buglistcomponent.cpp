/***************************************************************************
                          buglistcomponent.cpp  -  description
                             -------------------
    begin                : Sun Dec 10 2000
    copyright            : (C) 2000 by Ivan Hawkes
    email                : linuxgroupie@ivanhawkes.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "buglistcomponent.h"
#include "buglist.h"
#include "main.h"

#include <qvbox.h>
#include <qwhatsthis.h>
#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>
#include <kiconloader.h>
#include <kdialogbase.h>


BugListComponent::BugListComponent (QObject *parent=0, const char *name=0)
: KDevComponent(parent, name)
{
    setInstance(BugListFactory::instance());
    setXMLFile("kdevbuglist.rc");

    // Not pointing nowhere like.
    m_pBugList = NULL;
}


BugListComponent::~BugListComponent()
{
    // Down she downs.
    if (m_pBugList)
        delete m_pBugList;
}

// Presumably, they pressed the stop button.

void BugListComponent::stopButtonClicked()
{
    kdDebug(9001) << "BugList::stopButtonClicked()" << endl;

    // Down she downs.
//    if (m_pBugList)
//    {
//        m_pBugList->slotCancelClicked ();
//        m_pBugList = NULL;
//    }
}


void BugListComponent::setupGUI()
{
    kdDebug(9001) << "SetupGUI BugList" << endl;

    KAction *action;
    action = new KAction(i18n("&Bug Tracking"), CTRL+ALT+Key_B, this,  SLOT(slotActivate()),
                         actionCollection(), "bug_tracking");
    action->setStatusText( i18n("Buglist status text.") );
    action->setWhatsThis( i18n("Buglist What Text\n\n"
                               "xxx") );
}


void BugListComponent::configWidgetRequested(KDialogBase *dlg)
{
//  QVBox *vbox = dlg->addVBoxPage(i18n("Grep"));
//  (void) new GrepConfigWidget(vbox, "grep config widget");
}


void BugListComponent::projectSpaceOpened()
{
    kdDebug(9001) << "BugList::projectSpaceOpened()" << endl;
//  m_widget->setProjectSpace(projectSpace());
/*
#if 0
    //at the moment GrepView writes only data that are user depended,
    // find the "GrepView" tag
    QDomDocument doc = pProjectSpace->readUserDocument();
    QDomNodeList grepList = doc.elementsByTagName("GrepView");
    QDomElement grepElement = grepList.item(0).toElement();
    kdDebug(9001) << "GrepView::readProjectSpaceUserConfig: Value: " << grepElement.attribute("test") << endl;
#endif
#if 0
    kdDebug(9001) << "GrepView::writeProjectSpaceUserConfig" << endl;
    QDomDocument doc = pProjectSpace->readUserDocument();
    QDomElement rootElement = doc.documentElement();
    QDomElement elem = doc.createElement("GrepView");
    elem.setAttribute("test", "value");
    rootElement.appendChild( elem );
#endif
*/
}


/*
    Use this slot to create an instance of the bug tracking object.
*/

void BugListComponent::slotActivate()
{
    kdDebug(9001) << "BugList Activated" << endl;

    if (!m_pBugList)
    {
        m_pBugList = new BugList ();
        connect (m_pBugList, SIGNAL(signalDeactivate()), this, SLOT(slotWidgetClosed()));
        m_pBugList->show ();
    }
}

/*
    Used to respond to the signal from our widget that it is closing down.
    This gives us a chance to dispose of the widget and NULL it's pointer.
*/

void BugListComponent::slotWidgetClosed ()
{
    kdDebug(9001) << "BugList Deactivate" << endl;

    // Clean up as required.
    if (m_pBugList)
    {
        delete m_pBugList;
        m_pBugList = NULL;
    }
}
