/* This file is part of KDevelop
Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/
#include "environmentselectionwidget.h"
#include "environmentgrouplist.h"
#include "environmentselectionmodel.h"
#include <ksettings/dispatcher.h>
#include <interfaces/icore.h>
#include <kcomponentdata.h>
#include <QBoxLayout>
#include <QLayout>
#include <KLocalizedString>

namespace KDevelop
{

struct EnvironmentSelectionWidgetPrivate
{
    KComboBox* comboBox;
    EnvironmentSelectionModel* model;
    EnvironmentSelectionWidget* owner;

    EnvironmentSelectionWidgetPrivate( EnvironmentSelectionWidget* _owner )
        : comboBox( new KComboBox( _owner ) )
        , model( new EnvironmentSelectionModel( _owner ) )
        , owner( _owner )
    {
        comboBox->setModel( model );
        comboBox->setEditable( false );
    }
};

EnvironmentSelectionWidget::EnvironmentSelectionWidget( QWidget *parent )
    : QWidget( parent ), d( new EnvironmentSelectionWidgetPrivate( this ) )
{
    // Taken from kdelibs/kdeui/dialogs/kconfigdialogmanager.cpp (no idea whether this is documented)
    // Commits d44186bce4670d2985fb6aba8dba59bbd2c4c77a and 8edc1932ecc62370d9a31836dfa9b2bd0175a293
    // introduced a regression in kdelibs to fix problems running some apps against Qt4.8. Unfortunately
    // this fix breaks exactly our use-case, which is to store the text-value in kconfig instead of
    // the index even though the combobox is editable. Since that change the special combobox-code in
    // kconfigdialogmanager.cpp is run before check a user-property and hence our user-property is
    // ignored. Setting this special kcfg_property to the name of our user-property again overrides
    // the hardcoded combobox-behaviour - until the next one breaks things in kdelibs :|
    setProperty("kcfg_property", QByteArray("currentProfile"));

    setLayout( new QHBoxLayout( this ) );
    layout()->addWidget( d->comboBox );
    layout()->setMargin( 0 );

    setCurrentProfile( QString() ); // select the default profile
}

EnvironmentSelectionWidget::~EnvironmentSelectionWidget()
{
    delete d;
}

QString EnvironmentSelectionWidget::currentProfile() const
{
    return d->model->index( d->comboBox->currentIndex(), 0 ).data( Qt::EditRole ).toString();
}

void EnvironmentSelectionWidget::setCurrentProfile( const QString& profile )
{
    d->comboBox->setCurrentIndex( d->comboBox->findData( profile, Qt::EditRole ) );
    emit currentProfileChanged(profile);
}

void EnvironmentSelectionWidget::reconfigure()
{
    QString selectedProfile = currentProfile();
    d->model->reload();
    setCurrentProfile( d->model->reloadSelectedItem( selectedProfile ) );
}

QString EnvironmentSelectionWidget::effectiveProfileName() const
{
    return d->model->index( d->comboBox->currentIndex(), 0 ).data( EnvironmentSelectionModel::EffectiveNameRole ).toString();
}

EnvironmentGroupList EnvironmentSelectionWidget::environment() const
{
    return d->model->environment();
}

}

