%{CPP_TEMPLATE}

#include "%{APPNAMELC}.h"
#include "settings.h"
#include "prefs.h"

#include <qdragobject.h>

#include <kaction.h>
#include <kconfigdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kprinter.h>
#include <kstatusbar.h>

%{APPNAMELC}::%{APPNAMELC}()
    : KMainWindow( 0, "%{APPNAMELC}" ),
      m_view(new %{APPNAMELC}View(this)),
      m_printer(0)
{
    // accept dnd
    setAcceptDrops(true);

    // tell the KMainWindow that this is indeed the main widget
    setCentralWidget(m_view);
    // and a status bar
    statusBar()->show();
    // then, setup our actions
    setupActions();

    // apply the saved mainwindow settings, if any, and ask the mainwindow
    // to automatically save settings if changed: window size, toolbar
    // position, icon size, etc.
    setAutoSaveSettings();

    // allow the view to change the statusbar and caption
    connect(m_view, SIGNAL(signalChangeStatusbar(const QString&)),
            this,   SLOT(changeStatusbar(const QString&)));
    connect(m_view, SIGNAL(signalChangeCaption(const QString&)),
            this,   SLOT(changeCaption(const QString&)));

}

%{APPNAMELC}::~%{APPNAMELC}()
{
}

void %{APPNAMELC}::setupActions()
{
    KStdAction::openNew(this, SLOT(fileNew()), actionCollection());
    KStdAction::quit(kapp, SLOT(quit()), actionCollection());

    KStdAction::preferences(this, SLOT(optionsPreferences()), actionCollection());

    // custom menu and menu item 
    KAction *custom = new KAction(i18n("Swi&tch Colors"), 0,
                                  m_view, SLOT(switchColors()),
                                  actionCollection(), "switch_action");

    setupGUI();
}

void %{APPNAMELC}::newToolbarConfig()
{
    // this slot is called when user clicks "Ok" or "Apply" in the toolbar editor.
    // recreate our GUI, and re-apply the settings (e.g. "text under icons", etc.)
    setupGUI();
}

void %{APPNAMELC}::optionsPreferences()
{
	// The preference dialog is derived from prefs-base.ui which is subclassed into Prefs
	//
	// compare the names of the widgets in the .ui file 
	// to the names of the variables in the .kcfg file
        KConfigDialog *dialog = new KConfigDialog(this, "settings", Settings::self(), KDialogBase::Swallow);
        dialog->addPage(new Prefs(), i18n("General"), "package_settings");
        connect(dialog, SIGNAL(settingsChanged()), m_view, SLOT(settingsChanged()));
        dialog->show();
}

void %{APPNAMELC}::changeCaption(const QString& text)
{
    // display the text on the caption
    setCaption(text);
}

void %{APPNAMELC}::changeStatusbar(const QString& text)
{
    // display the text on the statusbar
    statusBar()->message(text);
}

void %{APPNAMELC}::fileNew()
{
    // this slot is called whenever the File->New menu is selected,
    // the New shortcut is pressed (usually CTRL+N) or the New toolbar
    // button is clicked

    // create a new window
    (new %{APPNAMELC})->show();
}

#include "%{APPNAMELC}.moc"

