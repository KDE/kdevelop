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

#ifndef PARTEXPLORERFORM_H
#define PARTEXPLORERFORM_H

#include "partexplorerformbase.h"

/**
Adds new signals and slots so we can do more information hiding and make the PartExplorerPart more abstract.

@author Mario Scalas
*/
class PartExplorerForm : public PartExplorerFormBase
{
	Q_OBJECT

public:
    PartExplorerForm( QWidget *parent=0, const char *name=0, WFlags fl=0 );
    virtual ~PartExplorerForm();

	KListView *getListViewResults() const;

signals:
	void searchQuery( QString, QString );

public slots:
	void slotDisplayError( QString );

protected slots:
	void slotSearchRequested();
};

#endif
