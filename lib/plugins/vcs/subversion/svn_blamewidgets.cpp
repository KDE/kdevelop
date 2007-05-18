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

#include "svn_blamewidgets.h"
#include <QListWidget>
#include <QGridLayout>
#include <QPushButton>
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

/////////////////////////////////////////////////////////////////////////////

SvnBlameFileSelectDlg::SvnBlameFileSelectDlg( QWidget *parent )
    : KDialog( parent )
{
    m_selected = "";
    setWindowTitle( i18n("Select one file to view annotation") );

    m_layout = new QGridLayout( this );
    m_listWidget = new QListWidget( this );
    m_okBtn = new QPushButton( i18n("OK"), this );
    m_cancelBtn = new QPushButton( i18n("Cancel"), this );
    m_layout->addWidget( m_listWidget, 0, 0, 1, -1 );
    m_layout->addWidget( m_okBtn, 1, 0 );
    m_layout->addWidget( m_cancelBtn, 1, 1 );

    connect( m_okBtn, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( m_cancelBtn, SIGNAL(clicked()), this, SLOT(reject()) );
}

SvnBlameFileSelectDlg::~SvnBlameFileSelectDlg()
{}

void SvnBlameFileSelectDlg::setCandidate( QStringList *list )
{
    for( QList<QString>::iterator it = list->begin(); it != list->end(); ++it ){
        m_listWidget->addItem( *it );
    }
}

QString SvnBlameFileSelectDlg::selected()
{
    return m_selected;
}

void SvnBlameFileSelectDlg::accept()
{
    while( true ){
        QListWidgetItem *item = m_listWidget->currentItem();
        if( item ){
            m_selected = item->text();
            break;
        }
        else{
            KMessageBox::error( this, i18n("Select file from list to view annotation") );
        }
    }
    QDialog::accept();
}

#include "svn_blamewidgets.moc"
