#ifndef PHPBOOKCONFIG_H
#define PHPBOOKCONFIG_H

#include "phpbookconfigwidget.h"
#include <qstring.h>
class PHPConfigData;

class PHPBookConfig : public PHPBookConfigWidget{ 
  Q_OBJECT
    public:
  PHPBookConfig( PHPConfigData* data,QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
  ~PHPBookConfig();
  
  public slots:
    virtual void slotOK();
  virtual void slotCancel();
  
 private:
  PHPConfigData* configData;
  
};

#endif
