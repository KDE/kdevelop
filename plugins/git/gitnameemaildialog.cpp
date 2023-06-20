/*
    SPDX-FileCopyrightText: 2016 Artur Puzio <cytadela88@gmail.com>
    SPDX-FileCopyrightText: 2016 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "gitnameemaildialog.h"
#include "ui_gitnameemaildialog.h"

#include "gitplugin.h"

#include <KLocalizedString>

#include <QDialog>
#include <QPushButton>
#include <QRegularExpression>
#include <QValidator>

using namespace KDevelop;

GitNameEmailDialog::GitNameEmailDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::GitNameEmailDialog)
{
    ui->setupUi(this);

    ui->buttonBox->button(QDialogButtonBox::Save)->setDisabled(true);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &GitNameEmailDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &GitNameEmailDialog::reject);

    QRegularExpression rx(QStringLiteral(".+"));
    auto validator = new QRegularExpressionValidator(rx, this);
    ui->emailEdit->setValidator(validator);
    ui->nameEdit->setValidator(validator);

    connect(ui->emailEdit, &QLineEdit::textChanged, this, &GitNameEmailDialog::updateUi);
    connect(ui->nameEdit, &QLineEdit::textChanged, this, &GitNameEmailDialog::updateUi);
}

GitNameEmailDialog::~GitNameEmailDialog() = default;

void GitNameEmailDialog::updateUi()
{
    ui->buttonBox->button(QDialogButtonBox::Save)->setDisabled(
        !ui->nameEdit->hasAcceptableInput() || !ui->emailEdit->hasAcceptableInput());
}

void GitNameEmailDialog::setName(const QString& name)
{
    ui->nameEdit->setText(name);
}

void GitNameEmailDialog::setEmail(const QString& email)
{
    ui->emailEdit->setText(email);
}

QString GitNameEmailDialog::name() const
{
    return ui->nameEdit->text();
}

QString GitNameEmailDialog::email() const
{
    return ui->emailEdit->text();
}

bool GitNameEmailDialog::isGlobal() const
{
    return ui->globalCheckBox->isChecked();
}

#include "moc_gitnameemaildialog.cpp"
