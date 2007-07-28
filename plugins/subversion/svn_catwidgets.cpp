#include "svn_catwidgets.h"
#include "svn_revisionwidget.h"
#include <kurlrequester.h>
#include <QList>

SvnCatOptionDlg::SvnCatOptionDlg( const KUrl &path, QWidget *parent )
    : KDialog( parent )
{
    QWidget *widget = new QWidget(this);
    ui.setupUi(widget);
    setMainWidget( widget );
    setCaption( "Subversion Cat" );
    setButtons( KDialog::Ok | KDialog::Cancel );

    QList<SvnRevision::RevKeyword> keys;
    keys << SvnRevision::HEAD << SvnRevision::BASE << SvnRevision::COMMITTED << SvnRevision::PREV;
    ui.revisionWidget->installKeys(keys);
    ui.revisionWidget->setKey( SvnRevision::HEAD );
    ui.revisionWidget->enableType( SvnRevision::kind );

    ui.urledit->setUrl( path );
}

SvnCatOptionDlg::~SvnCatOptionDlg()
{}

KUrl SvnCatOptionDlg::url()
{
    return ui.urledit->url();
}

SvnRevision SvnCatOptionDlg::revision()
{
    return ui.revisionWidget->revision();
}

#include "svn_catwidgets.moc"
