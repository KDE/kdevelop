/***************************************************************************
 *   Copyright 2010 Julien Desgats <julien.desgats@gmail.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
    foreach(const QChar &ch, pattern)
    {
        if(expectEscape)
        {
            expectEscape = false;
            if(ch == '%')
                result.append('%');
            else if(ch == 's')
                result.append(subst);
            else
                result.append('%').append(ch);
        }
        else if(ch == '%')
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
    int skippedItem = -1;
    if (!combo->currentText().isEmpty() || allowEmpty) {
        list << combo->currentText();
    }
    if (combo->currentIndex() != -1 && !combo->itemText(combo->currentIndex()).isEmpty()) {
        skippedItem = combo->currentIndex();
    }
    for (int i = 0; i < std::min(MAX_LAST_SEARCH_ITEMS_COUNT, combo->count()); ++i) {
        if (i != skippedItem && !combo->itemText(i).isEmpty()) {
            list << combo->itemText(i);
        }
    }
    return list;
}

