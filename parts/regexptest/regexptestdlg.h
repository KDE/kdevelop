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


class RegexpTestDialog : public RegexpTestDialogBase
{
    Q_OBJECT
    
public:
    RegexpTestDialog( /*KDevPart *part*/ );
    ~RegexpTestDialog();

private:
    virtual void somethingChanged();
};

#endif
