
#ifndef _CVSWIDGET_H_
#define _CVSWIDGET_H_

#include <qwidget.h>

#include "processwidget.h"

class CvsPart;

class CvsWidget : public ProcessWidget {
    Q_OBJECT


public:
    CvsWidget(CvsPart *part);
    ~CvsWidget();
    
public slots:
    void startCommand(QString &dir, QString &command);

private slots:
    void lineHighlighted(int line);

private:
    virtual void insertStdoutLine(const QString & line);

    CvsPart * m_part;
    QString dir;
};

#endif

