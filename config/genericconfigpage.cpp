/* This file is part of KDevelop
  Copyright 2013 Christoph Thielecke <crissi99@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, o (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <KPluginFactory>

#include "debug.h"
#include "genericconfigpage.h"
#include "plugin.h"

#include "ui_genericconfig.h"

#include <interfaces/iproject.h>

#include <KSharedConfig>
#include <KConfigGroup>
#include <QMessageBox>

namespace ClangTidy
{

GenericConfigPage::GenericConfigPage ( KDevelop::IProject *project, QWidget* parent )
    : ConfigPage ( nullptr, nullptr, parent ) , m_project ( project )
{
    ui = new Ui::GenericConfig();
    ui->setupUi ( this );

    m_availableChecksModel = new QStringListModel();
    ui->checkListView->setModel ( m_availableChecksModel );

    m_selectedItemModel = new QItemSelectionModel ( m_availableChecksModel );
    ui->checkListView->setSelectionModel ( m_selectedItemModel );
}

GenericConfigPage::~GenericConfigPage ( void )
{
    delete ui;
}

QString GenericConfigPage::name() const
{
    return i18n ( "clang-tidy" );
}

void GenericConfigPage::setActiveChecksReceptorList ( QStringList* list )
{
    m_activeChecksReceptor = list;
}

void GenericConfigPage::setList ( QStringList list )
{
    KSharedConfigPtr ptr = m_project->projectConfiguration();
    KConfigGroup group = ptr->group ( "Clangtidy" );

    m_underlineAvailChecks = list;
    m_availableChecksModel->setStringList ( m_underlineAvailChecks );

    for ( int i=0; i<m_availableChecksModel->rowCount(); ++i ) {
        QModelIndex index = m_availableChecksModel->index ( i,0 );
        if ( index.isValid() ) {
            if ( group.readEntry ( index.data().toString() )==QLatin1String ( "true" ) ) {
                //todo continue.
                m_selectedItemModel->select ( index, QItemSelectionModel::Select );
            } else {
                m_selectedItemModel->select ( index, QItemSelectionModel::Deselect );
            }
        }

    }
}


void GenericConfigPage::apply()
{
    KSharedConfigPtr ptr = m_project->projectConfiguration();
    KConfigGroup projConf = ptr->group ( "Clangtidy" );

    //TODO: discover a way to set the project folders where user header files might exist into this option.
    //Right now it only works with manual entry.
    projConf.writeEntry ( "HeaderFilter",ui->headerFilterText->text() );
    projConf.writeEntry ( "AdditionalParameters", ui->clangtidyParameters->text() );
    projConf.writeEntry ( "CheckSystemHeaders", (ui->sysHeadersCheckBox->isChecked() ?
        "--system-headers" : "" ) );
    projConf.writeEntry ( "OverrideConfigFile", (ui->overrideConfigFileCheckBox->isChecked() ? "--config" : "") );
    projConf.writeEntry ( "DumpConfigToFile",(ui->dumpCheckBox->isChecked() ? "--dump-config" : "" ) );


    for ( int i=0; i<m_availableChecksModel->rowCount(); ++i ) {
        QModelIndex index = m_availableChecksModel->index ( i,0 );
        if ( index.isValid() ) {
            bool isSelected = m_selectedItemModel->isSelected ( index );
            auto check = index.data().toString();
            projConf.writeEntry ( check, isSelected );
            if ( isSelected ) {
                *m_activeChecksReceptor << check;
            } else {
                m_activeChecksReceptor->removeAll ( check );
            }
        }

    }
}

void GenericConfigPage::defaults()
{
    bool wasBlocked = signalsBlocked();
    blockSignals ( true );

    KSharedConfigPtr ptr = m_project->projectConfiguration();
    KConfigGroup projConf = ptr->group ( "Clangtidy" );

    //TODO: discover a way to set the project folders where user header files might exist into this option.
    //Right now it only works with manual entry.
    projConf.writeEntry ( "HeaderFilter","" );
    ui->headerFilterText->setText ( "" );

    projConf.writeEntry ( "AdditionalParameters", "" );
    ui->clangtidyParameters->setText ( QString ( "" ) );

    projConf.writeEntry ( "CheckSystemHeaders", "" );
    ui->sysHeadersCheckBox->setChecked ( false );

    projConf.writeEntry ( "OverrideConfigFile", "" );
    ui->overrideConfigFileCheckBox->setChecked ( false );
    ui->CheckListGroupBox->setEnabled ( false );

    projConf.writeEntry ( "DumpConfigToFile","--dump-config" );
    ui->dumpCheckBox->setChecked ( true );
    ui->CheckListGroupBox->setEnabled ( true );


    for ( int i=0; i<m_availableChecksModel->rowCount(); ++i ) {
        QModelIndex index = m_availableChecksModel->index ( i,0 );
        if ( index.isValid() ) {
            auto check = index.data().toString();
            bool enable = check.contains ( "cert" ) || check.contains ( "-core." ) || check.contains ( "-cplusplus" ) ||
                          check.contains ( "-deadcode" ) || check.contains ( "-security" ) || check.contains (
                              "cppcoreguide" ) ;
            m_selectedItemModel->select ( index, enable ? QItemSelectionModel::Select :
                                          QItemSelectionModel::Deselect );
            projConf.writeEntry ( check, enable );
        }
    }

    blockSignals ( wasBlocked );
}

void GenericConfigPage::reset()
{
    KSharedConfigPtr ptr = m_project->projectConfiguration();
    KConfigGroup projConf = ptr->group ( "Clangtidy" );
    if ( !projConf.isValid() ) {
        return;
    }
    ui->headerFilterText->setText ( projConf.readEntry ( "HeaderFilter" ) );
    ui->clangtidyParameters->setText ( projConf.readEntry ( "AdditionalParameters" ) );
    ui->sysHeadersCheckBox->setChecked (!projConf.readEntry ( "CheckSystemHeaders" ).isEmpty());
    ui->overrideConfigFileCheckBox->setChecked (!projConf.readEntry ( "OverrideConfigFile" ).isEmpty());
    ui->CheckListGroupBox->setEnabled (!projConf.readEntry ( "OverrideConfigFile" ).isEmpty());
    ui->dumpCheckBox->setChecked (!projConf.readEntry ( "DumpConfigToFile" ).isEmpty());
}

}
