/***************************************************************************
                             -------------------
    begin                : 12/21/2002
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

#ifndef _ADDEXISTINGDIRECTORIESDLG_H_
#define _ADDEXISTINGDIRECTORIESDLG_H_

#include "addexistingdlgbase.h"

#include <qdialog.h>
#include <kfile.h>

//#include "misc.h"
#include "fileselectorwidget.h"

class FileSelectorWidget;
class AutoProjectWidget;
class AutoProjectPart;
class SubprojectItem;
class TargetItem;
class KFileItem;
class KImportIconView;

// class KImportIconView : public KFileDnDIconView
// {
// 	Q_OBJECT
// 	
// public:
// 	KImportIconView ( const QString& strIntro, QWidget* parent = 0, const char* name = 0 );
// 	~KImportIconView() {}
// 	
// 	void somethingDropped ( bool dropped );
// 
// protected:
// 	void drawContents ( QPainter *p, int cx, int cy, int cw, int ch );
// 
// private:
// 	QString m_strIntro;
// 	bool m_bDropped;
// };

class AddExistingDirectoriesDialog : public AddExistingDlgBase
{
	Q_OBJECT
	
public:
  AddExistingDirectoriesDialog ( AutoProjectPart* part, AutoProjectWidget *widget, SubprojectItem* spitem, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
  ~AddExistingDirectoriesDialog();

private:
    FileSelectorWidget* sourceSelector;
    KImportIconView* importView;

    AutoProjectPart* m_part;
	AutoProjectWidget* m_widget;

	TargetItem* m_titem;
	SubprojectItem* m_spitem;

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
