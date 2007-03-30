/*
 *  Copyright (C) 2007 Dukju Ahn (dukjuahn@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 */
 
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
