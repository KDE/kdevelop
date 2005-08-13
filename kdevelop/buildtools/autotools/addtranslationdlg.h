/***************************************************************************
*   Copyright (C) 1999 by Sandy Meier                                     *
*   bernd@kdevelop.org                                                    *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef _ADDTRANSLATIONDLG_H_
#define _ADDTRANSLATIONDLG_H_

#include <qdialog.h>

class QComboBox;
class AutoProjectPart;


class AddTranslationDialog : public QDialog
{
	Q_OBJECT

public:
	AddTranslationDialog( AutoProjectPart *part, QWidget *parent = 0, const char *name = 0 );
	~AddTranslationDialog();

private:
	virtual void accept();

	QComboBox *lang_combo;
	AutoProjectPart *m_part;
};

#endif
