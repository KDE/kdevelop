=begin
  This class serves as the main window for %{APPNAME}.  It handles the
  menus, toolbars, and status bars.
 
  @short Main window class
  @author %{AUTHOR} <%{EMAIL}>
  @version %{VERSION}
=end

class %{APPNAME} < KDE::MainWindow

    slots 'fileNew()',
          'fileOpen()',
          'fileSave()',
          'fileSaveAs()',
          'filePrint()',
          'optionsShowToolbar()',
          'optionsShowStatusbar()',
          'optionsConfigureKeys()',
          'optionsConfigureToolbars()',
          'optionsPreferences()',
          'newToolbarConfig()',
          'changeStatusbar(const QString&)',
          'changeCaption(const QString&)'

    def initialize()
        super( nil, "%{APPNAME}" )
        @view = %{APPNAME}View.new(self)
        @printer = nil
        # accept dnd
        setAcceptDrops(true)
    
        # tell the KDE::MainWindow that this is indeed the main widget
        setCentralWidget(@view)
    
        # then, setup our actions
        setupActions()
    
        # and a status bar
        statusBar().show()
    
        # apply the saved mainwindow settings, if any, and ask the mainwindow
        # to automatically save settings if changed: window size, toolbar
        # position, icon size, etc.
        setAutoSaveSettings()
    
        # allow the view to change the statusbar and caption
        connect(@view, SIGNAL('signalChangeStatusbar(const QString&)'),
                self,   SLOT('changeStatusbar(const QString&)'))
        connect(@view, SIGNAL('signalChangeCaption(const QString&)'),
                self,   SLOT('changeCaption(const QString&)'))
    
    end
    
    
    def load(url)
        target = ""
        # the below code is what you should normally do.  in this
        # example when, we want the url to our own.  you probably
        # want to use this code instead for your app
    
        if false
        # download the contents
        if KIO::NetAccess.download(url, target, self)
            # set our caption
            setCaption(url.url)
    
            # load in the file (target is always local)
            @view.openURL(KDE::URL.new(target))
    
            # and remove the temp file
            KIO::NetAccess.removeTempFile(target)
        end
        end
    
        setCaption(url.prettyURL())
        @view.openURL(url)
    end
    
    def setupActions()
        KDE::StdAction.openNew(self, SLOT('fileNew()'), actionCollection())
        KDE::StdAction.open(self, SLOT('fileOpen()'), actionCollection())
        KDE::StdAction.save(self, SLOT('fileSave()'), actionCollection())
        KDE::StdAction.saveAs(self, SLOT('fileSaveAs()'), actionCollection())
        KDE::StdAction.print(self, SLOT('filePrint()'), actionCollection())
        KDE::StdAction.quit($kapp, SLOT('quit()'), actionCollection())
    
        @statusbarAction = KDE::StdAction.showStatusbar(self, SLOT('optionsShowStatusbar()'), actionCollection())
    
        KDE::StdAction.keyBindings(self, SLOT('optionsConfigureKeys()'), actionCollection())
        KDE::StdAction.configureToolbars(self, SLOT('optionsConfigureToolbars()'), actionCollection())
        KDE::StdAction.preferences(self, SLOT('optionsPreferences()'), actionCollection())
    
        # this doesn't do anything useful.  it's just here to illustrate
        # how to insert a custom menu and menu item
        custom = KDE::Action.new(i18n("Cus&tom Menuitem"), KDE::Shortcut.new(),
                                    self, SLOT('optionsPreferences()'),
                                    actionCollection(), "custom_action")
        createGUI(Dir.getwd + '/%{APPNAMELC}ui.rc')
    end
    
    def saveProperties(config)
        # the 'config' object points to the session managed
        # config file.  anything you write here will be available
        # later when this app is restored
    
        if !@view.currentURL().empty?
            config.writeEntry("lastURL", @view.currentURL())
        end
    end
    
    def readProperties(config)
        # the 'config' object points to the session managed
        # config file.  This function is automatically called whenever
        # the app is being restored.  read in here whatever you wrote
        # in 'saveProperties'
    
        url = config.readPathEntry("lastURL")
    
        if !url.empty?
            @view.openURL(KDE::URL.new(url))
        end
    end
    
    def dragEnterEvent(event)
        # accept uri drops only
        event.accept(KDE::URLDrag.canDecode(event))
    end
    
    def dropEvent(event)
        # This is a very simplistic implementation of a drop event.  we
        # will only accept a dropped URL.  the Qt dnd code can do *much*
        # much more, so please read the docs there
        urls = []
    
        # see if we can decode a URI.. if not, just ignore it
        if KDE::URLDrag.decode(event, urls) && !urls.empty?
            # okay, we have a URI.. process it
            url = urls.shift
    
            # load in the file
            load(url)
        end
    end
    
    def fileNew()
        # This slot is called whenever the File.New menu is selected,
        # the New shortcut is pressed (usually CTRL+N) or the New toolbar
        # button is clicked
    
        # create a new window
        %{APPNAME}.new.show()
    end
    
    def fileOpen()
        # This slot is called whenever the File.Open menu is selected,
        # the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
        # button is clicked
    
        # This brings up the generic open dialog
        url = KDE::URLRequesterDlg.getURL(nil, self, i18n("Open Location") )
    
        # standard filedialog
        url = KDE::FileDialog.getOpenURL(nil, nil, self, i18n("Open Location"))
        if !url.empty?
            @view.openURL(url)
        end
    end
    
    def fileSave()
        # This slot is called whenever the File.Save menu is selected,
        # the Save shortcut is pressed (usually CTRL+S) or the Save toolbar
        # button is clicked
    
        # save the current file
    end
    
    def fileSaveAs()
        # This slot is called whenever the File.Save As menu is selected,
        file_url = KDE::FileDialog.getSaveURL()
        if !file_url.empty? && file_url.valid?
            # save your info, here
        end
    end
    
    def filePrint()
        # This slot is called whenever the File.Print menu is selected,
        # the Print shortcut is pressed (usually CTRL+P) or the Print toolbar
        # button is clicked
        if @printer.nil? then @printer = KDE::Printer.new end
        if @printer.setup(self)
            # setup the printer.  with Qt, you always "print" to a
            # Qt::Painter.. whether the output medium is a pixmap, a screen,
            # or paper
            p = Qt::Painter.new
            p.begin(@printer)
    
            # we let our view do the actual printing
            metrics = Qt::PaintDeviceMetrics.new(@printer)
            @view.print(p, metrics.height(), metrics.width())
    
            # and send the result to the printer
            p.end()
        end
    end
    
    def optionsShowToolbar()
        # This is all very cut and paste code for showing/hiding the
        # toolbar
        if @toolbarAction.isChecked()
            toolBar().show()
        else
            toolBar().hide()
        end
    end
    
    def optionsShowStatusbar()
        # This is all very cut and paste code for showing/hiding the
        # statusbar
        if @statusbarAction.isChecked()
            statusBar().show()
        else
            statusBar().hide()
        end
    end
    
    def optionsConfigureKeys()
        KDE::KeyDialog.configure(actionCollection())
    end
    
    def optionsConfigureToolbars()
        # use the standard toolbar editor
        saveMainWindowSettings(KDE::Global.config(), autoSaveGroup())
    end
    
    def newToolbarConfig()
        # This slot is called when user clicks "Ok" or "Apply" in the toolbar editor.
        # recreate our GUI, and re-apply the settings (e.g. "text under icons", etc.)
        createGUI(Dir.getwd + '/%{APPNAMELC}ui.rc')
    
        applyMainWindowSettings(KDE::Global.config(), autoSaveGroup())
    end
    
    def optionsPreferences()
        # popup some sort of preference dialog, here
        dlg = %{APPNAME}Preferences.new
        if dlg.exec()
            # redo your settings
        end
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
