#ifndef __UICHOOSER_WIDGET_H__
#define __UICHOOSER_WIDGET_H__


#include <qwidget.h>


#include "uichooser.h"

class UIChooserPart;
    
class UIChooserWidget : public UIChooser
{
  Q_OBJECT

public:
  UIChooserWidget( UIChooserPart * part, QWidget *parent=0, const char *name=0 );

public slots:
  void accept();
  
protected slots:
  void maybeEnableCloseOnHover( bool );

private slots:

  void load();
  void save();

private:
  UIChooserPart* m_part;
  QWidget * _lastMode;
};


#endif
