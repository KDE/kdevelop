/***************************************************************************
 *   Copyright (C) 2007 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef COMMITDLG_H
#define COMMITDLG_H

#include <QDialog>
#include <KTextEdit>

#include "ui_commitdlg.h"

/**
 * Allows to enter text which can them be used as
 * parameter for @code cvs commit @endcode
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class CommitDlg : public QDialog, private Ui::CommitDlgBase
{
Q_OBJECT
public:
    CommitDlg(QDialog *parent = 0);
    virtual ~CommitDlg();

    /**
     * @return The text entered by the user
     */
    QString message() { return textedit->toPlainText(); }
};

#endif
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
