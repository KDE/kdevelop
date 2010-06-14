/*
    This plugin is part of KDevelop.

    Copyright (C) 2010 Milian Wolff <mail@milianw.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "externalscriptview.h"

#include "externalscriptplugin.h"

#include <KLocalizedString>

#include <QStandardItemModel>
#include <QSortFilterProxyModel>

ExternalScriptView::ExternalScriptView( ExternalScriptPlugin* plugin, QWidget* parent )
    : QWidget( parent ), m_plugin( plugin )
{
    Ui::ExternalScriptViewBase::setupUi( this );

    setWindowTitle( i18n("External Scripts") );

    m_model = new QSortFilterProxyModel( this );
    m_model->setSourceModel( m_plugin->model() );
    connect( filterText, SIGNAL(userTextChanged(QString)),
             m_model, SLOT(setFilterWildcard(QString)) );

    scriptTree->setModel( m_model );
}

ExternalScriptView::~ExternalScriptView()
{

}

#include "externalscriptview.moc"