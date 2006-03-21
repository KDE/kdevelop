/***************************************************************************
 *   Copyright (C) 2001 by Harald Fernengel                                *
 *   harry@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DIFFDLG_H_
#define _DIFFDLG_H_

#include <kdialogbase.h>
#include <kurl.h>

class DiffWidget;

class DiffDlg : public KDialogBase
{
    Q_OBJECT

public:
    DiffDlg( QWidget *parent = 0, const char *name = 0 );
    virtual ~DiffDlg();

public slots:
    /** The URL has to point to a diff file */
    void openURL( const KUrl& url );
    /** Pass a diff file in here */
    void setDiff( const QString& diff );
    /** clears the difference viewer */
    void slotClear();

private:
    DiffWidget* diffWidget;

};

#endif
