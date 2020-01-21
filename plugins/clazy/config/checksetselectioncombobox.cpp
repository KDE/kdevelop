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

#include "checksetselectioncombobox.h"

// KF
#include <KLocalizedString>

namespace Clazy {

CheckSetSelectionComboBox::CheckSetSelectionComboBox(QWidget* parent)
    : KComboBox(parent)
{
    connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CheckSetSelectionComboBox::onCurrentIndexChanged);
}

void CheckSetSelectionComboBox::setCheckSetSelections(const QVector<CheckSetSelection>& checkSetSelections,
                                                      const QString& defaultCheckSetSelectionId)
{
    clear();

    addItem(i18nc("@item:inlistbox", "Custom"), QVariant());

    for (const auto& checkSetSelection : checkSetSelections) {
        if (checkSetSelection.id() == defaultCheckSetSelectionId) {
            addItem(i18nc("@item:inlistbox", "Use default (currently: %1)", checkSetSelection.name()),
                    QStringLiteral("Default"));
            break;
        }
    }

    for (const auto& checkSetSelection : checkSetSelections) {
        addItem(checkSetSelection.name(), checkSetSelection.id());
    }
}

QString CheckSetSelectionComboBox::selection() const
{
    return currentData().toString();
}

void CheckSetSelectionComboBox::setSelection(const QString& selection)
{
    const int index = findData(selection);
    setCurrentIndex(index);
}

void CheckSetSelectionComboBox::onCurrentIndexChanged()
{
    emit selectionChanged(currentData().toString());
}

}
