/***************************************************************************
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlabel.h>
#include <qlineedit.h>

#include "releaseinputdialog.h"

///////////////////////////////////////////////////////////////////////////////
// class ReleaseInputDialog
///////////////////////////////////////////////////////////////////////////////

ReleaseInputDialog::ReleaseInputDialog( const QString &releaseMsg, QWidget* parent, const char* name )
	: ReleaseInputDialogBase( parent, name, true, 0 )
{
	setReleaseMsg( releaseMsg );
}

///////////////////////////////////////////////////////////////////////////////

ReleaseInputDialog::~ReleaseInputDialog()
{
}

///////////////////////////////////////////////////////////////////////////////

void ReleaseInputDialog::setReleaseMsg( const QString &msg )
{
	this->releaseNameLabel->setText( msg );
}

///////////////////////////////////////////////////////////////////////////////

QString ReleaseInputDialog::releaseTag() const
{
	return this->releaseLineEdit->text();
}

#include "releaseinputdialog.moc"

