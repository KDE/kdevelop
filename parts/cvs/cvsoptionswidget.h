
#ifndef _CVSOPTIONSWIDGET_H_
#define _CVSOPTIONSWIDGET_H_

#include <qwidget.h>
#include "cvsoptionswidgetbase.h"
/**
 * 
 * KDevelop Authors
 **/

class CvsPart;

class CvsOptionsWidget : public CvsOptionsWidgetBase
{
  Q_OBJECT

public:
  CvsOptionsWidget(CvsPart *widget, QWidget *parent, const char *name=0);
  ~CvsOptionsWidget();
  
public slots:
     void accept();

private:
    void readConfig();
    void storeConfig();
    
    CvsPart *m_part;
  
};

#endif
