/*
 * This file is part of KDevelop
 * Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "documentationfindwidget.h"
#include "ui_documentationfindwidget.h"

using namespace KDevelop;

DocumentationFindWidget::DocumentationFindWidget(QWidget* parent)
    : QWidget(parent)
{
    m_ui = new Ui::FindWidget;
    m_ui->setupUi(this);

    m_ui->hideButton->setIcon(QIcon::fromTheme("dialog-close"));
    m_ui->nextButton->setIcon(QIcon::fromTheme("go-down-search"));
    m_ui->previousButton->setIcon(QIcon::fromTheme("go-up-search"));

    connect(m_ui->findText, SIGNAL(returnPressed(QString)), SLOT(searchNext()));
    connect(m_ui->nextButton, SIGNAL(clicked(bool)), SLOT(searchNext()));
    connect(m_ui->previousButton, SIGNAL(clicked(bool)), SLOT(searchPrevious()));
}

DocumentationFindWidget::~DocumentationFindWidget()
{
    delete m_ui;
}

void KDevelop::DocumentationFindWidget::searchNext()
{
    FindOptions opts=Next;
    if(m_ui->matchCase->checkState()==Qt::Checked)
        opts |= MatchCase;

    emit newSearch(m_ui->findText->text(), opts);
}

void KDevelop::DocumentationFindWidget::searchPrevious()
{
    FindOptions opts=Previous;
    if(m_ui->matchCase->checkState()==Qt::Checked)
        opts |= MatchCase;

    emit newSearch(m_ui->findText->text(), opts);
}

void KDevelop::DocumentationFindWidget::showEvent(QShowEvent* e)
{
    m_ui->findText->setFocus();
    QWidget::showEvent(e);
}
