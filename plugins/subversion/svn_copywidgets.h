#ifndef SVN_COPYWIDGETS_H
#define SVN_COPYWIDGETS_H

#include <kdialog.h>
#include "svn_models.h"
#include "ui_uicopy_option_dlg.h"

class KUrl;
class SvnRevision;

class SvnCopyOptionDlg : public KDialog
{
    Q_OBJECT
public:
    explicit SvnCopyOptionDlg( const KUrl &reqUrl, SvnInfoHolder *info, QWidget *parent );
    ~SvnCopyOptionDlg();

    KUrl source();
    SvnRevision sourceRev();
    KUrl dest();

private Q_SLOTS:
    void srcAsUrlClicked();
    void srcAsPathClicked();

private:
    Ui::SvnCopyOptionDlg ui;
    KUrl m_reqUrl;
    SvnInfoHolder *m_info;
};

#endif
