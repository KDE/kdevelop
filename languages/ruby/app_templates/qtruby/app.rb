#!/usr/bin/env ruby

require 'Qt'

app = Qt::Application.new(ARGV)
widget = Qt::Widget.new()
widget.setGeometry(50, 500, 400, 400)

app.setMainWidget(widget)
widget.show()
app.exec()
