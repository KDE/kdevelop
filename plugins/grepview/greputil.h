/*
    SPDX-FileCopyrightText: 2010 Julien Desgats <julien.desgats@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// some utility functions used at various places
#ifndef KDEVPLATFORM_PLUGIN_GREPUTIL_H
#define KDEVPLATFORM_PLUGIN_GREPUTIL_H

class QComboBox;
#include <QStringList>
class QString;

/// Returns the contents of a QComboBox as a QStringList
QStringList qCombo2StringList( QComboBox* combo, bool allowEmpty = false );

/// Replaces each occurrence of "%s" in pattern by searchString (and "%%" by "%")
QString substitudePattern(const QString& pattern, const QString& searchString);

#endif
