%{H_TEMPLATE}

#ifndef __%{APPNAMEUC}_WIDGET_H__
#define __%{APPNAMEUC}_WIDGET_H__


#include <qwidget.h>
#include <qstring.h>


class KDevProject;
class %{APPNAME}Part;


class %{APPNAME}Widget : public QWidget
{
  Q_OBJECT
    
public:
		  
  %{APPNAME}Widget(%{APPNAME}Part *part);
  ~%{APPNAME}Widget();

};


#endif
