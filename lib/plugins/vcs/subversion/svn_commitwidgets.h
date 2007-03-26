#ifndef SVN_COMMITWIDGETS_H
#define SVN_COMMITWIDGETS_H

#include "ui_svncommitloginputdlg.h"
#include <apr_tables.h>

class SvnCommitLogInputDlg : public QDialog, public Ui::SvnCommitLogInputDlg
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

#endif
