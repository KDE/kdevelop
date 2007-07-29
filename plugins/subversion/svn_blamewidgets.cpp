/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svn_blamewidgets.h"
#include "svn_revision.h"
#include "svn_revisionwidget.h"
#include <kmessagebox.h>

SvnBlameWidget::SvnBlameWidget(QWidget *parent)
    : QWidget(parent), Ui::SvnBlameWidget()
{
    Ui::SvnBlameWidget::setupUi(this);

    m_item = new BlameItem();

    m_blameModel= new BlameTreeModel(m_item);
    treeView->setModel( m_blameModel );
    treeView->setSortingEnabled(true);
    treeView->setIndentation(-7);
    treeView->sortByColumn(0, Qt::DescendingOrder);

}

SvnBlameWidget::~SvnBlameWidget()
{}

void SvnBlameWidget::refreshWithNewData( QList<SvnBlameHolder> datalist )
{
    m_blameModel->prepareItemUpdate();
    m_item->setHolderList( datalist );
    m_blameModel->finishedItemUpdate();
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
    ui.startRevWidget->enableType( SvnRevision::number );
    ui.endRevWidget->setKey( SvnRevision::BASE );
    ui.endRevWidget->enableType( SvnRevision::kind );
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

#include "svn_blamewidgets.moc"
