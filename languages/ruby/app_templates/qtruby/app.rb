#!/usr/bin/env ruby

require 'Qt'

app = Qt::Application.new(ARGV)
widget = Qt::Widget.new
widget.setGeometry(50, 500, 400, 400)
widget.caption = "Hello World!"

app.mainWidget = widget
widget.show
app.exec
