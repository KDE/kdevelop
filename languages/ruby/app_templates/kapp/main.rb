#!/usr/bin/env ruby

require 'Korundum'

require '%{APPNAMELC}iface.rb'
require '%{APPNAMELC}.rb'
require '%{APPNAMELC}view.rb'
require 'pref.rb'

description = I18N_NOOP("A KDE Application")

version = "%{VERSION}"

options = [
    [ "+[URL]", I18N_NOOP( "Document to open" ), "" ] ]



about = KDE::AboutData.new("%{APPNAMELC}", I18N_NOOP("%{APPNAME}"), version, description,
                     KDE::AboutData.License_%{LICENSE}, "(C) %{YEAR} %{AUTHOR}", nil, nil, "%{EMAIL}")
about.addAuthor( "%{AUTHOR}", nil, "%{EMAIL}" )
KDE::CmdLineArgs.init(ARGV, about)
KDE::CmdLineArgs.addCmdLineOptions(options)
app = KDE::Application.new

# register ourselves as a dcop client
app.dcopClient().registerAs(app.name, false)

# see if we are starting with session management
if app.restored?
    RESTORE(%{APPNAME})
else
    # no session.. just start up normally
    args = KDE::CmdLineArgs.parsedArgs
    if args.count == 0
        widget = %{APPNAME}.new
        widget.show
    else
	    for i in 0...args.count do
            widget = %{APPNAME}.new
            widget.show
            widget.load(args.url(i))
        end
    end
    args.clear
end

app.exec

