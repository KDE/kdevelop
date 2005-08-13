require 'Korundum'
require '%{APPNAMELC}.rb'

description = I18N_NOOP("A KDE DCOP Application")
version = "0.1"
options = []

KDE::Locale.mainCatalogue = "kdelibs"
aboutdata = KDE::AboutData.new("%{APPNAMELC}", I18N_NOOP("KDE"),
                                version, description,
                                KDE::AboutData.License_GPL, "(C) %{YEAR}, %{AUTHOR}")
aboutdata.addAuthor("%{AUTHOR}",I18N_NOOP("Developer"),"%{EMAIL}")

KDE::CmdLineArgs.init( ARGV, aboutdata )
KDE::CmdLineArgs.addCmdLineOptions( options )
KDE::UniqueApplication.addCmdLineOptions()

if !KDE::UniqueApplication.start
    puts "%{APPNAMELC} is already running!"
    exit(0)
end

app = KDE::UniqueApplication.new
puts "starting %{APPNAMELC} "
# This app is started automatically, no need for session management
app.disableSessionManagement
service = %{APPNAME}.new
puts "starting %{APPNAMELC} "
app.exec

