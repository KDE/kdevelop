
class %{APPNAME} < Qt::MainWindow

    slots :newFile,
          :open,
          :save,
          :saveAs,
          :about,
          :documentWasModified

    def initialize
        super
        @textEdit = Qt::TextEdit.new
        setCentralWidget(@textEdit)
      
        createActions()
        createMenus()
        createToolBars()
        createStatusBar()
      
        readSettings()
      
        connect(@textEdit.document, SIGNAL(:contentsChanged),
                self, SLOT(:documentWasModified))
      
        setCurrentFile("")
    end

    def closeEvent(event)
        if maybeSave
            writeSettings
            event.accept
        else
            event.ignore
        end
    end

    def newFile()
        if maybeSave
            @textEdit.clear
            setCurrentFile("")
        end
    end

    def open()
        if maybeSave
            fileName = Qt::FileDialog.getOpenFileName(self)
            if !fileName.nil?
                loadFile(fileName)
            end
        end
    end

    def save()
        if @curFile.nil?
            return saveAs()
        else
            return saveFile(@curFile);
        end
    end

    def saveAs()
        fileName = Qt::FileDialog.getSaveFileName(self)
        if fileName.nil?
            return false
        end

        return saveFile(fileName)
    end

    def about()
        Qt::MessageBox.about(self, tr("About Application"),
            tr("The <b>Application</b> example demonstrates how to " \
                  "write modern GUI applications using Qt, with a menu bar, " \
                  "toolbars, and a status bar."))
    end

    def documentWasModified()
        setWindowModified(true)
    end

    def createActions()
        @newAct = Qt::Action.new(Qt::Icon.new(":/filenew.xpm"), tr("&New"), self)
        @newAct.shortcut = tr("Ctrl+N")
        @newAct.statusTip = tr("Create a new file")
        connect(@newAct, SIGNAL(:triggered), self, SLOT(:newFile))

        @openAct = Qt::Action.new(Qt::Icon.new(":/fileopen.xpm"), tr("&Open..."), self)
        @openAct.shortcut = tr("Ctrl+O")
        @openAct.statusTip = tr("Open an existing file")
        connect(@openAct, SIGNAL(:triggered), self, SLOT(:open))

        @saveAct = Qt::Action.new(Qt::Icon.new(":/filesave.xpm"), tr("&Save"), self)
        @saveAct.shortcut = tr("Ctrl+S")
        @saveAct.statusTip = tr("Save the document to disk")
        connect(@saveAct, SIGNAL(:triggered), self, SLOT(:save))

        @saveAsAct = Qt::Action.new(tr("Save &As..."), self)
        @saveAsAct.statusTip = tr("Save the document under a new name")
        connect(@saveAsAct, SIGNAL(:triggered), self, SLOT(:saveAs))

        @exitAct = Qt::Action.new(tr("E&xit"), self)
        @exitAct.shortcut = tr("Ctrl+Q")
        @exitAct.statusTip = tr("Exit the application")
        connect(@exitAct, SIGNAL(:triggered), self, SLOT(:close))

        @cutAct = Qt::Action.new(Qt::Icon.new(":/editcut.xpm"), tr("Cu&t"), self)
        @cutAct.shortcut = tr("Ctrl+X")
        @cutAct.statusTip = tr("Cut the current selection's contents to the " \
                                "clipboard")
        connect(@cutAct, SIGNAL(:triggered), @textEdit, SLOT(:cut))

        @copyAct = Qt::Action.new(Qt::Icon.new(":/editcopy.xpm"), tr("&Copy"), self)
        @copyAct.shortcut = tr("Ctrl+C")
        @copyAct.statusTip = tr("Copy the current selection's contents to the " \
                              "clipboard")
        connect(@copyAct, SIGNAL(:triggered), @textEdit, SLOT(:copy))

        @pasteAct = Qt::Action.new(Qt::Icon.new(":/editpaste.xpm"), tr("&Paste"), self)
        @pasteAct.shortcut = tr("Ctrl+V")
        @pasteAct.statusTip = tr("Paste the clipboard's contents into the current " \
                                "selection")
        connect(@pasteAct, SIGNAL(:triggered), @textEdit, SLOT(:paste))

        @aboutAct = Qt::Action.new(tr("&About"), self)
        @aboutAct.statusTip = tr("Show the application's About box")
        connect(@aboutAct, SIGNAL(:triggered), self, SLOT(:about))

        @aboutQtAct = Qt::Action.new(tr("About &Qt"), self)
        @aboutQtAct.statusTip = tr("Show the Qt library's About box")
        connect(@aboutQtAct, SIGNAL(:triggered), $qApp, SLOT(:aboutQt))

        @cutAct.setEnabled(false)
        @copyAct.setEnabled(false)
        connect(@textEdit, SIGNAL('copyAvailable(bool)'),
                @cutAct, SLOT('setEnabled(bool)'))
        connect(@textEdit, SIGNAL('copyAvailable(bool)'),
                @copyAct, SLOT('setEnabled(bool)'))
    end

    def createMenus()
        @fileMenu = menuBar().addMenu(tr("&File"))
        @fileMenu.addAction(@newAct)
        @fileMenu.addAction(@openAct)
        @fileMenu.addAction(@saveAct)
        @fileMenu.addAction(@saveAsAct)
        @fileMenu.addSeparator()
        @fileMenu.addAction(@exitAct)

        @editMenu = menuBar.addMenu(tr("&Edit"))
        @editMenu.addAction(@cutAct)
        @editMenu.addAction(@copyAct)
        @editMenu.addAction(@pasteAct)

        menuBar.addSeparator()

        @helpMenu = menuBar().addMenu(tr("&Help"))
        @helpMenu.addAction(@aboutAct)
        @helpMenu.addAction(@aboutQtAct)
    end

    def createToolBars()
        @fileToolBar = addToolBar(tr("File"))
        @fileToolBar.addAction(@newAct)
        @fileToolBar.addAction(@openAct)
        @fileToolBar.addAction(@saveAct)

        @editToolBar = addToolBar(tr("Edit"))
        @editToolBar.addAction(@cutAct)
        @editToolBar.addAction(@copyAct)
        @editToolBar.addAction(@pasteAct)
    end

    def createStatusBar()
        statusBar.showMessage(tr("Ready"))
    end

    def readSettings()
        settings = Qt::Settings.new("Trolltech", "Application Example")
        pos = settings.value("pos", Qt::Variant.new(Qt::Point.new(200, 200))).toPoint()
        size = settings.value("size", Qt::Variant.new(Qt::Size.new(400, 400))).toSize()
        resize(size)
        move(pos)
    end

    def writeSettings()
        settings = Qt::Settings.new("Trolltech", "Application Example")
        settings.setValue("pos", Qt::Variant.new(pos()))
        settings.setValue("size", Qt::Variant.new(size()))
    end

    def maybeSave()
        if @textEdit.document.modified?
            ret = Qt::MessageBox.warning(self, tr("Application"),
                        tr("The document has been modified.\n" \
                        "Do you want to save your changes?"),
                        Qt::MessageBox::Yes | Qt::MessageBox::Default,
                        Qt::MessageBox::No,
                        Qt::MessageBox::Cancel | Qt::MessageBox::Escape)
            if ret == Qt::MessageBox::Yes
                return save()
            elsif ret == Qt::MessageBox::Cancel
                return false
            end
        end
        return true
    end

    def loadFile(fileName)
        file = Qt::File.new(fileName)
        if !file.open(Qt::File::ReadOnly | Qt::File::Text)
            Qt::MessageBox.warning( self, tr("Application"),
                                    tr("Cannot read file %s\n%s." % [fileName, file.errorString]) )
            return
        end

        inf = Qt::TextStream.new(file)
        Qt::Application.overrideCursor = Qt::Cursor.new(Qt::WaitCursor)
        @textEdit.setPlainText(inf.readAll)
        Qt::Application.restoreOverrideCursor()

        setCurrentFile(fileName)
        statusBar.showMessage(tr("File loaded"), 2000)
    end

    def saveFile(fileName)
        file = Qt::File.new(fileName)
        if !file.open(Qt::File::WriteOnly | Qt::File::Text)
            Qt::MessageBox.warning(self, tr("Application"),
                                    tr("Cannot write file %s\n%s." % [fileName, file.errorString]) )
            return false
        end

        outf = Qt::TextStream.new(file)
        Qt::Application.overrideCursor = Qt::Cursor.new(Qt::WaitCursor)
        outf << @textEdit.toPlainText()
        Qt::Application.restoreOverrideCursor()
        outf.flush

        setCurrentFile(fileName)
        statusBar().showMessage(tr("File saved"), 2000)
        return true
    end

    def setCurrentFile(fileName)
        @curFile = fileName;
        @textEdit.document().modified = false
        setWindowModified(false)

        shownName = ""
        if @curFile.nil?
            shownName = "untitled.txt"
        else
            shownName = strippedName(@curFile)
        end

        setWindowTitle(tr("%s[*] - %s" % [shownName, tr("Application")]))
    end

    def strippedName(fullFileName)
        return Qt::FileInfo.new(fullFileName).fileName()
    end
end