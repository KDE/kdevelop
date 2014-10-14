/*
 * This file is part of KDevelop
 * Copyright 2010 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KDEVPLATFORM_PLUGIN_CODEUTILSPLUGIN_H
#define KDEVPLATFORM_PLUGIN_CODEUTILSPLUGIN_H

#include <interfaces/iplugin.h>
#include <interfaces/contextmenuextension.h>

#include <QtCore/QVariant>
#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(PLUGIN_CODEUTILS)

class CodeUtilsPlugin : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    CodeUtilsPlugin( QObject *parent, const QVariantList & = QVariantList() );
    ~CodeUtilsPlugin();

private slots:
    void documentDeclaration();
};


#endif // KDEVPLATFORM_PLUGIN_CODEUTILSPLUGIN_H
