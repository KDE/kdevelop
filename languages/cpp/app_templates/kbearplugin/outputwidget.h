
#ifndef __%{APPNAMEUC}_OUTPUTWIDGET_H__
#define __%{APPNAMEUC}_OUTPUTWIDGET_H__

#include <qwidget.h>
#include <qstring.h>

class KBear%{APPNAME}OutputWidget : public QWidget
{
    Q_OBJECT
public:
    KBear%{APPNAME}OutputWidget( QWidget* parent = 0, const char* name = 0 );
    virtual ~KBear%{APPNAME}OutputWidget();
};


#endif // __%{APPNAMEUC}_OUTPUTWIDGET_H__

