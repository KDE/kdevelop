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

#ifndef _REGEXPTESTDLG_H_
#define _REGEXPTESTDLG_H_

#include "regexptestdlgbase.h"

class KDevPlugin;


class RegexpTestDialog : public RegexpTestDialogBase
{
    Q_OBJECT
    
public:
    RegexpTestDialog( KDevPlugin *part );
    ~RegexpTestDialog();

protected:
    virtual void showEvent(QShowEvent *e);

protected slots:
    void showRegExpEditor();

private:
    virtual void somethingChanged();
    virtual void checkPOSIX();
    virtual void checkQRegExp();
    virtual void checkKRegExp();
    virtual void insertQuoted();

    KDevPlugin *m_part;

    QDialog * _regexp_dialog;
};

#endif
