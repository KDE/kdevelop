/***************************************************************************
                            -------------------
   begin                : 12/21/2002
   copyright            : (C) 2002 by Victor R�er
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

#ifndef _ADDEXISTINGDIRECTORIESDLG_H_
#define _ADDEXISTINGDIRECTORIESDLG_H_

#include "addexistingdlgbase.h"

#include <qdialog.h>
//Added by qt3to4:
#include <QDropEvent>
#include <kfile.h>

#include "misc.h"
#include "fileselectorwidget.h"

class FileSelectorWidget;
class AutoProjectWidget;
class AutoProjectPart;
class SubprojectItem;
class TargetItem;
class KFileItem;
class KImportIconView;

class AddExistingDirectoriesDialog : public AddExistingDlgBase
{
	Q_OBJECT

public:
	AddExistingDirectoriesDialog ( AutoProjectPart* part, AutoProjectWidget *widget,
	                               SubprojectItem* spitem, QWidget* parent = 0, 
	                               const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
	~AddExistingDirectoriesDialog();

private:
	FileSelectorWidget* sourceSelector;
	KImportIconView* importView;

	AutoProjectPart* m_part;
	AutoProjectWidget* m_widget;

	TargetItem* m_titem;
	SubprojectItem* m_spitem;

	KFileItemList m_importList;

protected:
	void init();
	void importItems ();

protected slots:
	void slotAddSelected();
	void slotAddAll();
	void slotRemoveAll();
	void slotRemoveSelected();
	void slotDropped ( QDropEvent* ev );

	void slotOk();
};

#endif 
// kate: indent-mode csands; tab-width 4;

