/***************************************************************************
 *   Copyright (C) 2008 by Andreas Pakulat <apaku@gmx.de                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "projectinfopage.h"
#include "ui_projectinfopage.h"

namespace KDevelop
{
    
const int FileNameRole = Qt::UserRole + 1;

ProjectInfoPage::ProjectInfoPage( QWidget* parent )
        : QWidget( parent )
{
    page_ui = new Ui::ProjectInfoPage;
    page_ui->setupUi( this );
    connect( page_ui->nameEdit, &QLineEdit::textEdited, 
             this, &ProjectInfoPage::projectNameChanged );
    connect( page_ui->managerCombo, QOverload<int>::of(&QComboBox::activated),
             this, &ProjectInfoPage::changeProjectManager );
}

ProjectInfoPage::~ProjectInfoPage()
{
    delete page_ui;
}

void ProjectInfoPage::changeProjectManager( int idx )
{
    emit projectManagerChanged( page_ui->managerCombo->itemData( idx ).toString(),
                                page_ui->managerCombo->itemData( idx, FileNameRole ).toString() );
}

void ProjectInfoPage::setProjectName( const QString& name )
{
    page_ui->nameEdit->setText( name );
    emit projectNameChanged( page_ui->nameEdit->text() );
}

void ProjectInfoPage::populateProjectFileCombo(const QVector<ProjectFileChoice>& choices)
{
    page_ui->managerCombo->clear();
    for (const auto& item : choices) {
        const int index = page_ui->managerCombo->count();
        page_ui->managerCombo->addItem(QIcon::fromTheme(item.iconName), item.text, item.pluginId);
        page_ui->managerCombo->setItemData( index, item.fileName, FileNameRole );
    }
    changeProjectManager(0);
}

}

