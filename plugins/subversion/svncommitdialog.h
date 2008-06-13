/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SVN_COMMITDIALOG_H
#define SVN_COMMITDIALOG_H

#include "ui_commitdialog.h"

#include <kdialog.h>

class KDevSvnPlugin;
namespace KDevelop
{
    class VcsFileInfo;
}

class SvnCommitDialog : public KDialog
{
    Q_OBJECT
public:
    SvnCommitDialog( KDevSvnPlugin *part, QWidget *parent = 0 );
    virtual ~SvnCommitDialog();
    void setCommitCandidates( const KUrl::List &list );
    void setMessage( const QString& );
    void setKeepLocks( bool );
    void setRecursive( bool );
    void setOldMessages( const QStringList& );
    KUrl::List checkedUrls() const;
    bool recursive() const;
    bool keepLocks() const;
    QString message() const;
signals:
    void okClicked( SvnCommitDialog* dlg );
    void cancelClicked( SvnCommitDialog* dlg );
private slots:
    void ok();
    void cancel();
private:
    bool eventFilter( QObject* o, QEvent* e );
//     void insertRow( const KDevelop::VcsFileInfo &info );
    void insertRow( const QString& state, const KUrl& url );

    KDevSvnPlugin *m_part;
    Ui::SvnCommitDialog ui;
};

#endif
