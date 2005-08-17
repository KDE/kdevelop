/***************************************************************************
                            -------------------
   begin                : 21.11.2002
   copyright            : (C) 2002 by Victor Röder
   email                : victor_roeder@gmx.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _REMOVETARGETDLG_H_
#define _REMOVETARGETDLG_H_

#include "removetargetdlgbase.h"

#include <q3ptrlist.h>

class AutoProjectWidget;
class AutoProjectPart;
class SubprojectItem;
class TargetItem;

/**
 * 
 * KDevelop Authors
 **/
class RemoveTargetDialog : public RemoveTargetDlgBase
{

public:
	RemoveTargetDialog( AutoProjectWidget *widget, AutoProjectPart* part, SubprojectItem *spitem,
	                    TargetItem *titem, QWidget* parent = 0, const char* name = 0 );
	~RemoveTargetDialog();

private:
	SubprojectItem* m_spitem;
	TargetItem* m_titem;
	AutoProjectWidget* m_widget;
	AutoProjectPart* m_part;

	Q3PtrList <SubprojectItem> dependentSubprojects;

protected:
	void init ();
	virtual void accept();
};

#endif 
// kate: indent-mode csands; tab-width 4;
