/***************************************************************************
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __DIFFDIALOG_H
#define __DIFFDIALOG_H

#include "diffdialogbase.h"
#include "cvsentry.h"

/**
* Implementation for a dialog which collects data for diff operation
*
* @author Mario Scalas
*/
class DiffDialog : public DiffDialogBase
{
    Q_OBJECT
public:
    DiffDialog(const CVSEntry &entry, QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    virtual ~DiffDialog();

    QString revA() const;
    QString revB() const;

private:
    enum  DiffType { diffLocalBASE, diffLocalHEAD, diffLocalOther, diffArbitrary };

    DiffType requestedDiff() const;
    CVSEntry m_entry;
protected slots:
    virtual void languageChange();
};

#endif // __DIFFDIALOG_H
