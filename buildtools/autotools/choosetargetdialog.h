/***************************************************************************
                            -------------------
   begin                : 29.11.2002
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

#ifndef _CHOOSETARGETDIALOG_H_
#define _CHOOSETARGETDIALOG_H_

#include "choosetargetdlgbase.h"

class AutoProjectWidget;
class AutoProjectPart;
class SubprojectItem;
class TargetItem;

class QStringList;

class ChooseTargetDialog : public ChooseTargetDlgBase
{

public:
	ChooseTargetDialog( AutoProjectWidget* widget, AutoProjectPart* part,
	                    QStringList fileList, QWidget* parent = 0,
	                    const char* name = 0 );
	~ChooseTargetDialog();

private:
	AutoProjectWidget* m_widget;
	AutoProjectPart* m_part;
	QStringList m_fileList;
	QPtrList <SubprojectItem> m_subprojectList;
	SubprojectItem* m_choosenSubproject;
	TargetItem* m_choosenTarget;

public:
	TargetItem* choosenTarget();
	SubprojectItem* choosenSubproject();

private slots:
	void slotActiveTargetToggled( bool );
	void slotChooseTargetToggled( bool );
	void slotSubprojectChanged ( const QString& );
	void slotTargetChanged ( const QString& );
	void slotSubprojectTextChanged ( const QString& );
	void slotTargetTextChanged ( const QString& );

protected:
	virtual void accept ();
};

#endif 
// kate: indent-mode csands; tab-width 4;
