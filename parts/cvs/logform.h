/****************************************************************************
** Form interface generated from reading ui file './logform.ui'
**
** Created: Wed Jun 18 16:14:26 2003
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef LOGFORM_H
#define LOGFORM_H

#include <qvariant.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QTextBrowser;
class QProcess;

class LogForm : public QWidget
{
    Q_OBJECT

public:
    LogForm( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~LogForm();

    QTextBrowser* contents;

public slots:
    virtual void init();
    virtual void destroy();
    virtual void start( const QString & workDir, const QString & fn );
    virtual void slotProcessExited();
    virtual void slotReadStdout();
    virtual void slotReadStderr();
    virtual void setText( const QString & text );
    virtual void linkClicked( const QString & link );

protected:
    QString pathName;
    QProcess* process;

    QVBoxLayout* LogFormLayout;

protected slots:
    virtual void languageChange();

};

#endif // LOGFORM_H
