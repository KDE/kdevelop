 /**************************************************************************
 *   Copyright 2016 Artur Puzio <cytadela88@gmail.com>                     *
 *   Copyright 2016 Kevin Funk <kfunk@kde.org>                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

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
