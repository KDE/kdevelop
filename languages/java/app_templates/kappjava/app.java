/*
 * Copyright (C) %{YEAR} %{AUTHOR} <%{EMAIL}>
 */

import java.util.*;
import org.kde.qt.*;
import org.kde.koala.*;

/**
 * This class serves as the main window for %{APPNAME}.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author $AUTHOR <$EMAIL>
 * @version $APP_VERSION
 */
public class %{APPNAME} extends KMainWindow
{
    private %{APPNAME}View m_view;

    private QPrinter   m_printer;
    private KToggleAction m_toolbarAction;
    private KToggleAction m_statusbarAction;

%{APPNAME}()
{
    super( null, "%{APPNAME}" );
    m_view = new %{APPNAME}View(this);
    m_printer = new QPrinter();
    // accept dnd
    setAcceptDrops(true);

    // tell the KMainWindow that this is indeed the main widget
    setCentralWidget(m_view);

    // then, setup our actions
    setupActions();

    // and a status bar
    statusBar().show();

    // allow the view to change the statusbar and caption
    connect(m_view, SIGNAL("signalChangeStatusbar(String)"),
            this,   SLOT("changeStatusbar(String)"));
    connect(m_view, SIGNAL("signalChangeCaption(String)"),
            this,   SLOT("changeCaption(String)"));

}

public void load(KURL url)
{
    String target = null;
    // the below code is what you should normally do.  in this
    // example case, we want the url to our own.  you probably
    // want to use this code instead for your app

    // download the contents
    if (NetAccess.download(url, target))
    {
        // set our caption
        setCaption(url.fileName());

        // load in the file (target is always local)
//        loadFile(target);

        // and remove the temp file
        NetAccess.removeTempFile(target);
    }

    setCaption(url.url());
    m_view.openURL(url);
}

public void setupActions()
{
    KApplication kapp = KApplication.kApplication();
    KStdAction.openNew(this, SLOT("fileNew()"), actionCollection());
    KStdAction.open(this, SLOT("fileOpen()"), actionCollection());
    KStdAction.save(this, SLOT("fileSave()"), actionCollection());
    KStdAction.saveAs(this, SLOT("fileSaveAs()"), actionCollection());
    KStdAction.print(this, SLOT("filePrint()"), actionCollection());
    KStdAction.quit(kapp, SLOT("quit()"), actionCollection());

    m_toolbarAction = KStdAction.showToolbar(this, SLOT("optionsShowToolbar()"), actionCollection());
    m_statusbarAction = KStdAction.showStatusbar(this, SLOT("optionsShowStatusbar()"), actionCollection());

    KStdAction.keyBindings(this, SLOT("optionsConfigureKeys()"), actionCollection());
    KStdAction.configureToolbars(this, SLOT("optionsConfigureToolbars()"), actionCollection());
    KStdAction.preferences(this, SLOT("optionsPreferences()"), actionCollection());

    // this doesn't do anything useful.  it's just here to illustrate
    // how to insert a custom menu and menu item
    KAction custom = new KAction(tr("Cus&tom Menuitem"), 0,
                                  this, SLOT("optionsPreferences()"),
                                  actionCollection(), "custom_action");
    createGUI();
}

protected void saveProperties(KConfig config)
{
    // the 'config' object points to the session managed
    // config file.  anything you write here will be available
    // later when this app is restored

    if (m_view.currentURL() != null)
        config.writeEntry("lastURL", m_view.currentURL());
}

protected void readProperties(KConfig config)
{
    // the 'config' object points to the session managed
    // config file.  this function is automatically called whenever
    // the app is being restored.  read in here whatever you wrote
    // in 'saveProperties'

    String url = config.readPathEntry("lastURL");

    if (url != null)
        m_view.openURL(new KURL(url));
}

protected void dragEnterEvent(QDragEnterEvent event)
{
    // accept uri drops only
    event.accept(QUriDrag.canDecode(event));
}

protected void dropEvent(QDropEvent event)
{
    // this is a very simplistic implementation of a drop event.  we
    // will only accept a dropped URL.  the Qt dnd code can do *much*
    // much more, so please read the docs there
    ArrayList uri = new ArrayList();

    // see if we can decode a URI.. if not, just ignore it
    if (QUriDrag.decode(event, (String[]) uri.toArray()))
    {
        // okay, we have a URI.. process it
        String url, target;
        url = (String) uri.get(0);

        // load in the file
        load(new KURL(url));
    }
}

private void fileNew()
{
    // this slot is called whenever the File.New menu is selected,
    // the New shortcut is pressed (usually CTRL+N) or the New toolbar
    // button is clicked

    // create a new window
    (new %{APPNAME}()).show();
}

private void fileOpen()
{
    // this slot is called whenever the File.Open menu is selected,
    // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
    // button is clicked
    KURL url = KURLRequesterDlg.getURL(null, this, tr("Open Location") );
    if (!url.isEmpty())
        m_view.openURL(url);
}

private void fileSave()
{
    // this slot is called whenever the File.Save menu is selected,
    // the Save shortcut is pressed (usually CTRL+S) or the Save toolbar
    // button is clicked

    // save the current file
}

private void fileSaveAs()
{
    // this slot is called whenever the File.Save As menu is selected,
    KURL file_url = KFileDialog.getSaveURL();
    if (!file_url.isEmpty() && !file_url.isMalformed())
    {
        // save your info, here
    }
}

private void filePrint()
{
    // this slot is called whenever the File.Print menu is selected,
    // the Print shortcut is pressed (usually CTRL+P) or the Print toolbar
    // button is clicked
    if (m_printer == null) m_printer = new QPrinter();
    if (QPrintDialog.getPrinterSetup(m_printer))
    {
        // setup the printer.  with Qt, you always "print" to a
        // QPainter.. whether the output medium is a pixmap, a screen,
        // or paper
        QPainter p = new QPainter();
        p.begin(m_printer);

        // we let our view do the actual printing
        QPaintDeviceMetrics metrics = new QPaintDeviceMetrics(m_printer);
        m_view.print(p, metrics.height(), metrics.width());

        // and send the result to the printer
        p.end();
    }
}

private void optionsShowToolbar()
{
    // this is all very cut and paste code for showing/hiding the
    // toolbar
    if (m_toolbarAction.isChecked())
        toolBar().show();
    else
        toolBar().hide();
}

private void optionsShowStatusbar()
{
    // this is all very cut and paste code for showing/hiding the
    // statusbar
    if (m_statusbarAction.isChecked())
        statusBar().show();
    else
        statusBar().hide();
}

private void optionsConfigureKeys()
{
    KKeyDialog.configureKeys(actionCollection(), "%{APPNAMELC}ui.rc");
}

private void optionsConfigureToolbars()
{
    // use the standard toolbar editor
    KEditToolbar dlg = new KEditToolbar(actionCollection());
    if (dlg.exec() != 0)
    {
        // recreate our GUI
        createGUI();
    }
}

private void optionsPreferences()
{
    // popup some sort of preference dialog, here
    %{APPNAME}Preferences dlg = new %{APPNAME}Preferences();
    if (dlg.exec() != 0)
    {
        // redo your settings
    }
}

private void changeStatusbar(String text)
{
    // display the text on the statusbar
    statusBar().message(text);
}

private void changeCaption(String text)
{
    // display the text on the caption
    setCaption(text);
}

static String description =
    "A KDE Application";

static String version = "%{VERSION}";

static String[][] options =
{
    { "+[URL]", "Document to open.", null }
};

static void main(String[] cmdLineArgs)
{
    KAboutData about = new KAboutData("%{APPNAMELC}", "%{APPNAME}", version, description,
                     KAboutData.License_$LICENSE$, "(C) %{YEAR} %{AUTHOR}", null, null, "%{EMAIL}");
    about.addAuthor( "%{AUTHOR}", null, "%{EMAIL}" );
    KCmdLineArgs.init(cmdLineArgs, about);
    KCmdLineArgs.addCmdLineOptions(options);
    KApplication app = new KApplication();

    // see if we are starting with session management
    if (app.isRestored())
        RESTORE("%{APPNAME}");
    else
    {
        // no session.. just start up normally
        KCmdLineArgs args = KCmdLineArgs.parsedArgs();
        if (args.count() == 0)
        {
            %{APPNAME} widget = new %{APPNAME}();
            widget.show();
        }
        else
        {
            int i = 0;
            for (; i < args.count(); i++)
            {
                %{APPNAME} widget = new %{APPNAME}();
                widget.show();
                widget.load(args.url(i));
            }
        }
        args.clear();
    }

    app.exec();
    return;
}
	
	static {
		System.loadLibrary("qtjava");
		try {
			Class c = Class.forName("org.kde.qt.qtjava");
		} catch (Exception e) {
			System.out.println("Can't load qtjava class");
		}
		
		System.loadLibrary("kdejava");
		try {
			Class c = Class.forName("org.kde.koala.kdejava");
		} catch (Exception e) {
			System.out.println("Can't load kdejava class");
		}
	}

}

