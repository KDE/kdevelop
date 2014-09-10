/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_SVN_STATUSWIDGETS_H
#define KDEVPLATFORM_PLUGIN_SVN_STATUSWIDGETS_H

#include <kdialog.h>
#include <QTreeWidget>
#include "ui_statusoptiondlg.h"

class SvnStatusHolder;
template <typename T1, typename T2> class QMap;
class QUrl;

class SvnStatusOptionDlg : public KDialog
{
    Q_OBJECT
public:
    explicit SvnStatusOptionDlg( const QUrl &path, QWidget *parent = 0 );
    ~SvnStatusOptionDlg();

    SvnRevision revision();
    bool recurse();
    bool getAll();
    bool contactRep();
    bool noIgnore();
    bool ignoreExternals();

private:
    Ui::SvnStatusOptionDlg ui;
};

///////////////////////////////////////////////////////////////

class SvnStatusDisplayWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit SvnStatusDisplayWidget( const QUrl &requestedUrl,
                                     bool contactRepository,
                                     QWidget *parent = 0 );
    ~SvnStatusDisplayWidget();

    void setResults( const QMap< QUrl, SvnStatusHolder > &map );

    bool m_repContacted;
    QUrl m_reqUrl;
};

#endif
