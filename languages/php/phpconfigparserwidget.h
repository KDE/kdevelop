#ifndef PHPCONFIGPARSERWIDGET_H
#define PHPCONFIGPARSERWIDGET_H

#include "phpconfigparserwidgetbase.h"
#include <qstring.h>
class KProcess;
class PHPConfigData;

class PHPConfigParserWidget : public PHPConfigParserWidgetBase
{ 
    Q_OBJECT

public:
    PHPConfigParserWidget( PHPConfigData* data,QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~PHPConfigParserWidget();
    public slots:
    void accept();
 private:
    PHPConfigData* m_configData;
};

#endif
