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

    connect(m_ui->findText, &QLineEdit::textEdited,
            this, &DocumentationFindWidget::emitDataChanged);
    connect(m_ui->matchCase, &QAbstractButton::toggled,
            this, &DocumentationFindWidget::emitDataChanged);
    connect(m_ui->findText, &QLineEdit::returnPressed,
            this, &DocumentationFindWidget::searchNext);
    connect(m_ui->nextButton, &QToolButton::clicked,
            this, &DocumentationFindWidget::searchNext);
    connect(m_ui->previousButton, &QToolButton::clicked,
            this, &DocumentationFindWidget::searchPrevious);
    // TODO: disable next/previous buttons if no (more) search hits, color coding in text field
}

DocumentationFindWidget::~DocumentationFindWidget()
{
    delete m_ui;
}

void KDevelop::DocumentationFindWidget::searchNext()
{
    FindOptions opts=Next;
    if (m_ui->matchCase->isChecked())
        opts |= MatchCase;

    emit searchRequested(m_ui->findText->text(), opts);
}

void KDevelop::DocumentationFindWidget::searchPrevious()
{
    FindOptions opts=Previous;
    if (m_ui->matchCase->isChecked())
        opts |= MatchCase;

    emit searchRequested(m_ui->findText->text(), opts);
}

void KDevelop::DocumentationFindWidget::startSearch()
{
    show();
    m_ui->findText->selectAll();
    m_ui->findText->setFocus();
}

void DocumentationFindWidget::emitDataChanged()
{
    FindOptions opts;
    if (m_ui->matchCase->isChecked())
        opts |= MatchCase;

    emit searchDataChanged(m_ui->findText->text(), opts);
}

void KDevelop::DocumentationFindWidget::hideEvent(QHideEvent* event)
{
    emit searchFinished();
    QWidget::hideEvent(event);
}
