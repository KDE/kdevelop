/***************************************************************************
*   Copyright (C) 2001 by Bernd Gehrmann                                  *
*   bernd@kdevelop.org                                                    *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef _TARGETOPTIONSDLG_
#define _TARGETOPTIONSDLG_

#include "targetoptionsdlgbase.h"

class AutoProjectWidget;
class TargetItem;


class TargetOptionsDialog : public TargetOptionsDialogBase
{
	Q_OBJECT

public:
	TargetOptionsDialog( AutoProjectWidget *widget, TargetItem *item,
	                     QWidget *parent = 0, const char *name = 0 );
	~TargetOptionsDialog();

private:
	virtual void insideMoveUpClicked();
	virtual void insideMoveDownClicked();
	virtual void outsideMoveUpClicked();
	virtual void outsideMoveDownClicked();
	virtual void outsideAddClicked();
	virtual void outsideEditClicked();
	virtual void outsideRemoveClicked();
	virtual void accept();

	void readConfig();
	void storeConfig();

	TargetItem *target;
	AutoProjectWidget *m_widget;
};

#endif 
// kate: indent-mode csands; tab-width 4;

