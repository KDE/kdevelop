/***************************************************************************
 *   Copyright (C) 1999, 2000 by Bernd Gehrmann                            *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _COMMITDIALOG_H_
#define _COMMITDIALOG_H_

#include <qdialog.h>
#include <qmultilineedit.h>


class CommitDialog : public QDialog
{
	Q_OBJECT
public:
    CommitDialog();
    QString logMessage()
        { return edit->text(); }

protected slots:
	// Override: must check for message not being void.
	virtual void accept();

private:
    QMultiLineEdit *edit;
};

#endif
