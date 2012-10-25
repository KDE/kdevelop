/*
 * This file is part of KDevelop
 * Copyright 2012 Przemysław Czekaj <xcojack@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef OPENWITHCONFIG_H
#define OPENWITHCONFIG_H

#include <QWidget>
#include <KDialog>
#include <QStandardItemModel>

#include <KDE/KService>

#include "ui_openwithconfig.h"

class OpenWithConfig : public KDialog
{
public:
    Q_OBJECT

public:
    explicit OpenWithConfig( const QString& mimeType, QWidget *parent = 0 );
    virtual ~OpenWithConfig();

    void addItem( const KService::Ptr& service );

private slots:
    void save();
    void serviceSelected( const QModelIndex& index );
    void internalCheckboxToggle( bool checked );

private:
    void manageOkButton();

    Ui::OpenWithConfigUI *configWidget;
    QString mimeType;
    QStandardItemModel *serviceModel;
    QStandardItem *serviceItem = 0;
};

#endif // OPENWITHCONFIG_H
