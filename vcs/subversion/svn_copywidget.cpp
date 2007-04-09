#include "svn_copywidget.h"
#include <klineedit.h>
#include <kurl.h>
#include "subversion_global.h"
#include <kurlrequester.h>
#include <knuminput.h>
#include <kcombobox.h>
#include <qradiobutton.h>

SvnCopyDialog::SvnCopyDialog( const QString &reqPath, SvnGlobal::SvnInfoHolder *holder, QWidget *parent )
	: SvnCopyDialogBase( parent )
	, m_holder(holder)
{
	reqEdit->setText( reqPath );

	connect( urlRadio, SIGNAL(clicked()), this, SLOT(setSourceAsUrl()) );
	connect( pathRadio, SIGNAL(clicked()), this, SLOT(setSourceAsLocalPath()) );
	connect( revnumRadio, SIGNAL(toggled(bool)), revnumInput, SLOT(setEnabled(bool)) );
	connect( revnumRadio, SIGNAL(toggled(bool)), revkindCombo, SLOT(setDisabled(bool)) );
	
	// In many cases, users copy from reository to repository. This is for making tag/branche.
	// The case where copying from local path to repository may be lesser than the above one.
	// Thus, by default retrieve the repository URL of the given local path
	urlRadio->setChecked( true );
	srcEdit->setText( m_holder->url.prettyURL() );
	// Also, revision is set to HEAD by default
	revkindRadio->setChecked( true );
	revkindCombo->insertItem( "HEAD" );
}

SvnCopyDialog::~SvnCopyDialog()
{
}

KURL SvnCopyDialog::sourceUrl()
{
	return KURL( srcEdit->text() );
}

int SvnCopyDialog::revision()
{
	if( revnumRadio->isChecked() )
		return revnumInput->value();
	else
		return -1;
}

QString SvnCopyDialog::revKind()
{
	if( revkindRadio->isChecked() )
		return revkindCombo->currentText();
	else
		return QString("");
}

KURL SvnCopyDialog::destUrl()
{
	return KURL( destRequester->url() );
}

void SvnCopyDialog::setSourceAsUrl()
{
	srcEdit->setText( m_holder->url.prettyURL() );
	revkindCombo->clear();
	revkindCombo->insertItem( "HEAD" );
}

void SvnCopyDialog::setSourceAsLocalPath()
{
	srcEdit->setText( reqEdit->text() );
	revkindCombo->clear();
	revkindCombo->insertItem( "WORKING" );
}

#include "svn_copywidget.moc"
