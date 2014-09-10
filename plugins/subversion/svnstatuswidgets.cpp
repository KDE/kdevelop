/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svnstatuswidgets.h"
#include "svnrevisionwidget.h"
#include "svnmodels.h"
#include <QUrl>
#include <QCheckBox>
#include <QMap>
#include <QHeaderView>
#include <QDir>

SvnStatusOptionDlg::SvnStatusOptionDlg( const QUrl &path, QWidget *parent )
    : KDialog( parent )
{
    ui.setupUi(mainWidget());
    setCaption( i18n("Subversion Status") );
    setButtons( KDialog::Ok | KDialog::Cancel );

    ui.revisionWidget->setKey( SvnRevision::Head );
    ui.revisionWidget->enableType( SvnRevision::Kind );
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

SvnStatusDisplayWidget::SvnStatusDisplayWidget( const QUrl &url, bool repContacted, QWidget *parent )
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
//     headerView->setSectionResizeMode( QHeaderView::ResizeToContents );
}

SvnStatusDisplayWidget::~SvnStatusDisplayWidget()
{
}

void SvnStatusDisplayWidget::setResults( const QMap< QUrl, SvnStatusHolder > &map )
{
    clear();
    QMap<QUrl, SvnStatusHolder>::const_iterator it;

    const QDir requestPath(m_reqUrl.toLocalFile());

    for( it = map.begin(); it != map.end(); ++it ){
        SvnStatusHolder holder = it.value();
        QTreeWidgetItem *item = new QTreeWidgetItem( this );

        QString relative = requestPath.relativeFilePath( it.key().toLocalFile() );
        item->setText( 0, relative );
        item->setText( 1, SvnStatusHolder::statusToString( holder.textStatus ) );
        item->setText( 2, SvnStatusHolder::statusToString( holder.propStatus ) );
        if( m_repContacted ){
            item->setText( 3, SvnStatusHolder::statusToString( holder.reposTextStat ) );
            item->setText( 4, SvnStatusHolder::statusToString( holder.reposPropStat ) );
        }
    }
}


#include "svnstatuswidgets.moc"

