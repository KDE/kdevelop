/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "documentationfindwidget.h"
#include "ui_documentationfindwidget.h"

#include <QGuiApplication>

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
    connect(m_ui->findText, &QLineEdit::returnPressed, this, [this]() {
        if (QGuiApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
            searchPrevious();
        } else {
            searchNext();
        }
    });
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

#include "moc_documentationfindwidget.cpp"
