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

#ifndef CLANGTIDY_CHECKSETSELECTIONCOMBOBOX_H
#define CLANGTIDY_CHECKSETSELECTIONCOMBOBOX_H

// plugin
#include "checksetselection.h"
// KF
#include <KComboBox>
// Qt
#include <QVector>


namespace ClangTidy {

class CheckSetSelectionComboBox : public KComboBox
{
    Q_OBJECT
    Q_PROPERTY(QString selection READ selection WRITE setSelection NOTIFY selectionChanged USER true)

public:
    explicit CheckSetSelectionComboBox(QWidget* parent = nullptr);

public:
    void setCheckSetSelections(const QVector<CheckSetSelection>& checkSetSelections,
                               const QString& defaultCheckSetSelectionId);

public:
    QString selection() const;
    void setSelection(const QString& selection);

Q_SIGNALS:
    void selectionChanged(const QString& selection);

private Q_SLOTS:
    void onCurrentIndexChanged();
};

}

#endif
