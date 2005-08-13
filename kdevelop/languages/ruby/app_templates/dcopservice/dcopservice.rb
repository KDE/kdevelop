
class %{APPNAME} < KDE::DCOPObject

    k_dcop 'QString string(int)',
           'QStringList list()',
           'void add(QString)',
           'bool remove(QString)',
           'bool exit()'

    def initialize() 
        super("serviceInterface")
        puts "Starting new service... "
        @list = []
    end
    
    def string(idx)
        return @list[idx]
    end
    
    def list()
        return @list
    end
    
    def add(arg)
        puts "Adding " + arg + " to the list"
        @list << arg
    end
    
    def remove(arg)
	    @list.delete(arg) ? true : false
    end
    
    def exit()
        $kapp.quit()
        return true
    end
end
