#ifndef __PASCALPROJECT_WIDGET_H__
#define __PASCALPROJECT_WIDGET_H__


#include <qwidget.h>
#include <qstring.h>


class KDevProject;
class PascalProjectPart;


class PascalProjectWidget : public QWidget
{
  Q_OBJECT
    
public:
		  
  PascalProjectWidget(PascalProjectPart *part);
  ~PascalProjectWidget();

};


#endif
