/* Copyright (C) 2001-2002 by Bernd Gehrmann
 * bernd@kdevelop.org
 * Copyright (C) 2003 Oliver Kellogg
 * okellogg@users.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef SERVICE_H
#define SERVICE_H

#include <ktrader.h>

class ServiceComboBox
{
public:
    static void insertStringList(QComboBox *combo, const QValueList<KService::Ptr> &list,
                                 QStringList *names, QStringList *execs);
    static QString currentText(QComboBox *combo, const QStringList &names);
    static void setCurrentText(QComboBox *combo, const QString &str, const QStringList &names);
    static int itemForText(const QString &str, const QStringList &names);
    static QString defaultCompiler();
};

#endif
