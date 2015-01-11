/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svnrevertwidgets.h"
#include "svnpart.h"
#include "svnmodels.h"
#include <QList>
#include <QTreeWidget>
#include <QFileInfo>
#include <QUrl>

SvnRevertOptionDlg::SvnRevertOptionDlg( KDevSubversionPart *part, QWidget *parent )
    : KDialog(parent), m_part(part)
{
    ui.setupUi( mainWidget() );
    setCaption( i18n("Subversion Revert") );
    setButtons( KDialog::Ok | KDialog::Cancel );
}

SvnRevertOptionDlg::~SvnRevertOptionDlg()
{}

void SvnRevertOptionDlg::setCandidates( const QList<QUrl> &urls )
{
    foreach( const QUrl &_url, urls ){
        QFileInfo fileInfo(_url.toLocalFile());

        if( fileInfo.isFile() ){
            QList<SvnStatusHolder> holderList = m_part->statusSync( _url, false, true, false ).values();
            if( holderList.count() < 1 ){
                insertRow( _url, "unknown", "unknown" );
            }
            else{
                if( holderList.at(0).wcPath == _url.toLocalFile() ){
                    insertRow( _url,
                               SvnStatusHolder::statusToString( holderList.at(0).textStatus ),
                               SvnStatusHolder::statusToString( holderList.at(0).propStatus ) );
                }
                else{
                    insertRow( _url, "unknown", "unknown" );
                }
            }
        }// end of isFile()

        else if( fileInfo.isDir() ){
            // FIXME this code adds every items whose status is not normal. Should we allow
            // only added/removed/modified here?
            QList<SvnStatusHolder> holderList = m_part->statusSync( _url, true, false, false, true ).values();
            foreach( const SvnStatusHolder &_holder, holderList ){

                if( _holder.textStatus != svn_wc_status_unversioned &&
                    _holder.textStatus != svn_wc_status_none )

                    insertRow( _holder.wcPath,
                           SvnStatusHolder::statusToString( _holder.textStatus ),
                           SvnStatusHolder::statusToString( _holder.propStatus ) );
            }
        }

    }
}

QList<QUrl> SvnRevertOptionDlg::candidates()
{
    QList<QUrl> list;

    QTreeWidgetItemIterator it( ui.treeWidget, QTreeWidgetItemIterator::Checked );
    for( ; *it; ++it ){
        QString path = (*it)->text( 3 );
        list << path;
    }
    return list;
}

bool SvnRevertOptionDlg::recurse()
{
    return ui.recursive->isChecked();
}

void SvnRevertOptionDlg::insertRow( const QUrl &url, const QString &textStat, const QString &propStat )
{
    QStringList list;
    list << "" << textStat << propStat << url.toLocalFile() ;
    QTreeWidgetItem *item = new QTreeWidgetItem( ui.treeWidget, list );
    item->setCheckState(0, Qt::Checked);
}


