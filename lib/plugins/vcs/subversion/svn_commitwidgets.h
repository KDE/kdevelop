/***************************************************************************
 *   Copyright (C) 2007 by Dukju Ahn                                       *
 *   dukjuahn@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SVN_COMMITWIDGETS_H
#define SVN_COMMITWIDGETS_H

#include "ui_svncommitloginputdlg.h"
#include "ui_uicommit_option_dlg.h"
#include <apr_tables.h>

class KDevSubversionPart;
namespace KDevelop
{
    class VcsFileInfo;
}

class SvnCommitLogInputDlg : public KDialog, public Ui::SvnCommitLogInputDlg
{
//     Q_OBJECT
    public:
        SvnCommitLogInputDlg( QWidget *parent = 0 );
        virtual ~SvnCommitLogInputDlg(){};

        void setCommitItems( apr_array_header_t *cis );
        QString message();

    private:
        apr_array_header_t *m_commit_items;
};

class SvnCommitOptionDlg : public KDialog, public Ui::SvnCommitOptionDlg
{
    Q_OBJECT
public:
    SvnCommitOptionDlg( KDevSubversionPart *part, QWidget *parent );
    virtual ~SvnCommitOptionDlg();
    void setCommitCandidates( const KUrl::List &list );
    KUrl::List checkedUrls();
    bool recursive();
    bool keepLocks();

public slots:
    int exec();
private:
    void insertRow( const KDevelop::VcsFileInfo &info );
    void insertRow( const QString& state, const KUrl& url );
    KDevSubversionPart *m_part;
};

#endif
