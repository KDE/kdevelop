/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SVN_CHECKOUTWIDGETS_H
#define SVN_CHECKOUTWIDGETS_H

#include <kdialog.h>
#include "ui_uicheckoutoptiondlg.h"

class KUrl;
class SvnRevision;

class SvnCheckoutDialog : public KDialog
{
    Q_OBJECT
public:
    SvnCheckoutDialog( const KUrl &destDir, QWidget *parent );
    virtual ~SvnCheckoutDialog();

    KUrl reposUrl();
    KUrl destPath();
    SvnRevision revision();
//     SvnRevision pegRevision();
    bool recurse();
    bool ignoreExternals();
private:
    Ui::SvnCheckoutDialog ui;
};

#endif
