/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_SVNUPDATEWIDGET_H
#define KDEVPLATFORM_PLUGIN_SVNUPDATEWIDGET_H

#include <kdialog.h>
class SvnRevision;

class SvnUpdateOptionDlg : public KDialog
{
    Q_OBJECT
public:
    SvnUpdateOptionDlg( QWidget *parent = NULL );
    ~SvnUpdateOptionDlg();

    SvnRevision revision();
    bool recurse();
    bool ignoreExternal();
private:
    class Private;
    Private *d;
};

#endif
