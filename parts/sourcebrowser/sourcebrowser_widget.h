#ifndef __SOURCEBROWSER_WIDGET_H__
#define __SOURCEBROWSER_WIDGET_H__


#include <qwidget.h>
#include <qstring.h>


class KDevProject;
class SourceBrowserPart;
namespace KParts {
  class ReadOnlyPart;
}


class SourceBrowserWidget : public QWidget
{
  Q_OBJECT
    
public:
		  
  SourceBrowserWidget(SourceBrowserPart *part);
  ~SourceBrowserWidget();

};


#endif
