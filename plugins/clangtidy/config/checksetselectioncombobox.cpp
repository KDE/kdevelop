/*
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "checksetselectioncombobox.h"

// KF
#include <KLocalizedString>

namespace ClangTidy {

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
