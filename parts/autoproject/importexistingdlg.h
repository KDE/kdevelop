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

/** Here resides the Import-existing-dialog of the Automake Manager **/
/** (a Gideon build tool part) **/

#ifndef IMPORTEXISTINGDLG_H
#define IMPORTEXISTINGDLG_H

#include <qdialog.h>
#include <kfile.h>

#include "importexistingdlgbase.h"
#include "kfiledndiconview.h"
#include "fileselectorwidget.h"

class FileSelectorWidget;
class AutoProjectWidget;
class AutoProjectPart;
class SubprojectItem;
class TargetItem;
class KFileItem;

class KImportIconView : public KFileDnDIconView
{
	Q_OBJECT
	
public:
	KImportIconView ( const QString& strIntro, QWidget* parent = 0, const char* name = 0 );
	~KImportIconView() {}
	
	void somethingDropped ( bool dropped );

protected:
    void drawContents ( QPainter *p, int cx, int cy, int cw, int ch );

private:
    QString m_strIntro;
	bool m_bDropped;
};


class ImportExistingDialog : public ImportExistingDlgBase
{ 
    Q_OBJECT

public:
    ImportExistingDialog( AutoProjectPart* part, AutoProjectWidget *widget, SubprojectItem* spitem, TargetItem* titem, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
//    ImportExistingDialog( AutoProjectPart* part, AutoProjectWidget *widget, SubprojectItem* spitem, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~ImportExistingDialog();

private:
    FileSelectorWidget* sourceSelector;
    KImportIconView* importView;

    AutoProjectPart* m_part;
	AutoProjectWidget* m_widget;

	TargetItem* m_titem;
	SubprojectItem* m_spitem;
	
  	KFileItemList m_importList;

protected:
	virtual void resizeEvent ( QResizeEvent* ev );
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

#endif // IMPORTEXISTINGDLG_H
