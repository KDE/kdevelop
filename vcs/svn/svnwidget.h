
#ifndef _SVNWIDGET_H_
#define _SVNWIDGET_H_

#include <qwidget.h>

#include "processwidget.h"

class SvnPart;

class SvnWidget : public ProcessWidget {
    Q_OBJECT


public:
    SvnWidget(SvnPart *part);
    ~SvnWidget();
    virtual void insertStdoutLine(const QString & line);
    
private slots:
    void lineHighlighted(int line);

private:

    SvnPart * m_part;
    QString dir;
};

#endif

