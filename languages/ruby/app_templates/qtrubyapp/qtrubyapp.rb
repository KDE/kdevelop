class %{APPNAME} < Qt::MainWindow

    slots 'newDoc()',
          'choose()',
          'load( const QString& )',
          'save()',
          'saveAs()',
          'print()',
          'about()',
          'aboutQt()'
    
	def initialize()
        super( nil, "%{APPNAME}", WDestructiveClose )
        @printer = Qt::Printer.new
    
        fileTools = Qt::ToolBar.new( self, "file operations" )
        fileTools.setLabel( tr("File Operations") )
    
        openIcon = Qt::Pixmap.new( "fileopen.xpm" )
        fileOpen = Qt::ToolButton.new( Qt::IconSet.new(openIcon), tr("Open File"), nil,
                            self, SLOT('choose()'), fileTools, "open file" )
    
        saveIcon = Qt::Pixmap.new( "filesave.xpm" )
        fileSave = Qt::ToolButton.new( Qt::IconSet.new(saveIcon), tr("Save File"), nil,
                            self, SLOT('save()'), fileTools, "save file" )
    
        printIcon = Qt::Pixmap.new( "fileprint.xpm" )
        filePrint = Qt::ToolButton.new( Qt::IconSet.new(printIcon), tr("Print File"), nil,
                            self, SLOT('print()'), fileTools, "print file" )
    
    
        Qt::WhatsThis.whatsThisButton( fileTools )
    
        fileOpenText = tr('<p><img source="fileopen"> ' +
                    "Click this button to open a <em>new file</em>. <br>" +
                    "You can also select the <b>Open</b> command " +
                    "from the <b>File</b> menu.</p>")
    
        Qt::WhatsThis.add( fileOpen, fileOpenText )
    
        Qt::MimeSourceFactory.defaultFactory().setPixmap( "fileopen", openIcon )
    
        fileSaveText = tr("<p>Click this button to save the file you " +
                    "are editing. You will be prompted for a file name.\n" +
                    "You can also select the <b>Save</b> command " +
                    "from the <b>File</b> menu.</p>")
    
        Qt::WhatsThis.add( fileSave, fileSaveText )
    
        filePrintText = tr("Click this button to print the file you " +
                    "are editing.\n You can also select the Print " +
                    "command from the File menu.")
    
        Qt::WhatsThis.add( filePrint, filePrintText )
    
    
        file = Qt::PopupMenu.new( self )
        menuBar().insertItem( tr("&File"), file )
    
    
        file.insertItem( tr("&New"), self, SLOT('newDoc()'), Qt::KeySequence.new(CTRL+Key_N) )
    
        id = file.insertItem( Qt::IconSet.new(openIcon), tr("&Open..."),
                            self, SLOT('choose()'), Qt::KeySequence.new(CTRL+Key_O) )
        file.setWhatsThis( id, fileOpenText )
    
        id = file.insertItem( Qt::IconSet.new(saveIcon), tr("&Save"),
                            self, SLOT('save()'), Qt::KeySequence.new(CTRL+Key_S) )
        file.setWhatsThis( id, fileSaveText )
    
        id = file.insertItem( tr("Save &As..."), self, SLOT('saveAs()') )
        file.setWhatsThis( id, fileSaveText )
    
        file.insertSeparator()
    
        id = file.insertItem( Qt::IconSet.new(printIcon), tr("&Print..."),
                            self, SLOT('print()'), Qt::KeySequence.new(CTRL+Key_P) )
        file.setWhatsThis( id, filePrintText )
    
        file.insertSeparator()
    
        file.insertItem( tr("&Close"), self, SLOT('close()'), Qt::KeySequence.new(CTRL+Key_W) )
    
        file.insertItem( tr("&Quit"), $qApp, SLOT( 'closeAllWindows()' ), Qt::KeySequence.new(CTRL+Key_Q) )
    
        menuBar().insertSeparator()
    
        help = Qt::PopupMenu.new( self )
        menuBar().insertItem( tr("&Help"), help )
    
        help.insertItem( tr("&About"), self, SLOT('about()'), Qt::KeySequence.new(Key_F1) )
        help.insertItem( tr("About &Qt"), self, SLOT('aboutQt()') )
        help.insertSeparator()
        help.insertItem( tr("What's &This"), self, SLOT('whatsThis()'), Qt::KeySequence.new(SHIFT+Key_F1) )
    
        @e = Qt::TextEdit.new( self, "editor" )
        @e.setFocus()
        setCentralWidget( @e )
        statusBar().message( tr("Ready"), 2000 )
    
        resize( 450, 600 )
    end
       
    private
        
    def newDoc()
        ed = %{APPNAME}.new
        ed.setCaption(tr("Qt Example - Application"))
        ed.show()
    end
    
    def choose()
        fn = Qt::FileDialog.getOpenFileName( nil, nil,
                                                self)
        if !fn.nil?
            load( fn )
        else
            statusBar().message( tr("Loading aborted"), 2000 )
        end
    end
    
    
    def load( filename )
        f = Qt::File.new( filename )
        if !f.open( IO_ReadOnly )
            return
        end
    
        ts = Qt::TextStream.new( f )
        @e.setText( ts.read() )
        @e.setModified( false )
        setCaption( filename )
        statusBar().message( tr("Loaded document %s" % filename), 2000 )
    end
    
    
    def save()
        if @filename.nil? 
            saveAs()
            return
        end
    
        text = @e.text()
        f = Qt::File.new( @filename )
        if !f.open( IO_WriteOnly ) 
            statusBar().message( tr("Could not write to %s" % @filename),
                                2000 )
            return
        end
    
        t = Qt::TextStream.new( f )
        t << text
        f.close()
    
        @e.setModified( false )
    
        setCaption( @filename )
    
        statusBar().message( tr( "File %s saved" % @filename ), 2000 )
    end
    
    
    def saveAs()
        fn = Qt::FileDialog.getSaveFileName( nil, nil,
                                                self )
        if !fn.nil? 
            @filename = fn
            save()
        else
            statusBar().message( tr("Saving aborted"), 2000 )
        end
    end
    
    
    def print()
        # ###### Rewrite to use Qt::SimpleRichText to print here as well
        margin = 10
        pageNo = 1
    
        if @printer.setup(self)                # @printer dialog
            statusBar().message( tr("Printing...") )
            p = Qt::Painter.new
            if !p.begin( @printer )                # paint on @printer
                return
            end
    
            p.setFont( @e.font() )
            yPos        = 0                        # y-position for each line
            fm = p.fontMetrics()
            metrics = Qt::PaintDeviceMetrics.new( @printer ) # need width/height
                                                    # of @printer surface
            for i in 0...@e.lines() do
                if margin + yPos > metrics.height() - margin 
                    msg = "Printing (page "
                    msg += pageNo.to_s
                    pageNo += 1
                    msg += ")..."
                    statusBar().message( msg )
                    @printer.newPage()                # no more room on self page
                    yPos = 0                        # back to top of page
                end
                p.drawText( margin, margin + yPos,
                            metrics.width(), fm.lineSpacing(),
                            ExpandTabs | DontClip,
                            @e.text( i ) )
                yPos = yPos + fm.lineSpacing()
            end
            p.end()                                # send job to @printer
            statusBar().message( tr("Printing completed"), 2000 )
        else
            statusBar().message( tr("Printing aborted"), 2000 )
        end
    end
    
    protected

    def closeEvent( ce )
        if !@e.modified? 
            ce.accept()
            return
        end
    
        case Qt::MessageBox.information( self, tr("Qt Application Example"),
                                        tr("Do you want to save the changes" +
                                        " to the document?"),
                                        tr("Yes"), tr("No"), tr("Cancel"),
                                        0, 1 ) 
        when 0
            save()
            ce.accept()
        when 1
            ce.accept()
        when 2
            ce.ignore()
        else # just for sanity
            ce.ignore()
        end
    end
    
    private
    
    def about()
        Qt::MessageBox.about( self, tr("Qt Application Example"),
                            tr("This example demonstrates simple use of " +
                            "Qt::MainWindow,\nQt::MenuBar and Qt::ToolBar."))
    end
    
    
    def aboutQt()
        Qt::MessageBox.aboutQt( self, tr("Qt Application Example") )
    end

end
