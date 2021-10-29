/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_EDITEXTERNALSCRIPT_H
#define KDEVPLATFORM_PLUGIN_EDITEXTERNALSCRIPT_H

#include <QDialog>

#include "ui_editexternalscript.h"

class ExternalScriptItem;

class EditExternalScript
    : public QDialog
    , private Ui::EditExternalScriptBase
{
    Q_OBJECT

public:
    explicit EditExternalScript(ExternalScriptItem* item, QWidget* parent = nullptr);
    ~EditExternalScript() override;

private Q_SLOTS:
    void save();
    void validate();

private:
    ExternalScriptItem* m_item;
};

#endif // KDEVPLATFORM_PLUGIN_EDITEXTERNALSCRIPT_H
