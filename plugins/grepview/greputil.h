/***************************************************************************
 *   Copyright 2010 Julien Desgats <julien.desgats@gmail.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// some utility functions used at various places
#ifndef KDEVPLATFORM_PLUGIN_GREPUTIL_H
#define KDEVPLATFORM_PLUGIN_GREPUTIL_H

#include <QStringList>
#include <QString>

class QComboBox;

/// Returns the contents of a QComboBox as a QStringList
QStringList qCombo2StringList( QComboBox* combo, bool allowEmpty = false );

/// Replaces each occurence of "%s" in pattern by searchString (and "%%" by "%")
QString substitudePattern(const QString& pattern, const QString& searchString);

#endif
