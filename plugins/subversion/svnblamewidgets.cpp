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

#include "svnblamewidgets.h"
#include "svnrevision.h"
#include "svnrevisionwidget.h"
#include <kmessagebox.h>
#include <QSortFilterProxyModel>

SvnBlameWidget::SvnBlameWidget(QWidget *parent)
    : QWidget(parent), Ui::SvnBlameWidget()
{
    Ui::SvnBlameWidget::setupUi(this);

    m_blameModel= new SvnBlameModel(this);
    m_proxy = new QSortFilterProxyModel();
    m_proxy->setSourceModel( m_blameModel );
    treeView->setModel( m_proxy );
    treeView->setSortingEnabled(true);
    treeView->setIndentation(-7);
    treeView->sortByColumn(0, Qt::DescendingOrder);

}

SvnBlameWidget::~SvnBlameWidget()
{}

void SvnBlameWidget::refreshWithNewData( const QList<SvnBlameHolder>& datalist )
{
    m_blameModel->setBlameList( datalist );
    treeView->resizeColumnToContents(0);//line
    treeView->resizeColumnToContents(1);//rev
    treeView->resizeColumnToContents(2);//author
}


//////////////////////////////////////////////////////////////////////

SvnBlameOptionDlg::SvnBlameOptionDlg( QWidget *parent )
    : KDialog( parent )
{
    QWidget *widget = new QWidget(this);
    ui.setupUi(widget);
    setMainWidget( widget );
    setCaption( "Subversion Blame" );
    setButtons( KDialog::Ok | KDialog::Cancel );

    ui.startRevWidget->setNumber( 0 );
    ui.startRevWidget->enableType( SvnRevision::Number );
    ui.endRevWidget->setKey( SvnRevision::Base );
    ui.endRevWidget->enableType( SvnRevision::Kind );
}

SvnBlameOptionDlg::~SvnBlameOptionDlg()
{
}

SvnRevision SvnBlameOptionDlg::startRev()
{
    return ui.startRevWidget->revision();
}

SvnRevision SvnBlameOptionDlg::endRev()
{
    return ui.endRevWidget->revision();
}

#include "svnblamewidgets.moc"

