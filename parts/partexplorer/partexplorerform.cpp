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

#include <qlineedit.h>
#include <qpushbutton.h>

#include <kmessagebox.h>
#include <kdebug.h>


#include "partexplorerform.h"

PartExplorerForm::PartExplorerForm( QWidget *parent, const char *name, WFlags fl )
	: PartExplorerFormBase( parent, name, fl )
{
	connect( buttonSearch, SIGNAL(clicked()), this, SLOT(slotSearchRequested())	);
	connect( lineEditType, SIGNAL(returnPressed()), this, SLOT(slotSearchRequested()) );
	connect( lineEditCostraints, SIGNAL(returnPressed()), this, SLOT(slotSearchRequested()) );
}

PartExplorerForm::~PartExplorerForm()
{
}

void PartExplorerForm::slotSearchRequested()
{
	QString queryType = lineEditType->text(),
		queryCostraints = lineEditCostraints->text();

	kdDebug(9000) << ">> slot PartExplorerForm::slotSearchRequested(): " << endl
		<< "  ** queryType = " << queryType << ", queryCostraints = " << queryCostraints
		<< endl;

	if (queryType.isNull() || queryType.isEmpty())
	{
		slotDisplayError( "You must fill at least the type field!!" );
		return;
	}

	emit searchQuery( queryType, queryCostraints );
}

void PartExplorerForm::slotDisplayError( QString errorMessage )
{
	if (errorMessage.isNull() || errorMessage.isEmpty())
	{
		errorMessage = "Unknown error!";
	}
	KMessageBox::error( this, errorMessage );
}

KListView *PartExplorerForm::getListViewResults() const
{
	return listViewResults;
}

#include "partexplorerform.moc"
