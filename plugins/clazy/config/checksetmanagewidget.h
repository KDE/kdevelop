/*
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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
