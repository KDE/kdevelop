class %{APPNAMESC}Iface < KDE::DCOPObject

    k_dcop 'void openURL(QString)'

    def initialize(app)
        super("%{APPNAMESC}Iface")
        @app = app
    end
    
    def openURL(url)
        @app.openURL(url)
    end
end

