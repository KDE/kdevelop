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

#include "compilerprovider/settingsmanager.h"

#include <util/path.h>

namespace
{

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

bool isCustomParserArguments(const QString& arguments, const QStringList& standards)
{
    const auto defaultArguments = SettingsManager::globalInstance()->defaultParserArguments();

    auto standard = languageStandard(arguments);

    auto tmpArgs(arguments);
    tmpArgs.replace(standard, "c++11");

    if (tmpArgs == defaultArguments.cppArguments && standards.contains(standard)) {
        return false;
    }

    return true;
}

const int customProfileIdx = 0;
}

ParserWidget::ParserWidget(QWidget* parent)
    : QWidget(parent)
    , m_ui(new Ui::ParserWidget())
{
    m_ui->setupUi(this);

    connect(m_ui->parserOptionsC, &QLineEdit::textEdited, this, &ParserWidget::textEdited);
    connect(m_ui->parserOptionsCpp, &QLineEdit::textEdited, this, &ParserWidget::textEdited);
    connect(m_ui->parseHeadersInPlainC, &QCheckBox::stateChanged, this, &ParserWidget::textEdited);
    connect(m_ui->languageStandardsC, static_cast<void(QComboBox::*)(const
QString&)>(&QComboBox::activated), this,
&ParserWidget::languageStandardChangedC);
    connect(m_ui->languageStandardsCpp, static_cast<void(QComboBox::*)(const
QString&)>(&QComboBox::activated), this,
&ParserWidget::languageStandardChangedCpp);

    updateEnablements();
}

ParserWidget::~ParserWidget() = default;

void ParserWidget::textEdited()
{
    emit changed();
}

void ParserWidget::languageStandardChangedC(const QString& standard)
{
    if (m_ui->languageStandardsC->currentIndex() == customProfileIdx) {
        m_ui->parserOptionsC->setText(SettingsManager::globalInstance()->defaultParserArguments().cArguments);
    } else {
        auto text = SettingsManager::globalInstance()->defaultParserArguments().cArguments;
        auto currentStandard = languageStandard(text);
        m_ui->parserOptionsC->setText(text.replace(currentStandard, standard));
    }

    textEdited();
    updateEnablements();
}

void ParserWidget::languageStandardChangedCpp(const QString& standard)
{
    if (m_ui->languageStandardsCpp->currentIndex() == customProfileIdx) {
        m_ui->parserOptionsCpp->setText(SettingsManager::globalInstance()->defaultParserArguments().cppArguments);
    } else {
        auto text = SettingsManager::globalInstance()->defaultParserArguments().cppArguments;
        auto currentStandard = languageStandard(text);
        m_ui->parserOptionsCpp->setText(text.replace(currentStandard, standard));
    }

    textEdited();
    updateEnablements();
}

void ParserWidget::setParserArguments(const ParserArguments& arguments)
{
    auto setArguments = [this](QComboBox* languageStandards, QLineEdit* parserOptions, const QString& arguments) {
        QStringList standards;
        for (int i = 1; i < languageStandards->count(); i++) {
            standards << languageStandards->itemText(i);
        }

        if (isCustomParserArguments(arguments, standards)) {
            languageStandards->setCurrentIndex(customProfileIdx);
        } else {
            languageStandards->setCurrentText(languageStandard(arguments));
        }

        parserOptions->setText(arguments);
    };

    setArguments(m_ui->languageStandardsCpp, m_ui->parserOptionsCpp, arguments.cppArguments);
    setArguments(m_ui->languageStandardsC, m_ui->parserOptionsC, arguments.cArguments);

    m_ui->parseHeadersInPlainC->setChecked(!arguments.parseAmbiguousAsCPP);

    updateEnablements();
}

ParserArguments ParserWidget::parserArguments() const
{
    return {m_ui->parserOptionsC->text(), m_ui->parserOptionsCpp->text(), !m_ui->parseHeadersInPlainC->isChecked()};
}

void ParserWidget::updateEnablements()
{
    if (m_ui->languageStandardsCpp->currentIndex() == customProfileIdx) {
        m_ui->parserOptionsCpp->setEnabled(true);
    } else {
        m_ui->parserOptionsCpp->setEnabled(false);
    }

    if (m_ui->languageStandardsC->currentIndex() == customProfileIdx) {
        m_ui->parserOptionsC->setEnabled(true);
    } else {
        m_ui->parserOptionsC->setEnabled(false);
    }
}
