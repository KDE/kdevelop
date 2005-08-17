/***************************************************************************
                            -------------------
   begin                : Frag' mich was leichteres
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

/** Here resides the Import-existing-files-dialog of the Automake Manager **/
/** (a KDevelop build tool part) **/

#ifndef ADDEXISTINGFILESDLG_H
#define ADDEXISTINGFILESDLG_H

#include <qdialog.h>
//Added by qt3to4:
#include <QDropEvent>
#include <kfile.h>

#include "addexistingdlgbase.h"
#include "fileselectorwidget.h"

#include "misc.h"

class KImportIconView;
class FileSelectorWidget;
class AutoProjectWidget;
class AutoProjectPart;
class SubprojectItem;
class TargetItem;
class KFileItem;

class AddExistingFilesDialog : public AddExistingDlgBase
{
	Q_OBJECT

public:
	AddExistingFilesDialog( AutoProjectPart* part, AutoProjectWidget *widget, 
	                        SubprojectItem* spitem, TargetItem* titem, QWidget* parent = 0,
	                        const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
	~AddExistingFilesDialog();

private:
	FileSelectorWidget* sourceSelector;
	KImportIconView* importView;

	AutoProjectPart* m_part;
	AutoProjectWidget* m_widget;

	TargetItem* m_titem;
	SubprojectItem* m_spitem;

	KFileItemList m_importList;

protected:
	// 	virtual void resizeEvent ( QResizeEvent* ev );
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

#endif // ADDEXISTINGFILESDLG_H
// kate: indent-mode csands; tab-width 4;
