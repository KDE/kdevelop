
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
    
public slots:
    void startCommand(const QString &dir, const QString &command);

private slots:
    void lineHighlighted(int line);

private:
    virtual void insertStdoutLine(const QString & line);
    virtual void childFinished(bool normal, int status);

    SvnPart * m_part;
    QString dir;
};

#endif

