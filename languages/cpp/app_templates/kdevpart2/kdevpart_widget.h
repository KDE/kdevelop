%{H_TEMPLATE}
#ifndef %{APPNAMEUC}_WIDGET_H
#define %{APPNAMEUC}_WIDGET_H

#include <qwidget.h>
#include <qstring.h>

class KDevProject;
class %{APPNAME}Part;

class %{APPNAME}Widget: public QWidget
{
    Q_OBJECT
public:  
    %{APPNAME}Widget(%{APPNAME}Part *part);
    ~%{APPNAME}Widget();

private:
    %{APPNAME}Part *m_part;
};


#endif
