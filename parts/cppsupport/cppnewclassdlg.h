/***************************************************************************
 *   Copyright (C) 1998 by Sandy Meier                                     *
 *   smeier@rz.uni-potsdam.de                                              *
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CPPNEWCLASSDLG_H_
#define _CPPNEWCLASSDLG_H_

#include <qlineedit.h>
#include "cppnewclassdlgbase.h"

class KDevPlugin;


class CppNewClassDialog : public CppNewClassDialogBase
{
    Q_OBJECT

public:
    CppNewClassDialog(KDevPlugin *part, QWidget *parent=0, const char *name=0);
    ~CppNewClassDialog();

protected:
    virtual void accept();
    virtual void classNameChanged(const QString &text);
    virtual void headerChanged();
    virtual void implementationChanged();

private:
    bool headerModified;
    bool implementationModified;
    KDevPlugin *m_part;
};

#endif
