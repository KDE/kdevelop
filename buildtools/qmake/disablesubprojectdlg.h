/***************************************************************************
*   Copyright (C) 2006 by Andreas Pakulat                                 *
*   apaku@gmx.de                                                          *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef DISABLESUBPROJECTDLG_H
#define DISABLESUBPROJECTDLG_H

#include "disablesubprojectdlgbase.h"

class DisableSubprojectDlg : public DisableSubprojectDlgBase
{
    Q_OBJECT

public:
    DisableSubprojectDlg( const QStringList& projects, QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~DisableSubprojectDlg();
    /*$PUBLIC_FUNCTIONS$*/

    QStringList selectedProjects();

public slots:
    /*$PUBLIC_SLOTS$*/

protected:
    /*$PROTECTED_FUNCTIONS$*/

protected slots:
    /*$PROTECTED_SLOTS$*/

};

#endif


// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
