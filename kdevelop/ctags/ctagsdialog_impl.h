/***************************************************************************
                          ctagsdialog_impl.h
                          --------------------
    begin                : Wed April 26 2001
    copyright            : (C) 2001 by rokrau, the kdevelop-team
    email                : rokrau@yahoo.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef SEARCHTAGSDIALOGIMPL_H
#define SEARCHTAGSDIALOGIMPL_H
#include "ctagsdialog.h"

class searchTagsDialogImpl : public searchTagsDialog
{ 
    Q_OBJECT

public:
    searchTagsDialogImpl( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~searchTagsDialogImpl();

public slots:
    void slotLBItemSelected();
    void slotClear();
    void slotGotoTag();
    void slotSearchTag();

};

#endif // SEARCHTAGSDIALOGIMPL_H
