class %{APPNAME}Iface < KDE::DCOPObject

    k_dcop 'void openURL(QString)'

    def initialize(app)
        super("%{APPNAME}Iface")
        @app = app
    end
    
    def openURL(url)
        @app.openURL(url)
    end
end

