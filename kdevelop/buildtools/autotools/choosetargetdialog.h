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

#include <kdialogbase.h>

class AutoProjectWidget;
class AutoProjectPart;
class SubprojectItem;
class TargetItem;
class QStringList;
class QWidget;

class ChooseTargetDialog : public KDialogBase
{
Q_OBJECT
public:
	ChooseTargetDialog( AutoProjectWidget* widget, AutoProjectPart* part,
	                    QStringList fileList, QWidget* parent = 0,
	                    const char* name = 0 );
	~ChooseTargetDialog();

public:
	TargetItem* chosenTarget();
	SubprojectItem* chosenSubproject();
	
	bool alwaysUseActiveTarget() const;

public slots:
	void slotSubprojectChanged ( const QString& );
	void slotTargetChanged ( const QString& );

protected:
	virtual void slotOk();
	
private:
	class Private;
	ChooseTargetDialog::Private* d;
};

#endif 
// kate: indent-mode csands; tab-width 4;
