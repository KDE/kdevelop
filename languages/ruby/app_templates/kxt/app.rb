=begin
  This class serves as the main window for %{APPNAME}.  It handles the
  menus, toolbars, and status bars.
 
  @short Main window class
  @author %{AUTHOR} <%{EMAIL}>
  @version %{VERSION}
=end

class %{APPNAMESC} < KDE::MainWindow

    slots 'fileNew()',
          'optionsPreferences()',
          'newToolbarConfig()',
          'changeStatusbar(const QString&)',
          'changeCaption(const QString&)'

    def initialize()
        super( nil, "%{APPNAMESC}" )
        Settings.instance.readConfig
        @view = %{APPNAMESC}View(self)
        @printer = nil

        # accept dnd
        setAcceptDrops(true)
    
        # tell the KDE::MainWindow that this is indeed the main widget
        setCentralWidget(@view)
    
        # then, setup our actions
        setupActions()
    
        # and a status bar
        statusBar().show()
    
        # Apply the create the main window and ask the mainwindow to
        # automatically save settings if changed: window size, toolbar
        # position, icon size, etc.  Also to add actions for the statusbar
        # toolbar, and keybindings if necessary.
        setupGUI();
    
        # allow the view to change the statusbar and caption
        connect(@view, SIGNAL('signalChangeStatusbar(const QString&)'),
                self,   SLOT('changeStatusbar(const QString&)'))
        connect(@view, SIGNAL('signalChangeCaption(const QString&)'),
                self,   SLOT('changeCaption(const QString&)'))
    
    end
    
    def setupActions()
        KDE::StdAction.openNew(self, SLOT('fileNew()'), actionCollection())
        KDE::StdAction.quit($kapp, SLOT('quit()'), actionCollection())

        KDE::StdAction.preferences(self, SLOT('optionsPreferences()'), actionCollection())
    
        # this doesn't do anything useful.  it's just here to illustrate
        # how to insert a custom menu and menu item
        custom = KDE::Action.new(i18n("Swi&tch Colors"), KDE::Shortcut.new(),
                                    @view, SLOT('switchColors()'),
                                    actionCollection(), "switch_action")
    end
    
    def fileNew()
        # This slot is called whenever the File.New menu is selected,
        # the New shortcut is pressed (usually CTRL+N) or the New toolbar
        # button is clicked
    
        # create a new window
        %{APPNAMESC}.new.show()
    end
    
    def optionsPreferences()
        # The preference dialog is derived from prefs-base.ui which is subclassed into Prefs
        # 
        # compare the names of the widgets in the .ui file
        # to the names of the variables in the .kcfg file
        dialog = KDE::ConfigDialog.new(self, "settings", Settings.instance, KDE::DialogBase::Swallow)
        dialog.addPage(Prefs.new(), i18n("General"), "package_settings")
        connect(dialog, SIGNAL('settingsChanged()'), @view, SLOT('settingsChanged()'))
        dialog.show()

    end
    
    def changeStatusbar(text)
        # display the text on the statusbar
        statusBar().message(text)
    end
    
    def changeCaption(text)
        # display the text on the caption
        setCaption(text)
    end

end
