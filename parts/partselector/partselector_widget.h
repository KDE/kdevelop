#ifndef __PARTSELECTOR_WIDGET_H__
#define __PARTSELECTOR_WIDGET_H__


#include <qlist.h>


#include "partselector.h"


class KDevPart;


class PartSelectorWidget : public PartSelector
{
  Q_OBJECT
    
public:
		  
  PartSelectorWidget(QWidget *parent=0, const char *name=0);
  ~PartSelectorWidget();


public slots:
      
  void accept();


private:

  QList<KDevPart> _parts;

};


#endif
