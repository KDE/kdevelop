/*
 * This file is part of KDevelop
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "configwidget.h"
#include "ui_veritasconfig.h"

#include <KIcon>
#include <QToolButton>
#include <QLabel>

using Veritas::ConfigWidget;

ConfigWidget::ConfigWidget(QWidget* parent)
    : QWidget(parent)
{
    m_ui = new Ui::VeritasConfig;
    m_ui->setupUi(this);
    setupButtons();
}

ConfigWidget::~ConfigWidget()
{
}

void ConfigWidget::expandDetails(bool checked)
{
    KIcon icon(checked ? "arrow-up-double" : "arrow-down-double");
    expandDetailsButton()->setIcon(icon);
    QLabel* l = new QLabel;
    l->setText("ALLO WORLD");
    m_ui->frameworkLayout->addWidget(l);
    l->show();
    repaint();
}

void ConfigWidget::setupButtons()
{
    expandDetailsButton()->setToolButtonStyle( Qt::ToolButtonIconOnly );
    expandDetailsButton()->setIcon(KIcon("arrow-down-double"));
    expandDetailsButton()->setCheckable(true);
    expandDetailsButton()->setChecked(false);
    connect(expandDetailsButton(), SIGNAL(toggled(bool)), SLOT(expandDetails(bool)));

    addExecutableButton()->setToolButtonStyle( Qt::ToolButtonIconOnly );
    addExecutableButton()->setIcon(KIcon("list-add"));

    removeExecutableButton()->setToolButtonStyle( Qt::ToolButtonIconOnly );
    removeExecutableButton()->setIcon(KIcon("list-remove"));
}

void ConfigWidget::fto_clickExpandDetails() const
{
    expandDetailsButton()->toggle();
}

QToolButton* ConfigWidget::expandDetailsButton() const
{
    return m_ui->expandDetails;
}

QToolButton* ConfigWidget::addExecutableButton() const
{
    return m_ui->addExecutable;
}

QToolButton* ConfigWidget::removeExecutableButton() const
{
    return m_ui->removeExecutable;
}

#include "configwidget.moc"
