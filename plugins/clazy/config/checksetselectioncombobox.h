/*
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVCLAZY_CHECKSETSELECTIONCOMBOBOX_H
#define KDEVCLAZY_CHECKSETSELECTIONCOMBOBOX_H

// plugin
#include "checksetselection.h"
// KF
#include <KComboBox>
// Qt
#include <QVector>


namespace Clazy {

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
