/***************************************************************************
                          buglistcomponent.cpp  -  description
                             -------------------
    begin                : Sun Dec 10 2000
    copyright            : (C) 2000 by Ivan Hawkes
    email                : blackhawk@ivanhawkes.com
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

#include <qvbox.h>
#include <qwhatsthis.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdialogbase.h>

//#include "kdeveditormanager.h"
//#include "KDevComponentManager.h"

typedef KGenericFactory<BugListComponent> BugListFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevbuglist, BugListFactory( "kdevbuglist" ) )


BugListComponent::BugListComponent (QObject *parent, const char *name, const QStringList &)
  : KDevPlugin ("BugList", "buglist", parent, name ? name : "BugListComponent")
{
    setInstance(BugListFactory::instance());
    setXMLFile("kdevbuglist.rc");

    // Ensure it is NULL.
    m_pBugList = NULL;

    setupGUI();
}

BugListComponent::~BugListComponent()
{
    // Down she downs.
    if (m_pBugList)
        delete m_pBugList;
}

/*
ProjectSpace* BugListComponent::projectSpace(){
	return static_cast<ProjectSpace*>(componentManager()->component("ProjectSpace"));
}

KDevEditorManager* BugListComponent::editorManager(){
	return static_cast<KDevEditorManager*>(componentManager()->component("KDevEditorManager"));
}*/


// Presumably, they pressed the stop button.
void BugListComponent::slotStopButtonClicked()
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
    core()->insertNewItem( m_pMenuAction );

    // Bug tracking is only valid with a project.
    m_pMenuAction->setEnabled (FALSE);
}


void BugListComponent::slotProjectSpaceOpened()
{
    uint        Count;
    QString     LastProject;

    kdDebug(9040) << "BugList::projectSpaceOpened()" << endl;

    // Bug tracking is only valid with a project.
    m_pMenuAction->setEnabled (TRUE);

    // Grab the user details from the projectspace.
//    m_pProjectSpace = projectSpace();
//    m_Initials = m_pProjectSpace->initials ();
//    m_UserName = m_pProjectSpace->author ();
//    m_UserEMail = m_pProjectSpace->email ();

    // Get the Projectspace
    QDomDocument doc ;//= *m_pProjectSpace->readGlobalDocument();
    QDomElement psElement = doc.documentElement();

    // Get the list of projects.
    QDomNodeList projNodes = doc.elementsByTagName("Project");
    if (projNodes.count () > 1)
    {
        // Several projects - get details for last used project.
        LastProject = psElement.attribute("lastActiveProject");
        for (Count = 0;Count < projNodes.count ();Count++)
        {
            QDomElement projElement = projNodes.item(Count).toElement();
            if (LastProject == projElement.attribute("name"))
            {
                // Found the right project - grab what we need from it.
//                m_FileName = m_pProjectSpace->absolutePath () + "/" + projElement.attribute("relativePath") + projElement.attribute("bugfile");
            }
        }
    }
    else
    {
        // Just one project, use the bug file from that one.
        QDomElement projElement = projNodes.item(0).toElement();
//        m_FileName = m_pProjectSpace->absolutePath () + "/" + projElement.attribute("relativePath") + projElement.attribute("bugfile");
    }

//    kdDebug(9040) << "BugList::AbsPath = " << m_pProjectSpace->absolutePath () << endl;
    kdDebug(9040) << "BugList::BugFile = " << m_FileName << endl;
    kdDebug(9040) << "BugList::m_Initials = " << m_Initials << endl;
    kdDebug(9040) << "BugList::m_UserName = " << m_UserName << endl;
    kdDebug(9040) << "BugList::m_UserEMail = " << m_UserEMail << endl;

    // HACK: ProjectSpace not providing this stuff yet!!!
    m_Initials = "ILH";
    m_UserName = "Ivan Hawkes";
    m_UserEMail = "linuxgroupie@ivanhawkes.com";

    // Update the attributes if the component is currently running.
    if (m_pBugList)
    {
        m_pBugList->m_FileName = m_FileName;
        m_pBugList->m_Initials = m_Initials;
        m_pBugList->m_UserName = m_UserName;
        m_pBugList->m_UserEMail = m_UserEMail;
    }
}



//   Notification that the project space has now closed.


void BugListComponent::slotProjectSpaceClosed()
{
    kdDebug(9040) << "BugList::closeProjectSpace" << endl;

    // Bug tracking is only valid with a project.
    m_pMenuAction->setEnabled (FALSE);

    // Close down the tracking - warn of change lose.
    if (m_pBugList)
    {
        m_pBugList->slotCloseClicked ();
        m_pBugList = NULL;
    }

//    m_pProjectSpace = NULL;
}


//  The sub project has changed. Update the filename we will use for
//  the XML file.

void BugListComponent::slotProjectChanged()
{
    kdDebug(9040) << "BugList::projectChanged" << endl;

    // Parse the project space for the details we need on the new
    // subproject that we changed to.

// FIXME: Not in every change in project is the projectspace changed.
//        note that sigProjectChanged is generated also by others e.g. appwizard, though sigProjectSpaceOpened
//        only by the core.
//

//    slotProjectSpaceOpened ();
}


//
//  Use this slot to create an instance of the bug tracking object, or
//  bring the current instance to the fore if it is already loaded.
//

void BugListComponent::slotActivate()
{
    kdDebug(9040) << "BugList Activated" << endl;

    if (!m_pBugList)
    {
        m_pBugList = new BugList (NULL,"BugList",m_FileName,m_Initials,m_UserName,m_UserEMail);
        connect (m_pBugList, SIGNAL(signalDeactivate()), this, SLOT(slotWidgetClosed()));
        m_pBugList->show ();
    }
    else
    {
        m_pBugList->setActiveWindow ();
    }
}

//
//  Used to respond to the signal from our widget that it is closing down.
//  This gives us a chance to dispose of the widget and NULL it's pointer.
//

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
