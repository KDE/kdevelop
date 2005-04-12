%{CPP_TEMPLATE}

#include "%{APPNAMELC}.h"
#include "settings.h"
#include "prefs.h"

#include <qdragobject.h>
#include <kprinter.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>

#include <kconfigdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdeversion.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kkeydialog.h>
#include <ktip.h>

#include <kedittoolbar.h>

#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>

%{APPNAME}::%{APPNAME}()
    : KMainWindow( 0, "%{APPNAME}" ),
      m_view(new %{APPNAME}View(this)),
      m_printer(0)
{
    // accept dnd
    setAcceptDrops(true);

    // tell the KMainWindow that this is indeed the main widget
    setCentralWidget(m_view);

    // then, setup our actions
    setupActions();

    // and a status bar
    statusBar()->show();

    // apply the saved mainwindow settings, if any, and ask the mainwindow
    // to automatically save settings if changed: window size, toolbar
    // position, icon size, etc.
    setAutoSaveSettings();

    // allow the view to change the statusbar and caption
    connect(m_view, SIGNAL(signalChangeStatusbar(const QString&)),
            this,   SLOT(changeStatusbar(const QString&)));
    connect(m_view, SIGNAL(signalChangeCaption(const QString&)),
            this,   SLOT(changeCaption(const QString&)));

    showTipOnStart();
}

%{APPNAME}::~%{APPNAME}()
{
}

void %{APPNAME}::setupActions()
{
    KStdAction::openNew(this, SLOT(fileNew()), actionCollection());
    KStdAction::quit(kapp, SLOT(quit()), actionCollection());

    m_toolbarAction = KStdAction::showToolbar(this, SLOT(optionsShowToolbar()), actionCollection());
    m_statusbarAction = KStdAction::showStatusbar(this, SLOT(optionsShowStatusbar()), actionCollection());

    KStdAction::tipOfDay( this, SLOT( showTip() ), actionCollection()
            )->setWhatsThis(i18n("This shows useful tips on the use of this application."));

    KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
    KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
    KStdAction::preferences(this, SLOT(optionsPreferences()), actionCollection());

    // custom menu and menu item - the slot is in the class %{APPNAME}View
    KAction *custom = new KAction(i18n("Swi&tch Colors"), 0,
                                  m_view, SLOT(switchColors()),
                                  actionCollection(), "switch_action");

    createGUI();
}

void %{APPNAME}::fileNew()
{
    // this slot is called whenever the File->New menu is selected,
    // the New shortcut is pressed (usually CTRL+N) or the New toolbar
    // button is clicked

    // create a new window
    (new %{APPNAME})->show();
}

void %{APPNAME}::optionsShowToolbar()
{
    // this is all very cut and paste code for showing/hiding the
    // toolbar
    if (m_toolbarAction->isChecked())
        toolBar()->show();
    else
        toolBar()->hide();
}

void %{APPNAME}::optionsShowStatusbar()
{
    // show/hide the statusbar
    if (m_statusbarAction->isChecked())
        statusBar()->show();
    else
        statusBar()->hide();
}

void %{APPNAME}::optionsConfigureKeys()
{
    KKeyDialog::configure(actionCollection());
}

void %{APPNAME}::optionsConfigureToolbars()
{
    // use the standard toolbar editor
    saveMainWindowSettings(KGlobal::config(), autoSaveGroup());
}

void %{APPNAME}::newToolbarConfig()
{
    // this slot is called when user clicks "Ok" or "Apply" in the toolbar editor.
    // recreate our GUI, and re-apply the settings (e.g. "text under icons", etc.)
    createGUI();

    applyMainWindowSettings(KGlobal::config(), autoSaveGroup());
}

void %{APPNAME}::optionsPreferences()
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

void %{APPNAME}::changeStatusbar(const QString& text)
{
    // display the text on the statusbar
    statusBar()->message(text, 2000);
}

void %{APPNAME}::changeCaption(const QString& text)
{
    // display the text on the caption
    setCaption(text);
}

void %{APPNAME}::showTip()
{
    KTipDialog::showTip(this,QString::null,true);
}

void %{APPNAME}::showTipOnStart()
{
    KTipDialog::showTip(this);
}

void %{APPNAME}::switch_colors()
{
    if (!m_view)
        return;
    m_view->switchColors();
}

#include "%{APPNAMELC}.moc"

