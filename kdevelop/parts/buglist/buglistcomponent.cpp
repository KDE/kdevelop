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
    kdDebug(9040) << "BugList::stopButtonClicked()" << endl;

    // Down she goes.
//    if (m_pBugList)
//    {
//        m_pBugList->slotCancelClicked ();
//        m_pBugList = NULL;
//    }
}


void BugListComponent::setupGUI()
{
    kdDebug(9040) << "SetupGUI BugList" << endl;

    m_pMenuAction = new KAction(i18n("&Bug Tracking"), CTRL+ALT+Key_B, this,  SLOT(slotActivate()),
                         actionCollection(), "bug_tracking");
    m_pMenuAction->setStatusText (i18n("Provides bug tracking features for your project."));
    m_pMenuAction->setWhatsThis (i18n("Provides bug tracking features for your project."));

    // Bug tracking is only valid with a project.
    m_pMenuAction->setEnabled (FALSE);
}


void BugListComponent::configWidgetRequested(KDialogBase *dlg)
{
//  QVBox *vbox = dlg->addVBoxPage(i18n("Grep"));
//  (void) new GrepConfigWidget(vbox, "grep config widget");
}


void BugListComponent::projectSpaceOpened()
{
    kdDebug(9040) << "BugList::projectSpaceOpened()" << endl;

    // Take a note of the project space in use.
    if (projectSpace ())
    {
        // Bug tracking is only valid with a project.
        m_pMenuAction->setEnabled (TRUE);

        // Keep a track of the current project space. Probably not needed.
        kdDebug(9040) << "BugList::Project Space Read" << endl;
        m_pProjectSpace = projectSpace();

        QDomDocument doc = *m_pProjectSpace->readGlobalDocument();
//        QDomNodeList projNodes = doc->elementsByTagName("BugList");
//        QDomElement bugElement = projNodes.item(0).toElement();
//        kdDebug(9040) << "BugList::file = " << bugElement.attribute("file") << endl;
  QDomElement psElement = doc.documentElement(); // get the Projectspace
  kdDebug(9040) << "BugList::filex = " << psElement.attribute("pluginName") << endl;
    }
    else
    {
        kdDebug(9040) << "BugList::Project Space Write" << endl;

/*        QDomDocument *doc = m_pProjectSpace->readUserDocument();
        QDomElement rootElement = doc->documentElement();
        QDomElement elem = doc->createElement("BugList");
        elem.setAttribute ("test", "value");
        rootElement.appendChild (elem);*/
    }
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
    Notification that the project space has now closed.

    NOTE: Doesn't seem to work yet.
*/

void BugListComponent::projectSpaceClosed()
{
    // Bug tracking is only valid with a project.
    m_pMenuAction->setEnabled (FALSE);

    m_pProjectSpace = NULL;
}


/*
    Use this slot to create an instance of the bug tracking object.
*/

void BugListComponent::slotActivate()
{
    kdDebug(9040) << "BugList Activated" << endl;

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
    kdDebug(9040) << "BugList Deactivate" << endl;

    // Clean up as required.
    if (m_pBugList)
    {
        delete m_pBugList;
        m_pBugList = NULL;
    }
}
#include "buglistcomponent.moc"
