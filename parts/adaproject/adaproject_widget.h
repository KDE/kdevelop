#ifndef __ADAPROJECT_WIDGET_H__
#define __ADAPROJECT_WIDGET_H__

#include <qwidget.h>
#include <qstring.h>

class KDevProject;
class AdaProjectPart;

class AdaProjectWidget : public QWidget
{
  Q_OBJECT
    
public:
		  
  AdaProjectWidget(AdaProjectPart *part);
  ~AdaProjectWidget();

};

#endif
