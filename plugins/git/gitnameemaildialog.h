/*
    SPDX-FileCopyrightText: 2016 Artur Puzio <cytadela88@gmail.com>
    SPDX-FileCopyrightText: 2016 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_GIT_NAMEEMAILDIALOG_H
#define KDEVPLATFORM_PLUGIN_GIT_NAMEEMAILDIALOG_H

#include <QDialog>

namespace Ui { class GitNameEmailDialog; }

class GitNameEmailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GitNameEmailDialog(QWidget *parent = nullptr);
    ~GitNameEmailDialog() override;

    QString name() const;
    void setName(const QString& name);
    QString email() const;
    void setEmail(const QString& email);

    bool isGlobal() const;

private Q_SLOTS:
    void updateUi();

private:
    QScopedPointer<Ui::GitNameEmailDialog> ui;
};

#endif //KDEVPLATFORM_PLUGIN_GIT_NAMEEMAILDIALOG_H
