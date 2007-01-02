/***************************************************************************
 *   Copyright (C) 2007 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlabel.h>
#include <qpushbutton.h>

#include <kurl.h>
#include <klineedit.h>
#include <kcompletionbox.h>
#include <kstringhandler.h>
#include <kglobalsettings.h>
#include <kdebug.h>

#include "switchtodialog.h"

SwitchToDialog::SwitchToDialog(const KURL::List & urls, QWidget* parent, const char* name, bool modal, WFlags fl)
: SwitchToDialogBase(parent,name, modal,fl)
{
	QStringList filenames;
	QStringList unmodifedFilenames;
	for (KURL::List::ConstIterator it = urls.begin(); it != urls.end(); ++it)
	{
		KURL url = *it;
		QString filename = url.fileName();

		unmodifedFilenames << filename;
		int occurrences = unmodifedFilenames.contains( filename );
		if ( occurrences > 1 )
		{
			filename += QString(" (%1)").arg( occurrences );
		}

		m_urlMap.insert( filename, url );
		filenames << filename;
	}

	okButton->setEnabled( false );

	selectedFileName->completionObject()->setItems( filenames );
	selectedFileName->setCompletionMode( KGlobalSettings::CompletionPopup );
	selectedFileName->setFocus();	

	connect( selectedFileName->completionBox(), SIGNAL(highlighted(const QString&)), this, SLOT(slotHighlightChanged(const QString&)) );
	connect( selectedFileName, SIGNAL(returnPressed(const QString&)), this, SLOT(slotReturnPressed(const QString&)) );
}

SwitchToDialog::~SwitchToDialog()
{
}

KURL SwitchToDialog::selectedUrl()
{
	return m_selectedUrl;
}

void SwitchToDialog::slotHighlightChanged(const QString & match)
{
	if ( m_urlMap.contains( match ) )
	{
		setSqueezedLabel( m_urlMap[ match ].path() );
	}
}

// btw, not using KSqueezedLabel because it only does center squeeze..
void SwitchToDialog::setSqueezedLabel(const QString & text )
{
	m_unsqueezedString = text;

	QFontMetrics fm(fontMetrics());
	int labelWidth = selectionDisplay->size().width();
	int textWidth = fm.width(text);
	if (textWidth > labelWidth) 
	{
		QString squeezedText = KStringHandler::lPixelSqueeze( text, fm, labelWidth );
		selectionDisplay->setText( squeezedText );
	}
	else
	{
		selectionDisplay->setText( text );
	}

	okButton->setEnabled( true );
}

void SwitchToDialog::resizeEvent( QResizeEvent * e )
{
	setSqueezedLabel( m_unsqueezedString );
	QDialog::resizeEvent( e );
}

void SwitchToDialog::slotReturnPressed( const QString & text )
{
	if ( m_urlMap.contains( selectedFileName->text() ) )
	{
		m_selectedUrl = m_urlMap[ selectedFileName->text() ];
		accept();
	}
	else
	{	
		okButton->setEnabled( false );
	}
}


#include "switchtodialog.moc"

