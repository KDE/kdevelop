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

#ifndef _ADDFILEDLG_H_
#define _ADDFILEDLG_H_

#include <qdialog.h>

#include "addfiledlgbase.h"

class QCheckBox;
class QLineEdit;
class AutoProjectPart;
class AutoProjectWidget;
class SubprojectItem;
class TargetItem;


class AddFileDialog : public AddFileDlgBase
{
	Q_OBJECT

public:
	AddFileDialog( AutoProjectPart *part, AutoProjectWidget *widget,
	               SubprojectItem *spitem, TargetItem *item,
	               QWidget *parent = 0, const char *name = 0 );
	~AddFileDialog();

protected:
	virtual void accept();

private:
	AutoProjectPart *m_part;
	AutoProjectWidget *m_widget;
	SubprojectItem *subProject;
	TargetItem *target;
};

#endif
