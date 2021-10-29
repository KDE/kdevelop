/*
    SPDX-FileCopyrightText: 2010 Julien Desgats <julien.desgats@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "greputil.h"

#include <algorithm>
#include <QChar>
#include <QComboBox>

static int const MAX_LAST_SEARCH_ITEMS_COUNT = 15;

QString substitudePattern(const QString& pattern, const QString& searchString)
{
    QString subst = searchString;
    QString result;
    bool expectEscape = false;
    for (const QChar ch : pattern) {
        if(expectEscape)
        {
            expectEscape = false;
            if (ch == QLatin1Char('%'))
                result.append(QLatin1Char('%'));
            else if (ch == QLatin1Char('s'))
                result.append(subst);
            else
                result.append(QLatin1Char('%') + ch);
        }
        else if (ch == QLatin1Char('%'))
            expectEscape = true;
        else
            result.append(ch);
    }
    return result;
}

QStringList qCombo2StringList( QComboBox* combo, bool allowEmpty )
{
    QStringList list;
    if (!combo) {
        return list;
    }
    QString currentText = combo->currentText();
    int skippedItem = combo->currentIndex();
    if (!currentText.isEmpty() || allowEmpty) {
        list << currentText;
    }
    if (skippedItem != -1 && currentText != combo->itemText(skippedItem)) {
        skippedItem = -1;
    }
    for (int i = 0; i < std::min(MAX_LAST_SEARCH_ITEMS_COUNT, combo->count()); ++i) {
        if (i != skippedItem && !combo->itemText(i).isEmpty()) {
            list << combo->itemText(i);
        }
    }
    return list;
}

