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

#include "svn_commitwidgets.h"
#include "subversion_part.h"
#include "svn_models.h"
extern "C" {
#include <svn_auth.h>
#include <svn_client.h>
}
#include <ktextedit.h>
#include <klocale.h>

#include <QFileInfo>

using namespace KDevelop;

class SvnCommitLogInputDlgPrivate
{
public:
    apr_array_header_t *m_commit_items;
    Ui::SvnCommitLogInputDlg ui;
};

SvnCommitLogInputDlg::SvnCommitLogInputDlg( QWidget *parent )
    : KDialog( parent ), d( new SvnCommitLogInputDlgPrivate )
{
    QWidget *widget = new QWidget(this);
    d->ui.setupUi( widget );
    setMainWidget( widget );
    setButtons( KDialog::Ok | KDialog::Cancel );
//     m_commit_items = commit_items;
    setWindowTitle( i18n("Enter Subversion Commit Log Message") );
}
void SvnCommitLogInputDlg::setCommitItems( apr_array_header_t *cis )
{
    for( int i = 0; i < cis->nelts; i++ ){
        svn_client_commit_item_t *item = ((svn_client_commit_item_t **) cis->elts)[i];

        QString path( item->path );
        if( path.isEmpty() ) path = ".";

        char text_mod = '_', prop_mod = '_';
        if ((item->state_flags & SVN_CLIENT_COMMIT_ITEM_DELETE) && (item->state_flags & SVN_CLIENT_COMMIT_ITEM_ADD))
            text_mod = 'R';
        else if (item->state_flags & SVN_CLIENT_COMMIT_ITEM_ADD)
            text_mod = 'A';
        else if (item->state_flags & SVN_CLIENT_COMMIT_ITEM_DELETE)
            text_mod = 'D';
        else if (item->state_flags & SVN_CLIENT_COMMIT_ITEM_TEXT_MODS)
            text_mod = 'M';
        else if (item->state_flags & SVN_CLIENT_COMMIT_ITEM_PROP_MODS)
            prop_mod = 'M';
        else if (item->state_flags & SVN_CLIENT_COMMIT_ITEM_IS_COPY)
            prop_mod = 'C';
        else if (item->state_flags & SVN_CLIENT_COMMIT_ITEM_LOCK_TOKEN)
            prop_mod = 'L';

        QString oneRow;
        oneRow = oneRow + text_mod + ' ' + prop_mod + ' ' + path;
        listWidget->addItem( oneRow );
    }
}
QString SvnCommitLogInputDlg::message()
{
    return messageBox->toPlainText();
}

//////////////////////////////////////////////////////////////////////

class SvnCommitOptionDlgPrivate
{
public:
    KDevSubversionPart *m_part;
    Ui::SvnCommitOptionDlg ui;
};

SvnCommitOptionDlg::SvnCommitOptionDlg( KDevSubversionPart *part, QWidget *parent )
    : KDialog( parent ), d( new SvnCommitOptionDlgPrivate )
{
    d->m_part = part;

    QWidget *widget = new QWidget(this);
    d->ui.setupUi( widget );
    setMainWidget( widget );
    setButtons( KDialog::Ok | KDialog::Cancel );

    d->ui.treeWidget->resizeColumnToContents(0);
    d->ui.treeWidget->resizeColumnToContents(1);
}
SvnCommitOptionDlg::~SvnCommitOptionDlg()
{}

void SvnCommitOptionDlg::setCommitCandidates( const KUrl::List &urls )
{
    for( QList<KUrl>::const_iterator it = urls.begin(); it != urls.end(); ++it ){
        KUrl url(*it);
        QFileInfo fileInfo(url.toLocalFile());

        if( fileInfo.isFile() ){
            QList<SvnStatusHolder> holderList = d->m_part->statusSync( url, false, true, false ).values();
//             SvnKJobBase *statJob = m_part->svncore()->createStatusJob( url, workingRev,
//                             false, true, false, true, false );
            if( holderList.count() < 1 ){
                insertRow( "unknown", url );
            }
            else{
                if( holderList.at(0).wcPath == url.toLocalFile() ){
                    insertRow( SvnStatusHolder::statusToString( holderList.at(0).textStatus ), url );
                }
                else{
                    insertRow( "unknown", url );
                }
            }
        }// end of isFile()

        else if( fileInfo.isDir() ){
//             const QList<VcsFileInfo> vcslist =
//                     m_part->statusSync( url, IVersionControl::Recursive );
            QList<SvnStatusHolder> holderList = d->m_part->statusSync( url, true, false, false ).values();

            Q_FOREACH( SvnStatusHolder _holder, holderList ) {

                if( _holder.textStatus == svn_wc_status_added ||
                    _holder.textStatus == svn_wc_status_deleted ||
                    _holder.textStatus == svn_wc_status_modified ||
                    _holder.textStatus == svn_wc_status_replaced ||
                    _holder.propStatus == svn_wc_status_added ||
                    _holder.propStatus == svn_wc_status_deleted ||
                    _holder.propStatus == svn_wc_status_modified ||
                    _holder.propStatus == svn_wc_status_replaced ){

                        insertRow( SvnStatusHolder::statusToString( _holder.textStatus ), url );

                    }
            }

        }// end of isDIr()

        else if( !fileInfo.exists() ){
            // maybe delete file
//             this->insertItem( VCSFileInfo::state2String( VCSFileInfo::Deleted ), oneUrl );
            insertRow( "deleted", url );
        }

        else{
            // should not reach here
        }
    }
}
KUrl::List SvnCommitOptionDlg::checkedUrls()
{
    KUrl::List list;

    QTreeWidgetItemIterator it( d->ui.treeWidget, QTreeWidgetItemIterator::Checked );
    for( ; *it; ++it ){
        QString path = (*it)->text( 2 );
        list << path;
    }
    return list;
}

bool SvnCommitOptionDlg::recursive()
{
    return d->ui.recursiveChk->isChecked();
}

bool SvnCommitOptionDlg::keepLocks()
{
    return d->ui.keepLocksChk->isChecked();
}

// void SvnCommitOptionDlg::insertRow( const KDevelop::VcsFileInfo &info ) // {
//     QStringList strings;
//     strings << " " << VcsFileInfo::state2String(info.state()) << info.filePath().prettyUrl();
//     QTreeWidgetItem *item = new QTreeWidgetItem( treeWidget, strings );
//     item->setCheckState(0, Qt::Checked);
// }

// TODO display property status
void SvnCommitOptionDlg::insertRow( const QString& state, const KUrl& url )
{
    QStringList strings;
    strings << " " << state << url.prettyUrl();
    QTreeWidgetItem *item = new QTreeWidgetItem( d->ui.treeWidget, strings );
    item->setCheckState(0, Qt::Checked);
}

#include "svn_commitwidgets.moc"
