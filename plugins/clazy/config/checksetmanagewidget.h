/*
 * This file is part of KDevelop
 *
 * Copyright 2020 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef KDEVCLAZY_CHECKSETMANAGEWIDGET_H
#define KDEVCLAZY_CHECKSETMANAGEWIDGET_H

#include "ui_checksetmanagewidget.h"

// plugin
#include "checksdb.h"

namespace Clazy
{

class CheckSetSelectionListModel;
class CheckSetSelectionManager;

class CheckSetManageWidget: public QWidget
{
    Q_OBJECT

public:
    explicit CheckSetManageWidget(QWidget* parent = nullptr);

public:
    void setCheckSetSelectionManager(CheckSetSelectionManager* checkSetSelectionManager,
                                     const QSharedPointer<const ChecksDB>& db);

    void reload();
    void store() const;

Q_SIGNALS:
    void changed();

private:
    QString askNewCheckSetSelectionName(const QString& defaultName);
//     void batchModeEditButtonClicked();
    void addCheckSetSelection();
    void cloneSelectedCheckSetSelection();
    void removeSelectedCheckSetSelection();
    void setSelectedCheckSetSelectionAsDefault();
    void editSelectedCheckSetSelectionName();

    void onDefaultCheckSetSelectionChanged(const QString& checkSetSelectionId);
    void onSelectedCheckSetSelectionChanged(int selectedCheckSetSelectionIndex);
    void onEnabledChecksChanged(const QString& selection);

private:
    Ui::CheckSetManageWidget m_ui;
    CheckSetSelectionListModel* m_checkSetSelectionListModel = nullptr;
};

}

#endif
