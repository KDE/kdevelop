#ifndef __DISTPART_WIDGET_H__
#define __DISTPART_WIDGET_H__


#include <qwidget.h>
#include <qstring.h>
#include "distpart_ui.h"

class KDevProject;
class distpartPart;
class KParts::ReadOnlyPart;


class distpartWidget : public distpart_ui
{
  Q_OBJECT
    
public:
		  
  distpartWidget(distpartPart *part);
  ~distpartWidget();

};


#endif
