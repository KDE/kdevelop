#ifndef __UICHOOSER_WIDGET_H__
#define __UICHOOSER_WIDGET_H__


#include <qwidget.h>


#include "uichooser.h"

class UIChooserPart;
    
class UIChooserWidget : public UIChooser
{
  Q_OBJECT

public:

  UIChooserWidget(QWidget *parent=0, const char *name=0);
  void setPart(UIChooserPart* pPart);

private slots:

  void load();
  void save();

  void accept();

private:
  UIChooserPart* m_pMyPart;
};


#endif
