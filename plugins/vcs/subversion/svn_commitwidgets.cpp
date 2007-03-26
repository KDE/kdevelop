#include "svn_commitwidgets.h"
#include <svn_auth.h>
#include <svn_client.h>
#include <ktextedit.h>
#include <klocale.h>

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
