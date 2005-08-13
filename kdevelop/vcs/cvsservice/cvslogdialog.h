//
// C++ Interface: cvslogdialog
//
// Description:
//
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CVSLOGDIALOG_H
#define CVSLOGDIALOG_H

#include <kdialogbase.h>

class CvsJob_stub;
class CvsService_stub;
class CVSLogPage;

/**
Implementation for the form displaying 'cvs log' output.

@author KDevelop Authors
*/
class CVSLogDialog : public KDialogBase
{
    Q_OBJECT
public:
    CVSLogDialog( CvsService_stub *cvsService, QWidget *parent=0, const char *name=0, int flags=0 );
    virtual ~CVSLogDialog();

    void startLog( const QString &workDir, const QString &pathName );

private slots:
    void slotDiffRequested( const QString &pathName, const QString &revA, const QString &revB );
    virtual void slotCancel();

private:
//    void parseLogContent( const QString& text );
    void displayActionFeedback( bool working );

private:
    QString m_pathName;

    CVSLogPage *m_cvsLogPage;
    CvsService_stub *m_cvsService;
};

#endif
