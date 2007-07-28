#ifndef SVN_CATWIDGETS_H
#define SVN_CATWIDGETS_H

#include <kdialog.h>
#include "ui_uicat_option_dlg.h"
class KUrl;
class SvnRevision;

class SvnCatOptionDlg : public KDialog
{
    Q_OBJECT
public:
    explicit SvnCatOptionDlg( const KUrl &path, QWidget *parent );
    ~SvnCatOptionDlg();

    KUrl url();
    SvnRevision revision();

private:
    Ui::SvnCatOptionDlg ui;
};

#endif
