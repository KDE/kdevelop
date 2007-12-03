require 'Qt4'
require '%{APPNAMELC}.rb'
require 'qrc_application.rb'

app = Qt::Application.new(ARGV)
mw = %{APPNAME}.new
mw.show
app.exec


