require 'Qt'
require '%{APPNAMELC}.rb'

a = Qt::Application.new( ARGV )
mw = %{APPNAME}.new
mw.caption = "%{APPNAME}"
mw.show
a.connect( a, SIGNAL('lastWindowClosed()'), a, SLOT('quit()') )
a.exec

