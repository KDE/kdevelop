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

#ifndef _REMOVEFILEDLG_H_
#define _REMOVEFILEDLG_H_

#include <qdialog.h>

#include "removefiledlgbase.h"

class QCheckBox;
class AutoProjectWidget;
class AutoProjectPart;
class SubprojectItem;
class TargetItem;


class RemoveFileDialog : public RemoveFileDlgBase
{
	Q_OBJECT

public:
	RemoveFileDialog( AutoProjectWidget *widget, AutoProjectPart* part, SubprojectItem *spitem,
	                  TargetItem *item, const QString &filename,
	                  QWidget *parent = 0, const char *name = 0 );
	~RemoveFileDialog();

protected:
	virtual void accept();

private:
	QCheckBox *removeFromTargetsCheckBox;
	//QCheckBox *removefromdisk_box;

	AutoProjectWidget* m_widget;
	AutoProjectPart *m_part;
	SubprojectItem *subProject;
	TargetItem *target;
	QString fileName;
};

#endif 
// kate: indent-mode csands; tab-width 4;
