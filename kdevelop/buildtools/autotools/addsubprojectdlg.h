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

#ifndef _ADDSUBPROJECTDLG_H_
#define _ADDSUBPROJECTDLG_H_

#include <qdialog.h>
#include <qlineedit.h>

#include "addsubprojectdlgbase.h"

class AutoProjectPart;
class AutoSubprojectView;
class SubprojectItem;


class AddSubprojectDialog : public AddSubprojectDlgBase
{
	Q_OBJECT

public:
	AddSubprojectDialog( AutoProjectPart *part, AutoSubprojectView *widget,
	                     SubprojectItem *item, QWidget *parent = 0, const char *name = 0 );
	~AddSubprojectDialog();

private:
	virtual void accept();

	QLineEdit *name_edit;

	SubprojectItem *m_subProject;
	AutoSubprojectView *m_subprojectView;
	AutoProjectPart *m_part;
};

#endif
