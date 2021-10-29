/*
    SPDX-FileCopyrightText: 2018 Amish K. Naidu <amhndu@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCRATCHPADVIEW_H
#define SCRATCHPADVIEW_H

#include <QWidget>

#include <vector>

#include "ui_scratchpadview.h"

class Scratchpad;

class QModelIndex;
class QLineEdit;
class QAbstractProxyModel;

class ScratchpadView
    : public QWidget
    , public Ui::ScratchpadBaseView
{
    Q_OBJECT

public:
    ScratchpadView(QWidget* parent, Scratchpad* scratchpad);

private Q_SLOTS:
    void runSelectedScratch();
    void scratchActivated(const QModelIndex& index);
    void createScratch();

private:
    QAbstractProxyModel* proxyModel() const;
    QModelIndex currentIndex() const;

    void setupActions();
    void validateItemActions();

    Scratchpad* m_scratchpad;
    QLineEdit* m_filter = nullptr;
    QVector<QAction*> m_itemActions;
};

#endif // SCRATCHPADVIEW_H
