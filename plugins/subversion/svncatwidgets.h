/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_SVN_CATWIDGETS_H
#define KDEVPLATFORM_PLUGIN_SVN_CATWIDGETS_H

#include <kdialog.h>
#include "ui_catoptiondlg.h"
class QUrl;
class SvnRevision;

class SvnCatOptionDlg : public KDialog
{
    Q_OBJECT
public:
    explicit SvnCatOptionDlg( const QUrl &path, QWidget *parent );
    ~SvnCatOptionDlg();

    QUrl url();
    SvnRevision revision();

private:
    Ui::SvnCatOptionDlg ui;
};

#endif
