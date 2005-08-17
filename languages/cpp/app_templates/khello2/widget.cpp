%{CPP_TEMPLATE}

#include <qlabel.h>

#include "%{APPNAMELC}widget.h"

%{APPNAME}Widget::%{APPNAME}Widget(QWidget* parent, const char* name, Qt::WFlags fl)
        : %{APPNAME}WidgetBase(parent,name,fl)
{}

%{APPNAME}Widget::~%{APPNAME}Widget()
{}

/*$SPECIALIZATION$*/
void %{APPNAME}Widget::button_clicked()
{
    if ( label->text().isEmpty() )
    {
        label->setText( "Hello World!" );
    }
    else
    {
        label->clear();
    }
}



#include "%{APPNAMELC}widget.moc"

