/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_LOADEDPLUGINSDIALOG_H
#define KDEVPLATFORM_LOADEDPLUGINSDIALOG_H

#include <QDialog>

class LoadedPluginsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoadedPluginsDialog( QWidget* parent = nullptr );
};

#endif // KDEVPLATFORM_LOADEDPLUGINSDIALOG_H
