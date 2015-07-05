/*
 * This file is part of KDevelop
 *
 * Copyright 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "parserwidget.h"

#include "ui_parserwidget.h"

#include <util/path.h>

QString languageStandard(const QString& arguments)
{
    int idx = arguments.indexOf("-std=");
    if(idx == -1){
        return QStringLiteral("c++11");
    }

    idx += 5;
    int end = arguments.indexOf(' ', idx) != -1 ? arguments.indexOf(' ', idx) : arguments.size();
    return arguments.mid(idx, end - idx);
}

ParserWidget::ParserWidget(QWidget* parent)
    : QWidget(parent)
    , m_ui(new Ui::ParserWidget())
{
    m_ui->setupUi(this);

    connect(m_ui->parserOptions, &QLineEdit::textEdited, this, &ParserWidget::textEdited);

    connect(m_ui->languageStandards, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::activated), this, &ParserWidget::languageStandardChanged);
}

ParserWidget::~ParserWidget() = default;

void ParserWidget::textEdited()
{
    const auto parserOptions = m_ui->parserOptions->text();
    m_ui->languageStandards->setCurrentText(languageStandard(m_ui->parserOptions->text()));

    emit changed();
}

void ParserWidget::languageStandardChanged(const QString& standard)
{
    auto text = m_ui->parserOptions->text();

    auto currentStandard = languageStandard(text);

    m_ui->parserOptions->setText(text.replace(currentStandard, standard));

    textEdited();
}

void ParserWidget::setParserArguments(const QString& arguments)
{
    m_ui->parserOptions->setText(arguments);
    m_ui->languageStandards->setCurrentText(languageStandard(arguments));
}

QString ParserWidget::parserArguments() const
{
    return m_ui->parserOptions->text();
}
