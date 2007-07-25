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

#include "svn_statuswidgets.h"
#include "svn_revisionwidget.h"
#include "svn_models.h"
#include <kurl.h>
#include <QCheckBox>
#include <QMap>
#include <QHeaderView>

SvnStatusOptionDlg::SvnStatusOptionDlg( const KUrl &path, QWidget *parent )
    : KDialog( parent )
{
    QWidget *widget = new QWidget(this);
    ui.setupUi(widget);
    setMainWidget( widget );
    setCaption( "Subversion Status" );
    setButtons( KDialog::Ok | KDialog::Cancel );

    ui.revisionWidget->setKey( SvnRevision::HEAD );
    ui.revisionWidget->enableType( SvnRevision::kind );
    ui.urledit->setText( path.toLocalFile() );
}

SvnStatusOptionDlg::~SvnStatusOptionDlg()
{
}

SvnRevision SvnStatusOptionDlg::revision()
{
    return ui.revisionWidget->revision();
}

bool SvnStatusOptionDlg::recurse()
{
    return !(ui.nonrecurse->isChecked());
}

bool SvnStatusOptionDlg::getAll()
{
    return !(ui.onlyInterests->isChecked());
}

bool SvnStatusOptionDlg::contactRep()
{
    return ui.contactRepChk->isChecked();
}

bool SvnStatusOptionDlg::noIgnore()
{
    return ui.noignore->isChecked();
}

bool SvnStatusOptionDlg::ignoreExternals()
{
    return ui.ignoreExternalsChk->isChecked();
}

////////////////////////////////////////////////////////////////

SvnStatusDisplayWidget::SvnStatusDisplayWidget( const KUrl& url, bool repContacted, QWidget *parent )
    : QTreeWidget( parent )
    , m_repContacted( repContacted ), m_reqUrl(url)
{
    setRootIsDecorated( false );
    setAllColumnsShowFocus( true );
    // path, wc-text, wc-prop, rep-text, rep-prop. Have a room to show more fields.
    if( m_repContacted ){
        setColumnCount( 5 );
        setHeaderLabels( QStringList() << "Relative Path" << "Text" << "Prop" << "Remote text" << "Remote prop" );
    }
    else{
        setColumnCount( 3 );
        setHeaderLabels( QStringList() << "Relative Path" << "Text" << "Prop" );
    }

    sortByColumn(0, Qt::AscendingOrder);

//     QHeaderView *headerView = header();
//     headerView->setResizeMode( QHeaderView::ResizeToContents );
}

SvnStatusDisplayWidget::~SvnStatusDisplayWidget()
{
}

void SvnStatusDisplayWidget::setResults( const QMap< KUrl, SvnStatusHolder > &map )
{
    clear();
    QMap<KUrl, SvnStatusHolder>::const_iterator it;
    for( it = map.begin(); it != map.end(); ++it ){
        SvnStatusHolder holder = it.value();
        QTreeWidgetItem *item = new QTreeWidgetItem( this );

        QString relative = KUrl::relativePath( m_reqUrl.toLocalFile(), it.key().toLocalFile() );
        item->setText( 0, relative );
        item->setText( 1, SvnStatusHolder::statusToString( holder.textStatus ) );
        item->setText( 2, SvnStatusHolder::statusToString( holder.propStatus ) );
        if( m_repContacted ){
            item->setText( 3, SvnStatusHolder::statusToString( holder.reposTextStat ) );
            item->setText( 4, SvnStatusHolder::statusToString( holder.reposPropStat ) );
        }
    }
}


#include "svn_statuswidgets.moc"

