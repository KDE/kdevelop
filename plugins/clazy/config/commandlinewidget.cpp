/*
    SPDX-FileCopyrightText: 2018 Anton Anikin <anton@anikin.xyz>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "commandlinewidget.h"
#include "ui_commandlinewidget.h"

#include <QFontDatabase>
#include <QLineEdit>

namespace Clazy
{

CommandLineWidget::CommandLineWidget(QWidget* parent)
    : QWidget(parent)
    , m_ui(new Ui::CommandLineWidget)
{
    m_ui->setupUi(this);
    m_ui->cmdEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    connect(m_ui->cmdFilter, &QLineEdit::textChanged, this, &CommandLineWidget::updateCommandLine);
    connect(m_ui->cmdBreak, &QCheckBox::stateChanged, this, &CommandLineWidget::updateCommandLine);
}

CommandLineWidget::~CommandLineWidget() = default;

void CommandLineWidget::setText(const QString& text)
{
    if (m_text != text) {
        m_text = text;
        updateCommandLine();
    }
}

void CommandLineWidget::updateCommandLine()
{
    auto commandLine = m_text;
    if (m_ui->cmdBreak->isChecked()) {
        commandLine.replace(QLatin1String(" -"), QLatin1String("\n-"));
        commandLine.replace(QLatin1String(","), QLatin1String("\n,"));
    }

    auto filterText = m_ui->cmdFilter->text();
    if (!filterText.isEmpty()) {
        QStringList lines = commandLine.split(QLatin1Char('\n'));
        QMutableStringListIterator i(lines);

        while (i.hasNext()) {
            if (!i.next().contains(filterText)) {
                i.remove();
            }
        }

        commandLine = lines.join(QLatin1Char('\n'));
    }

    m_ui->cmdEdit->setPlainText(commandLine);
}

}
