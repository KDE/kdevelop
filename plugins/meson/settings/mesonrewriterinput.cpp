/* This file is part of KDevelop
    Copyright 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "mesonrewriterinput.h"
#include "mesonoptionbaseview.h"
#include "rewriter/mesonkwargsinfo.h"
#include "rewriter/mesonkwargsmodify.h"
#include "ui_mesonrewriterinput.h"
#include "ui_mesonrewriteroptioncontainer.h"

#include <KColorScheme>
#include <QLineEdit>
#include <debug.h>

MesonRewriterInputBase::MesonRewriterInputBase(QString name, QString kwarg, QWidget* parent)
    : QWidget(parent)
    , m_name(name)
    , m_kwarg(kwarg)
{
    m_ui = new Ui::MesonRewriterInputBase;
    m_ui->setupUi(this);
    m_ui->l_name->setText(m_name + QStringLiteral(":"));

    connect(this, &MesonRewriterInputBase::configChanged, this, &MesonRewriterInputBase::updateUi);
}

MesonRewriterInputBase::~MesonRewriterInputBase() {}

int MesonRewriterInputBase::nameWidth()
{
    return m_ui->l_name->fontMetrics().boundingRect(m_ui->l_name->text()).width() + 25;
}

void MesonRewriterInputBase::setMinNameWidth(int width)
{
    m_ui->l_name->setMinimumWidth(width);
}

void MesonRewriterInputBase::setInputWidget(QWidget* input)
{
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(input->sizePolicy().hasHeightForWidth());
    input->setSizePolicy(sizePolicy);
    m_ui->layout->insertWidget(1, input);
    updateUi();
}

void MesonRewriterInputBase::updateUi()
{
    KColorScheme scheme(QPalette::Normal);
    KColorScheme::ForegroundRole role;

    if (hasChanged()) {
        m_ui->l_name->setStyleSheet(QStringLiteral("font-weight: bold"));
        m_ui->b_reset->setDisabled(false || !m_enabled);
        role = KColorScheme::NeutralText;
    } else {
        m_ui->l_name->setStyleSheet(QString());
        m_ui->b_reset->setDisabled(true);
        role = KColorScheme::NormalText;
    }

    role = m_enabled ? role : KColorScheme::InactiveText;

    QPalette pal = m_ui->l_name->palette();
    pal.setColor(QPalette::Foreground, scheme.foreground(role).color());
    m_ui->l_name->setPalette(pal);

    m_ui->l_name->setDisabled(!m_enabled);
    inputWidget()->setDisabled(!m_enabled);
    m_ui->b_add->setHidden(m_enabled);
    m_ui->b_delete->setHidden(!m_enabled);
}

void MesonRewriterInputBase::reset()
{
    doReset();
    emit configChanged();
}

void MesonRewriterInputBase::remove()
{
    m_enabled = false;
    reset();
}

void MesonRewriterInputBase::add()
{
    m_enabled = true;
    reset();
}

void MesonRewriterInputBase::resetFromAction(MesonKWARGSInfo* action)
{
    resetValue(action->get(m_kwarg));
    m_default_enabled = m_enabled = action->hasKWARG(m_kwarg);
    if (m_enabled) {
        add();
    } else {
        remove();
    }
}

void MesonRewriterInputBase::writeToAction(MesonKWARGSModify* action)
{
    action->set(m_kwarg, value());
}

bool MesonRewriterInputBase::hasChanged() const
{
    return hasValueChanged() || (m_default_enabled != m_enabled);
}

bool MesonRewriterInputBase::isEnabled() const
{
    return m_enabled;
}

// String input class

MesonRewriterInputString::MesonRewriterInputString(QString name, QString kwarg, QWidget* parent)
    : MesonRewriterInputBase(name, kwarg, parent)
{
    m_lineEdit = new QLineEdit(this);
    connect(m_lineEdit, &QLineEdit::textChanged, this, [=]() { emit configChanged(); });
    setInputWidget(m_lineEdit);
}

MesonRewriterInputString::~MesonRewriterInputString() {}

MesonRewriterInputBase::Type MesonRewriterInputString::type() const
{
    return STRING;
}

bool MesonRewriterInputString::hasValueChanged() const
{
    return m_lineEdit->text() != m_initialValue;
}

QWidget* MesonRewriterInputString::inputWidget()
{
    return m_lineEdit;
}

void MesonRewriterInputString::doReset()
{
    m_lineEdit->setText(m_initialValue);
}

void MesonRewriterInputString::resetValue(QJsonValue val)
{
    m_initialValue = val.toString();
}

QJsonValue MesonRewriterInputString::value()
{
    return QJsonValue(m_lineEdit->text());
}

// Options container

MesonRewriterOptionContainer::MesonRewriterOptionContainer(MesonOptViewPtr optView, QWidget* parent)
    : QWidget(parent)
    , m_optView(optView)
{
    m_ui = new Ui::MesonRewriterOptionContainer;
    m_ui->setupUi(this);
    m_ui->h_layout->insertWidget(0, m_optView.get());

    connect(optView.get(), &MesonOptionBaseView::configChanged, this, [this]() { emit configChanged(); });
}

void MesonRewriterOptionContainer::deleteMe()
{
    m_markedForDeletion = true;
    emit configChanged();
}

bool MesonRewriterOptionContainer::shouldDelete() const
{
    return m_markedForDeletion;
}

bool MesonRewriterOptionContainer::hasChanged() const
{
    return m_optView->option()->isUpdated();
}

MesonOptViewPtr MesonRewriterOptionContainer::view()
{
    return m_optView;
}
