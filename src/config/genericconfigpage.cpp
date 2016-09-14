/*************************************************************************************
 *  Copyright (C) 2016 by Carlos Nihelton <carlosnsoliveira@gmail.com>               *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

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
    ConfigGroup group = ptr->group ( "Clangtidy" );

    m_underlineAvailChecks = list;
    m_availableChecksModel->setStringList ( m_underlineAvailChecks );
    *m_activeChecksReceptor = group.readEntry ( ConfigGroup::EnabledChecks ).split ( ',' );

    for ( int i=0; i<m_availableChecksModel->rowCount(); ++i ) {
        QModelIndex index = m_availableChecksModel->index ( i,0 );
        if ( index.isValid() ) {
            if ( m_activeChecksReceptor->contains ( ( index.data().toString() ) ) ) {
                //todo continue.
                m_selectedItemModel->select ( index, QItemSelectionModel::Select );
            } else {
                m_selectedItemModel->select ( index, QItemSelectionModel::Deselect );
            }
        }

    }
}

//TODO: Adapt from here to the end of file.
void GenericConfigPage::apply()
{
    KSharedConfigPtr ptr = m_project->projectConfiguration();
    ConfigGroup projConf = ptr->group ( "Clangtidy" );

    //TODO: discover a way to set the project folders where user header files might exist into this option.
    //Right now it only works with manual entry.
    projConf.writeEntry ( ConfigGroup::HeaderFilter,ui->headerFilterText->text() );
    projConf.writeEntry ( ConfigGroup::AdditionalParameters, ui->clangtidyParameters->text() );
    projConf.enableEntry ( ConfigGroup::CheckSystemHeaders, ui->sysHeadersCheckBox->isChecked() );
    projConf.enableEntry ( ConfigGroup::UseConfigFile, ui->overrideConfigFileCheckBox->isChecked() );
    projConf.enableEntry ( ConfigGroup::DumpConfig, ui->dumpCheckBox->isChecked() );


    for ( int i=0; i<m_availableChecksModel->rowCount(); ++i ) {
        QModelIndex index = m_availableChecksModel->index ( i,0 );
        if ( index.isValid() ) {
            bool isSelected = m_selectedItemModel->isSelected ( index );
            auto check = index.data().toString();
            if ( isSelected ) {
                *m_activeChecksReceptor << check;
            } else {
                m_activeChecksReceptor->removeAll ( check );
            }
        }

    }
    projConf.writeEntry ( ConfigGroup::EnabledChecks, m_activeChecksReceptor->join(','));
}

void GenericConfigPage::defaults()
{
    bool wasBlocked = signalsBlocked();
    blockSignals ( true );

    KSharedConfigPtr ptr = m_project->projectConfiguration();
    ConfigGroup projConf = ptr->group ( "Clangtidy" );

    //TODO: discover a way to set the project folders where user header files might exist into this option.
    //Right now it only works with manual entry.
    projConf.writeEntry ( ConfigGroup::HeaderFilter,"" );
    ui->headerFilterText->setText ( "" );

    projConf.writeEntry ( ConfigGroup::AdditionalParameters, "" );
    ui->clangtidyParameters->setText ( QString ( "" ) );

    projConf.writeEntry ( ConfigGroup::CheckSystemHeaders, "" );
    ui->sysHeadersCheckBox->setChecked ( false );

    projConf.enableEntry ( ConfigGroup::UseConfigFile, true );
    ui->overrideConfigFileCheckBox->setChecked ( false );
    ui->CheckListGroupBox->setEnabled ( false );

    projConf.enableEntry ( ConfigGroup::DumpConfig, true );
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
            if ( enable ) {
                *m_activeChecksReceptor << check;
            } else {
                m_activeChecksReceptor->removeAll ( check );
            }
        }
    }
    projConf.writeEntry ( ConfigGroup::EnabledChecks, m_activeChecksReceptor->join(',') );
    blockSignals ( wasBlocked );
}

void GenericConfigPage::reset()
{
    KSharedConfigPtr ptr = m_project->projectConfiguration();
    ConfigGroup projConf = ptr->group ( "Clangtidy" );
    if ( !projConf.isValid() ) {
        return;
    }
    ui->headerFilterText->setText ( projConf.readEntry ( ConfigGroup::HeaderFilter ) );
    ui->clangtidyParameters->setText ( projConf.readEntry (ConfigGroup::AdditionalParameters ) );
    ui->sysHeadersCheckBox->setChecked ( !projConf.readEntry ( ConfigGroup::CheckSystemHeaders ).isEmpty() );
    ui->overrideConfigFileCheckBox->setChecked ( projConf.readEntry (ConfigGroup::UseConfigFile ).isEmpty() );
    ui->CheckListGroupBox->setEnabled ( projConf.readEntry (ConfigGroup::UseConfigFile ).isEmpty() );
    ui->dumpCheckBox->setChecked ( !projConf.readEntry (ConfigGroup::DumpConfig).isEmpty() );
}

}
