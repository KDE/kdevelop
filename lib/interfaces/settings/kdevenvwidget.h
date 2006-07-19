/* This file is part of KDevelop
Copyright (C) 2006 Adam Treat <treat@kde.org>

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

#ifndef KDEVENVWIDGET_H
#define KDEVENVWIDGET_H

#include <QDialog>
#include "ui_kdevenvsettings.h"

class KDevEnvWidget: public QDialog, private Ui::EnvironmentWidget
{
    Q_OBJECT
public:
    KDevEnvWidget( QWidget *parent = 0 );
    virtual ~KDevEnvWidget();

private slots:
    void on_variable_combo_highlighted( const QString & );
    void on_add_button_clicked();
    void on_edit_button_clicked();
    void on_remove_button_clicked();
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
