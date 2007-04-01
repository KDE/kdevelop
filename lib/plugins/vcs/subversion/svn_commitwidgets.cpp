#include "svn_commitwidgets.h"
#include "subversion_part.h"
#include <iversioncontrol.h>
#include <svn_auth.h>
#include <svn_client.h>
#include <ktextedit.h>
#include <klocale.h>

#include <QFileInfo>

using namespace KDevelop;

SvnCommitLogInputDlg::SvnCommitLogInputDlg( QWidget *parent )
    : QDialog( parent ), Ui::SvnCommitLogInputDlg()
{
    Ui::SvnCommitLogInputDlg::setupUi( this );
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
        oneRow = oneRow + text_mod + " " + prop_mod + " " + path;
        listWidget->addItem( oneRow );
    }
}
QString SvnCommitLogInputDlg::message()
{
    return messageBox->toPlainText();
}

//////////////////////////////////////////////////////////////////////

SvnCommitOptionDlg::SvnCommitOptionDlg( KDevSubversionPart *part, QWidget *parent )
    : QDialog( parent ), Ui::SvnCommitOptionDlg()
    , m_part(part)
{
    Ui::SvnCommitOptionDlg::setupUi(this);
    treeWidget->resizeColumnToContents(0);
    treeWidget->resizeColumnToContents(1);
}
SvnCommitOptionDlg::~SvnCommitOptionDlg()
{}

void SvnCommitOptionDlg::setCommitCandidates( const KUrl::List &urls )
{
    for( QList<KUrl>::const_iterator it = urls.begin(); it != urls.end(); ++it ){
        KUrl url(*it);
        QFileInfo fileInfo(url.path());
        
        if( fileInfo.isFile() ){
            //QString dirPath = fileInfo.dirPath(true); //absolute parent path
            const QList<VcsFileInfo> vcslist =
                    m_part->statusSync( url, IVersionControl::NonRecursive );
            
            if( vcslist.count() == 1 ){
                if( vcslist.at(0).filePath() == url )
                    insertRow( vcslist.at(0) );
            } else{
                insertRow( VcsFileInfo::state2String( VcsFileInfo::Unknown), url );
            }
        }// end of isFile()
        
        else if( fileInfo.isDir() ){
            const QList<VcsFileInfo> vcslist =
                    m_part->statusSync( url, IVersionControl::Recursive );

            VcsFileInfo vcsInfo;
            for( QList<VcsFileInfo>::const_iterator it = vcslist.begin(); it != vcslist.end(); ++it ){
                vcsInfo = *it;
                
                if( vcsInfo.state() == VcsFileInfo::Added ||
                    vcsInfo.state() == VcsFileInfo::Deleted ||
                    vcsInfo.state() == VcsFileInfo::Modified ||
                    vcsInfo.state() == VcsFileInfo::Replaced ) {
                    
                    insertRow( vcsInfo );
                }
            }
            
        }// end of isDIr()
        
        else if( !fileInfo.exists() ){
            // maybe delete file
//             this->insertItem( VCSFileInfo::state2String( VCSFileInfo::Deleted ), oneUrl );
            insertRow( VcsFileInfo::state2String(VcsFileInfo::Deleted), url );
        }
        
        else{
            // should not reach here
        }
    }
}
KUrl::List SvnCommitOptionDlg::checkedUrls()
{
    KUrl::List list;
    
    QTreeWidgetItemIterator it( treeWidget, QTreeWidgetItemIterator::Checked );
    for( ; *it; ++it ){
        QString path = (*it)->text( 2 );
        list << path;
    }
    return list;
}

bool SvnCommitOptionDlg::recursive()
{
    return recursiveChk->isChecked();
}

bool SvnCommitOptionDlg::keepLocks()
{
    return keepLocksChk->isChecked();
}

int SvnCommitOptionDlg::exec()
{
    return QDialog::exec();
}

void SvnCommitOptionDlg::insertRow( const KDevelop::VcsFileInfo &info )
{
    QStringList strings;
    strings << " " << VcsFileInfo::state2String(info.state()) << info.filePath().prettyUrl();
    QTreeWidgetItem *item = new QTreeWidgetItem( treeWidget, strings );
    item->setCheckState(0, Qt::Checked);
}

void SvnCommitOptionDlg::insertRow( QString state, KUrl url )
{
    QStringList strings;
    strings << " " << state << url.prettyUrl();
    QTreeWidgetItem *item = new QTreeWidgetItem( treeWidget, strings );
    item->setCheckState(0, Qt::Checked);
}

#include "svn_commitwidgets.moc"
