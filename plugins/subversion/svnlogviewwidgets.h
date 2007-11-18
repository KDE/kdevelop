/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SVN_LOGVIEWWIDGET
#define SVN_LOGVIEWWIDGET
#include "ui_logviewwidget.h"
#include "svnmodels.h"
#include "svnpart.h"
#include <kdialog.h>

class QStandardItemModel;

class QModelIndex;

class SvnLogviewWidget : public QWidget, public Ui::SvnLogviewWidget
{
    Q_OBJECT
public:
    explicit SvnLogviewWidget( const KUrl &url, KDevSubversionPart *part, QWidget *parent );
    virtual ~SvnLogviewWidget();
    void refreshWithNewData( const QList<SvnLogHolder>& datalist );

protected slots:
    void customContextMenuEvent( const QPoint &point );
    void blameRev();
    void diffToPrev();
    void catThisRev();
    void treeViewClicked( const QModelIndex &index );
    void listViewClicked( const QModelIndex &index );

private:
    KDevSubversionPart *m_part;
    SvnLogModel *m_logviewModel;
    SvnChangedPathModel *m_logviewDetailedModel;

    QModelIndex m_contextIndex;
    KUrl m_url;
};

class SvnLogviewOptionDialogPrivate;

class SvnLogviewOptionDialog : public KDialog
{
    Q_OBJECT
public:
    explicit SvnLogviewOptionDialog( const KUrl &url, QWidget *parent = 0 );
    virtual ~SvnLogviewOptionDialog();

    SvnRevision startRev();
    SvnRevision endRev();
    int limit();
    bool strictNode();
private:
    SvnLogviewOptionDialogPrivate *d;

};

class SvnFileSelectFromLogviewDlg : public KDialog
{
    Q_OBJECT
public:
    explicit SvnFileSelectFromLogviewDlg( const QString &title, QWidget *parent = 0L );
    virtual ~SvnFileSelectFromLogviewDlg();
    void setCandidate( SvnChangedPathModel* modifies );
    SvnChangedPath selected();

public Q_SLOTS:
    virtual void accept();

private:
    SvnChangedPathModel* m_candidates;
    SvnChangedPath m_selected;
    QListView* widget;
};

#endif
