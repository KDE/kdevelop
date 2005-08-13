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

#ifndef _ADDTARGETDLG_H_
#define _ADDTARGETDLG_H_

#include "addtargetdlgbase.h"

class AutoDetailsView;
class AutoProjectWidget;
class SubprojectItem;


class AddTargetDialog : public AddTargetDialogBase
{
	Q_OBJECT

public:
	AddTargetDialog( AutoProjectWidget *widget, SubprojectItem *item,
	                 QWidget *parent = 0, const char *name = 0 );
	~AddTargetDialog();

protected slots:
	virtual void slotFileNameChanged ( const QString& );

private:
	virtual void primaryChanged();
	virtual void accept();

	SubprojectItem *m_subproject;
	AutoProjectWidget *m_widget;
	AutoDetailsView* m_detailsView;
};

#endif
