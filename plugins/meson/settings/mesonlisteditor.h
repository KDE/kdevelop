/*
    SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QDialog>

namespace Ui
{
class MesonListEditor;
}

class MesonListEditor : public QDialog
{
    Q_OBJECT

public:
    explicit MesonListEditor(const QStringList& content, QWidget* parent);
    virtual ~MesonListEditor();

    QStringList content() const;

public Q_SLOTS:
    void add();
    void remove();
    void first();
    void up();
    void down();
    void last();
    void currentItemChanged();

private:
    Ui::MesonListEditor* m_ui = nullptr;

    void moveItem(int src, int dst);
};
