/***************************************************************************
*   Copyright (C) 2003 by Roberto Raggi                                   *
*   roberto@kdevelop.org                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef CREATEPCSDIALOG_H
#define CREATEPCSDIALOG_H

#include "createpcsdialogbase.h"

class CppSupportPart;

class CreatePCSDialog : public CreatePCSDialogBase
{
	Q_OBJECT

public:
	CreatePCSDialog( CppSupportPart* part, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
	~CreatePCSDialog();
	/*$PUBLIC_FUNCTIONS$*/

public slots:
	/*$PUBLIC_SLOTS$*/
	virtual void slotSelected( const QString& );
	virtual void slotSelectionChanged( QListViewItem* );

protected:
	/*$PROTECTED_FUNCTIONS$*/

protected slots:
	/*$PROTECTED_SLOTS$*/
	virtual void back();
	virtual void next();
	virtual void reject();
	virtual void accept();
	void setNextPageEnabled( int );

private:
	CppSupportPart* m_part;
	QWidget* m_settings;
	class RppDriver;
};

#endif 
// kate: indent-mode csands; tab-width 4;



