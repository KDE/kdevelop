/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_SVN_REVERTWIDGETS_H
#define KDEVPLATFORM_PLUGIN_SVN_REVERTWIDGETS_H

#include <kdialog.h>
#include "ui_revertoptiondlg.h"

class KDevSubversionPart;

class SvnRevertOptionDlg : public KDialog
{
    Q_OBJECT
public:
    explicit SvnRevertOptionDlg( KDevSubversionPart *part, QWidget *parent = 0 );
    ~SvnRevertOptionDlg();

    void setCandidates( const KUrl::List &urls );
    KUrl::List candidates();
    bool recurse();

private:
    void insertRow( const KUrl &url, const QString &textStat, const QString &propStat );
    Ui::SvnRevertOptionDlg ui;
    KDevSubversionPart *m_part;
};

#endif
