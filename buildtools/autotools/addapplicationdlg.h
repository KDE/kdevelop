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

#ifndef _ADDAPPLICATIONDLG_H_
#define _ADDAPPLICATIONDLG_H_

#include "addapplicationdlgbase.h"

class AutoProjectWidget;
class SubprojectItem;
class TargetItem;


class AddApplicationDialog : public AddApplicationDialogBase
{
	Q_OBJECT

public:
	AddApplicationDialog( AutoProjectWidget *widget, SubprojectItem *spitem,
	                      QWidget *parent = 0, const char *name = 0 );
	~AddApplicationDialog();

protected:
	virtual void iconClicked();
	virtual void addTypeClicked();
	virtual void removeTypeClicked();
	virtual void accept();

private:
	AutoProjectWidget *m_widget;
	SubprojectItem *subProject;
	QString iconName;
};

#endif 
// kate: indent-mode csands; tab-width 4;

