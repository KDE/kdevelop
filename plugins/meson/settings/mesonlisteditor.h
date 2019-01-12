/* This file is part of KDevelop
    Copyright 2019 Daniel Mensinger <daniel@mensinger-ka.de>

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

#pragma once

#include <QDialog>

namespace Ui {
    class MesonListEditor;
}

class MesonListEditor : public QDialog {
    Q_OBJECT

public:
    explicit MesonListEditor(QStringList content, QWidget *parent);
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
    Ui::MesonListEditor *m_ui = nullptr;

    void moveItem(int src, int dst);
};
