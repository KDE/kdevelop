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

#ifndef _ADDSERVICEDLG_H_
#define _ADDSERVICEDLG_H_

#include "addservicedlgbase.h"

class AutoProjectWidget;
class SubprojectItem;
class TargetItem;


class AddServiceDialog : public AddServiceDialogBase
{
	Q_OBJECT

public:
	AddServiceDialog( AutoProjectWidget *widget, SubprojectItem *spitem,
	                  QWidget *parent = 0, const char *name = 0 );
	~AddServiceDialog();

protected:
	virtual void iconClicked();
	virtual void addTypeClicked();
	virtual void removeTypeClicked();
	virtual void propertyExecuted( Q3ListViewItem *item );
	virtual void accept();

private:
	void updateProperties();

	AutoProjectWidget *m_widget;
	SubprojectItem *subProject;
	QString iconName;
};

#endif
