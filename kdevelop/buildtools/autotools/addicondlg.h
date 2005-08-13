/***************************************************************************
*   Copyright (C) 2002 by Bernd Gehrmann                                  *
*   bernd@kdevelop.org                                                    *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef _ADDICONDLG_H_
#define _ADDICONDLG_H_

#include "addicondlgbase.h"

class AutoProjectPart;
class AutoProjectWidget;
class SubprojectItem;
class TargetItem;


class AddIconDialog : public AddIconDialogBase
{
	Q_OBJECT

public:
	AddIconDialog( AutoProjectPart *part, AutoProjectWidget *widget,
	               SubprojectItem *spitem, TargetItem *titem,
	               QWidget *parent = 0, const char *name = 0 );
	~AddIconDialog();


private:
	virtual void somethingChanged();
	virtual void accept();

	AutoProjectPart *m_part;
	AutoProjectWidget *m_widget;
	SubprojectItem *m_subProject;
	TargetItem *m_target;
};

#endif
