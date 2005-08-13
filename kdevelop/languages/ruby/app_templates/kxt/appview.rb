
=begin
  This is the main view class for %{APPNAME}.  Most of the non-menu,
  non-toolbar, and non-statusbar (e.g., non frame) GUI code should go
  here.
 
  This %{APPNAMELC} uses an HTML component as an example.
 
  @short Main view
  @author %{AUTHOR} <%{EMAIL}>
  @version %{VERSION}
=end
class %{APPNAMESC}View < %{APPNAMESC}view_base

    #
    # Use this signal to change the content of the statusbar
    #
    signals 'signalChangeStatusbar(const QString&)'

    #
    # Use this signal to change the content of the caption
    #
    signals 'signalChangeCaption(const QString&)'

    slots 'switchColors()',
          'settingsChanged()'
   
    def initialize(parent)
        super(parent)
        
        settingsChanged()
    end
    
    def switchColors()
        color = Settings.instance.col_background
        Settings.instance.setCol_background( Settings.instance.col_foreground )
        Settings.instance.setCol_foreground( color )

        settingsChanged()
    end

    def settingsChanged()
        sillyLabel.setPaletteBackgroundColor( Settings.instance.col_background )
        sillyLabel.setPaletteForegroundColor( Settings.instance.col_foreground )
        # i18n : internationallization
        sillyLabel.setText( i18n("This project is %d days old" % Settings.instance.val_time) )
        
        emit signalChangeStatusbar( i18n("Settings changed") )
    end
        
    def slotSetTitle(title)
        emit signalChangeCaption(title)
    end

end
