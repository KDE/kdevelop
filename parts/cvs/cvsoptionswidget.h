
#ifndef _CVSOPTIONSWIDGET_H_
#define _CVSOPTIONSWIDGET_H_

#include <qwidget.h>
#include "cvsoptionswidgetbase.h"

/**
 *
 * KDevelop Authors
 **/

class CvsPart;
class ServerConfigurationWidget;
class QLabel;
class QLineEdit;
class QVBoxLayout;

class CvsOptionsWidget : public CvsOptionsWidgetBase
{
    Q_OBJECT
public:
    CvsOptionsWidget( QWidget *parent, const char *name=0 );
    ~CvsOptionsWidget();

    QString cvs() const;
    QString commit() const;
    QString update() const;
    QString add() const;
    QString remove() const;
    QString diff() const;
    QString log() const;
    QString rsh() const;

public slots:
    void accept();

    ServerConfigurationWidget *m_cfgWidget;

private:
    void readConfig();
    void storeConfig();
};

#endif
