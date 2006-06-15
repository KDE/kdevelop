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

#ifndef _ADDPREFIXDLG_H_
#define _ADDPREFIXDLG_H_

#include <qdialog.h>
#include <klineedit.h>
class QPushButton;

class AddPrefixDialog : public QDialog
{
	Q_OBJECT

public:
	AddPrefixDialog( const QString& nameEdit = "", const QString& pathEdit = "",
	                 QWidget *parent = 0, const char *name = 0 );
	~AddPrefixDialog();

	QString name() const
	{
		return name_edit->text();
	}
	QString path() const
	{
		return path_edit->text();
	}
private slots:
	void slotPrefixChanged();

private:
	KLineEdit *name_edit;
	KLineEdit *path_edit;
	QPushButton *m_pOk;
};

#endif
