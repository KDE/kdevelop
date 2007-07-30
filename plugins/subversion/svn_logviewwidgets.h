/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SVN_LOGVIEWWIDGET
#define SVN_LOGVIEWWIDGET
#include "ui_svnlogviewwidget.h"
#include "svn_models.h"
#include "subversion_part.h"
#include <kdialog.h>

class QModelIndex;

class SvnLogviewWidget : public QWidget, public Ui::SvnLogviewWidget
{
    Q_OBJECT
public:
    explicit SvnLogviewWidget( KUrl &url, KDevSubversionPart *part, QWidget *parent );
    virtual ~SvnLogviewWidget();
    void refreshWithNewData( QList<SvnLogHolder> datalist );

protected slots:
    void customContextMenuEvent( const QPoint &point );
    void blameRev();
    void diffToPrev();
    void catThisRev();
    void treeViewClicked( const QModelIndex &index );
    void listViewClicked( const QModelIndex &index );

private:
    KDevSubversionPart *m_part;
    LogItem *m_item;
    LogviewTreeModel *m_logviewModel;
    LogviewDetailedModel *m_logviewDetailedModel;

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

class SvnFileSelectWidget;
class SvnFileSelectFromLogviewDlg : public KDialog
{
    Q_OBJECT
public:
    explicit SvnFileSelectFromLogviewDlg( const QString &title, QWidget *parent = 0L );
    virtual ~SvnFileSelectFromLogviewDlg();
    void setCandidate( QStringList *modifies );
    QString selected();

public Q_SLOTS:
    virtual void accept();

private:
    QStringList *m_candidates;
    QString m_selected;
    SvnFileSelectWidget *widget;
};

#endif
