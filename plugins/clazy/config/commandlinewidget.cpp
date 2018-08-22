/* This file is part of KDevelop

   Copyright 2018 Anton Anikin <anton@anikin.xyz>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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

    connect(m_ui->cmdFilter->lineEdit(), &QLineEdit::textChanged, this, &CommandLineWidget::updateCommandLine);
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

    auto filterText = m_ui->cmdFilter->lineEdit()->text();
    if (!filterText.isEmpty()) {
        QStringList lines = commandLine.split('\n');
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
