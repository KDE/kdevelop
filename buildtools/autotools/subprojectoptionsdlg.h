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

#ifndef _SUBPROJECTOPTIONSDLG_H_
#define _SUBPROJECTOPTIONSDLG_H_

#include "subprojectoptionsdlgbase.h"

class AutoProjectPart;
class AutoProjectWidget;
class SubprojectItem;


class SubprojectOptionsDialog : public SubprojectOptionsDialogBase
{
	Q_OBJECT

public:
	SubprojectOptionsDialog( AutoProjectPart *part, AutoProjectWidget *widget,
	                         SubprojectItem *item, QWidget *parent = 0, const char *name = 0 );
	~SubprojectOptionsDialog();

private:
	virtual void cflagsClicked();
	virtual void cxxFlagsClicked();
	virtual void fflagsClicked();
	virtual void insideMoveUpClicked();
	virtual void insideMoveDownClicked();
	virtual void outsideMoveUpClicked();
	virtual void outsideMoveDownClicked();
	virtual void outsideAddClicked();
	virtual void outsideRemoveClicked();
	virtual void outsideEditClicked();
	virtual void addPrefixClicked();
	virtual void editPrefixClicked();
	virtual void removePrefixClicked();
	virtual void buildorderMoveUpClicked();
	virtual void buildorderMoveDownClicked();
	virtual void accept();

	void readConfig();
	void storeConfig();

	SubprojectItem *subProject;
	AutoProjectWidget *m_widget;
	AutoProjectPart *m_part;

	QString ccompiler;
	QString cxxcompiler;
	QString f77compiler;
};

#endif 
// kate: indent-mode csands; tab-width 4;

