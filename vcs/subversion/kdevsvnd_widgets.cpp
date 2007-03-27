#include "kdevsvnd_widgets.h"
#include <qpushbutton.h>
#include <qlistview.h>
#include <qlabel.h>
#include <klocale.h>

SvnSSLTrustPrompt::SvnSSLTrustPrompt( QWidget* parent, const char* name, bool modal, WFlags f )
	:SvnSSLTrustPromptBase( parent, name, modal, f )
	, m_code(-1)
{
	listView1->setColumnText( 0, "Items" );
	listView1->setColumnText( 1, "Values" );
	btnPermanent->setText(i18n("Accept Permanently"));
	btnTemporary->setText(i18n("Accept Temporarily"));
	btnReject->setText(i18n("Reject"));
	connect( btnPermanent, SIGNAL(clicked()), this, SLOT(setPermanent()) );
	connect( btnTemporary, SIGNAL(clicked()), this, SLOT(setTemporary()) );
	connect( btnReject,    SIGNAL(clicked()), this, SLOT(setRejected ()) );
}
SvnSSLTrustPrompt::~SvnSSLTrustPrompt()
{}

void SvnSSLTrustPrompt::setupCertInfo( QString hostname, QString fingerPrint, QString validfrom, QString validuntil, QString issuerName, QString ascii_cert )
{
	// setup texts
	QListViewItem *host= new QListViewItem(listView1, i18n("Hostname"), hostname );
	QListViewItem *finger = new QListViewItem(listView1, i18n("FingerPrint"), fingerPrint );
	QListViewItem *validFrom = new QListViewItem(listView1, i18n("Valid From"), validfrom );
	QListViewItem *validUntil = new QListViewItem(listView1, i18n("Valid Until"), validuntil );
	QListViewItem *issName = new QListViewItem(listView1, i18n("Issuer"), issuerName );
	QListViewItem *cert = new QListViewItem(listView1, i18n("Cert"), ascii_cert );
}
void SvnSSLTrustPrompt::setupFailedReasonMsg( QString msg )
{
	errMsgLabel->setText( msg );
}
int SvnSSLTrustPrompt::code()
{
	return m_code;
}

void SvnSSLTrustPrompt::setPermanent()
{
	m_code = 1;
}

void SvnSSLTrustPrompt::setTemporary()
{
	m_code = 0;
}

void SvnSSLTrustPrompt::setRejected()
{
	m_code = -1;
}
#include "kdevsvnd_widgets.moc"
