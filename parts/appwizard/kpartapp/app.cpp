/*
 * Copyright (C) 2001  <kurt@granroth.org>
 */

#include "$APPNAMELC$.h"

#include <kkeydialog.h>
#include <kconfig.h>
#include <kurl.h>

#include <kedittoolbar.h>

#include <kaction.h>
#include <kstdaction.h>

#include <klibloader.h>
#include <kmessagebox.h>
#include <kstatusbar.h>

$APPNAME$::$APPNAME$()
    : KParts::MainWindow( 0L, "$APPNAME$" )
{
    // set the shell's ui resource file
    setXMLFile("$APPNAMELC$_shell.rc");

    // then, setup our actions
    setupActions();

    // and a status bar
    statusBar()->show();

    // this routine will find and load our Part.  it finds the Part by
    // name which is a bad idea usually.. but it's alright in this
    // case since our Part is made for this Shell
    KLibFactory *factory = KLibLoader::self()->factory("lib$APPNAMELC$part");
    if (factory)
    {
        // now that the Part is loaded, we cast it to a Part to get
        // our hands on it
        m_part = static_cast<KParts::ReadWritePart *>(factory->create(this,
                                "$APPNAMELC$_part", "KParts::ReadWritePart" ));

        if (m_part)
        {
            // tell the KParts::MainWindow that this is indeed the main widget
            setCentralWidget(m_part->widget());
            
            // and integrate the part's GUI with the shell's
            createGUI(m_part);
        }
    }
    else
    {
        // if we couldn't find our Part, we exit since the Shell by
        // itself can't do anything useful
        KMessageBox::error(this, "Could not find our Part!");
        kapp->quit();
    }
}

$APPNAME$::~$APPNAME$()
{
}

void $APPNAME$::load(const KURL& url)
{
    m_part->openURL( url );
}

void $APPNAME$::setupActions()
{
    KStdAction::openNew(this, SLOT(fileNew()), actionCollection());
    KStdAction::quit(kapp, SLOT(quit()), actionCollection());

    m_toolbarAction = KStdAction::showToolbar(this, SLOT(optionsShowToolbar()), actionCollection());
    m_statusbarAction = KStdAction::showStatusbar(this, SLOT(optionsShowStatusbar()), actionCollection());

    KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
    KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
}

void $APPNAME$::saveProperties(KConfig* /*config*/)
{
    // the 'config' object points to the session managed
    // config file.  anything you write here will be available
    // later when this app is restored
}

void $APPNAME$::readProperties(KConfig* /*config*/)
{
    // the 'config' object points to the session managed
    // config file.  this function is automatically called whenever
    // the app is being restored.  read in here whatever you wrote
    // in 'saveProperties'
}

void $APPNAME$::fileNew()
{
    // this slot is called whenever the File->New menu is selected,
    // the New shortcut is pressed (usually CTRL+N) or the New toolbar
    // button is clicked

    // create a new window
    (new $APPNAME$)->show();
}


void $APPNAME$::optionsShowToolbar()
{
    // this is all very cut and paste code for showing/hiding the
    // toolbar
    if (m_toolbarAction->isChecked())
        toolBar()->show();
    else
        toolBar()->hide();
}

void $APPNAME$::optionsShowStatusbar()
{
    // this is all very cut and paste code for showing/hiding the
    // statusbar
    if (m_statusbarAction->isChecked())
        statusBar()->show();
    else
        statusBar()->hide();
}

void $APPNAME$::optionsConfigureKeys()
{
    KKeyDialog::configureKeys(actionCollection(), "$APPNAMELC$_shell.rc");
}

void $APPNAME$::optionsConfigureToolbars()
{
    // use the standard toolbar editor
    KEditToolbar dlg(actionCollection());
    if (dlg.exec())
    {
        // recreate our GUI
        createGUI(m_part);
    } 
}

#include "$APPNAMELC$.moc"
