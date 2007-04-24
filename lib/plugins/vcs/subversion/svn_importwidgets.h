/***************************************************************************
 *   Copyright (C) 2007 by Dukju Ahn                                       *
 *   dukjuahn@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SVNIMPORTWIDGETS_H
#define SVNIMPORTWIDGETS_H

#include <kdialog.h>

class SvnImportDialog : public KDialog
{
    Q_OBJECT
public:
    SvnImportDialog( QWidget *parent = NULL );
    virtual ~SvnImportDialog();

    KUrl srcLocal();
    KUrl destRepository();
    bool nonRecurse();
    bool noIgnore();

private:
    class Private;
    Private *d;
};

#endif
