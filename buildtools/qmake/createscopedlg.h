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

#ifndef CREATESCOPEDLG_H
#define CREATESCOPEDLG_H

#include "createscopedlgbase.h"

class QMakeScopeItem;
class TrollProjectWigdet;

class CreateScopeDlg : public CreateScopeDlgBase
{
    Q_OBJECT

public:
        CreateScopeDlg( QMakeScopeItem* item, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~CreateScopeDlg();
    /*$PUBLIC_FUNCTIONS$*/

public slots:
    /*$PUBLIC_SLOTS$*/

protected:
    /*$PROTECTED_FUNCTIONS$*/

protected slots:
    /*$PROTECTED_SLOTS$*/
    virtual void accept();
private:
    QMakeScopeItem* m_item;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

