
#ifndef _SVNOPTIONSWIDGET_H_
#define _SVNOPTIONSWIDGET_H_

#include <qwidget.h>
#include "svnoptionswidgetbase.h"
/**
 * 
 * KDevelop Authors
 **/

class SvnPart;

class SvnOptionsWidget : public SvnOptionsWidgetBase
{
  Q_OBJECT

public:
  SvnOptionsWidget(SvnPart *widget, QWidget *parent, const char *name=0);
  ~SvnOptionsWidget();
  
public slots:
     void accept();

private:
    void readConfig();
    void storeConfig();
    
    SvnPart *m_part;
  
};

#endif
