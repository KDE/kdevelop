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

#ifndef SVNUPDATEWIDGET_H
#define SVNUPDATEWIDGET_H

#include <kdialog.h>

namespace SvnUtils
{
class SvnRevision;
}

class SvnUpdateOptionDlg : public KDialog
{
    Q_OBJECT
public:
    SvnUpdateOptionDlg( QWidget *parent = NULL );
    ~SvnUpdateOptionDlg();

    SvnUtils::SvnRevision revision();
    bool recurse();
    bool ignoreExternal();
private:
    class Private;
    Private *d;
};

#endif
