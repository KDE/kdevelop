require 'Korundum'
include KDE

about = AboutData.new("%{APPNAMELC}_client", "%{APPNAME} client", "0.1")
CmdLineArgs.init(ARGV, about)

app = Application.new

# get our DCOP client and attach so that we may use it
client = app.dcopClient
client.attach

# do a 'send' for now
dcopRef = DCOPRef.new("%{APPNAMELC}", "%{APPNAME}Iface")
dcopRef.openURL("http://www.kde.org")

app.exec

