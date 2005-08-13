require 'Qt'
require '%{APPNAMELC}.rb'

a = Qt::Application.new( ARGV )
mw = %{APPNAMESC}.new
mw.caption = "%{APPNAMESC}"
mw.show
a.connect( a, SIGNAL('lastWindowClosed()'), a, SLOT('quit()') )
a.exec

