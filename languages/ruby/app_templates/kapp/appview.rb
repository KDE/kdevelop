
=begin
  This is the main view class for %{APPNAME}.  Most of the non-menu,
  non-toolbar, and non-statusbar (e.g., non frame) GUI code should go
  here.
 
  This %{APPNAMELC} uses an HTML component as an example.
 
  @short Main view
  @author %{AUTHOR} <%{EMAIL}>
  @version %{VERSION}
=end
class %{APPNAME}View < Qt::Widget

    #
    # Use this signal to change the content of the statusbar
    #
    signals 'signalChangeStatusbar(const QString&)'

    #
    # Use this signal to change the content of the caption
    #
    signals 'signalChangeCaption(const QString&)'

    slots 'slotOnURL(const QString&)',
          'slotSetTitle(const QString&)'
   
    def initialize(parent)
        super(parent)
        
        # keep a reference to the DCOP Interface so it doesn't get gc'd
        @dcop = %{APPNAME}Iface.new(self)
        
        # setup our layout manager to automatically add our widgets
        top_layout = Qt::HBoxLayout.new(self)
        top_layout.setAutoAdd(true)
    
        # we want to look for all components that satisfy our needs.  the
        # trader will actually search through *all* registered KDE
        # applications and components -- not just KParts.  So we have to
        # specify two things: a service type and a constraint
        #
        # the service type is like a mime type.  we say that we want all
        # applications and components that can handle HTML -- 'text/html'
        #
        # however, by itself, this will return such things as Netscape..
        # not what we wanted.  so we constrain it by saying that the
        # string 'KParts/ReadOnlyPart' must be found in the ServiceTypes
        # field.  with this, only components of the type we want will be
        # returned.
        offers = KDE::Trader.self().query("text/html", "'KParts/ReadOnlyPart' in ServiceTypes")
    
        factory = nil
        # in theory, we only care about the first one.. but let's try all
        # offers just in when the first can't be loaded for some reason
        offers.each do |ptr|
            # we now know that our offer can handle HTML and is a part.
            # since it is a part, it must also have a library... let's try to
            # load that now
            factory = KDE::LibLoader.self().factory( ptr.library() )
            if ! factory.nil?
                @html = factory.create(self, ptr.name(), "KParts::ReadOnlyPart")
                break
            end
        end
    
        # if our factory is invalid, then we never found our component
        # and we might as well just exit now
        if factory.nil?
            KDE::MessageBox.error(self, i18n("Could not find a suitable HTML component"))
            return
        end
    
        connect(@html, SIGNAL('setWindowCaption(const QString&)'),
                self,   SLOT('slotSetTitle(const QString&)'))
        connect(@html, SIGNAL('setStatusBarText(const QString&)'),
                self,   SLOT('slotOnURL(const QString&)'))
    
    end
    
    
    def print(p, height, width)
        # do the actual printing, here
        # p.drawText(etc..)
    end
    
    def currentURL()
        return @html.url().url()
    end
    
    def openURL(url)
        @html.openURL(KDE::URL.new(url))
    end
    
    def slotOnURL(url)
        emit signalChangeStatusbar(url)
    end
    
    def slotSetTitle(title)
        emit signalChangeCaption(title)
    end

end
