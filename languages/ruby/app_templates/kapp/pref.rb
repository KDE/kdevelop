
class %{APPNAME}Preferences < KDE::DialogBase

    def initialize()
        super(TreeList, i18n("%{APPNAME} Preferences"),
                    Help|Default|Ok|Apply|Cancel, Ok)
        # this is the base class for your preferences dialog.  it is now
        # a Treelist dialog.. but there are a number of other
        # possibilities (including Tab, Swallow, and just Plain)
        frame = addPage(i18n("First Page"), i18n("Page One Options"))
        @pageOne = %{APPNAME}PrefPageOne.new(frame)
    
        frame = addPage(i18n("Second Page"), i18n("Page Two Options"))
        @pageTwo = %{APPNAME}PrefPageTwo.new(frame)
    end
end

class %{APPNAME}PrefPageOne < Qt::Frame

    def initialize(parent)
        super(parent)
        layout = Qt::HBoxLayout.new(self)
        layout.setAutoAdd(true)
    
        Qt::Label.new(i18n("Add something here"), self)
    end
end

class %{APPNAME}PrefPageTwo < Qt::Frame

    def initialize(parent)
        super(parent)
        layout = Qt::HBoxLayout.new(self)
        layout.setAutoAdd(true)
    
        Qt::Label.new(i18n("Add something here"), self)
    end
end
