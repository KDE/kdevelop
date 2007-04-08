#include <kurl.h>
#include <klineedit.h>
#include <qcheckbox.h>
#include <qradiobutton.h>

#include "svn_switchwidget.h"
#include "subversion_global.h"

SvnSwitchDlg::SvnSwitchDlg( const SvnGlobal::SvnInfoHolder *holder,
							const QString &wcPath, QWidget *parent )
	: SvnSwitchDlgBase( parent )
	, m_info( holder )
{
	connect( switchOnlyRadio, SIGNAL(clicked()), this, SLOT(resetCurrentRepositoryUrlEdit()) );
	connect( relocationRadio , SIGNAL(clicked()), this, SLOT(resetCurrentRepositoryUrlEdit()) );
	// set switch only
	switchOnlyRadio->setChecked( true );
	wcUrlEdit->setText( wcPath );
	currentUrlEdit->setText( m_info->url.prettyURL() );
}

SvnSwitchDlg::~SvnSwitchDlg()
{}

const QString SvnSwitchDlg::currentUrl()
{
	return currentUrlEdit->text();
}
const QString SvnSwitchDlg::destUrl()
{
	return destUrlEdit->text();
}

bool SvnSwitchDlg::recursive()
{
	return (! nonRecurseCheck->isChecked() );
}
bool SvnSwitchDlg::switchOnly()
{
	return switchOnlyRadio->isChecked();
}
bool SvnSwitchDlg::relocation()
{
	return relocationRadio->isChecked();
}

void SvnSwitchDlg::resetCurrentRepositoryUrlEdit()
{
	if( relocation() ){
		// only ROOT repository url should be given
		currentUrlEdit->setText( m_info->reposRootUrl.prettyURL() );
	} else if( switchOnly() ){
		// the full URL of item should be given
		currentUrlEdit->setText( m_info->url.prettyURL() );
	} else{
		// should not reach here!!
	}
}

#include "svn_switchwidget.moc"
