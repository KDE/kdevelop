#ifndef PHPCONFIGWIDGET_H
#define PHPCONFIGWIDGET_H

#include "phpconfigwidgetbase.h"
#include <qstring.h>
class KProcess;
class PHPConfigData;

class PHPConfigWidget : public PHPConfigWidgetBase
{ 
    Q_OBJECT

public:
    PHPConfigWidget( PHPConfigData* data,QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0 );
    ~PHPConfigWidget();

public slots:
    virtual void slotZendButtonClicked();
    virtual void slotAboutClicked();
    virtual void slotPHPExeButtonClicked();
    virtual void slotPHPIniButtonClicked();
    void accept();
    void slotReceivedPHPInfo (KProcess* proc, char* buffer, int buflen);
    
 private:
    QDomDocument* dom;
    QString m_phpInfo;
    PHPConfigData* configData;

};

#endif
