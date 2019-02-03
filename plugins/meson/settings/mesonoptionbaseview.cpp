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

#include "mesonoptionbaseview.h"

#include "mesonlisteditor.h"
#include "ui_mesonoptionbaseview.h"
#include <debug.h>

#include <KColorScheme>

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPalette>
#include <QPushButton>
#include <QSpinBox>
#include <QtGlobal>

using namespace std;

// Helper class for RAII signal blocking
class SignalBlocker
{
public:
    SignalBlocker(QWidget* widget)
        : m_widget(widget)
    {
        if (m_widget) {
            m_widget->blockSignals(true);
        }
    }

    ~SignalBlocker()
    {
        if (m_widget) {
            m_widget->blockSignals(false);
        }
    }

private:
    QWidget* m_widget = nullptr;
};

MesonOptionBaseView::MesonOptionBaseView(MesonOptionPtr option, QWidget* parent)
    : QWidget(parent)
{
    Q_ASSERT(option);

    m_ui = new Ui::MesonOptionBaseView;
    m_ui->setupUi(this);

    m_ui->l_name->setText(option->name() + QStringLiteral(":"));
    m_ui->l_name->setToolTip(option->description());
    setToolTip(option->description());
}

MesonOptionBaseView::~MesonOptionBaseView()
{
    if (m_ui) {
        delete m_ui;
    }
}

// Base class functions

int MesonOptionBaseView::nameWidth()
{
    return m_ui->l_name->fontMetrics().boundingRect(m_ui->l_name->text()).width() + 25;
}

void MesonOptionBaseView::setMinNameWidth(int width)
{
    m_ui->l_name->setMinimumWidth(width);
}

void MesonOptionBaseView::setInputWidget(QWidget* input)
{
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(input->sizePolicy().hasHeightForWidth());
    input->setSizePolicy(sizePolicy);
    input->setToolTip(option()->description());
    m_ui->layout->insertWidget(1, input);
    updateInput();
    setChanged(false);
}

void MesonOptionBaseView::reset()
{
    option()->reset();
    updateInput();
    setChanged(false);
}

void MesonOptionBaseView::setChanged(bool changed)
{
    KColorScheme scheme(QPalette::Normal);
    KColorScheme::ForegroundRole role;

    if (changed) {
        m_ui->l_name->setStyleSheet(QStringLiteral("font-weight: bold"));
        m_ui->b_reset->setDisabled(false);
        role = KColorScheme::NeutralText;
    } else {
        m_ui->l_name->setStyleSheet(QString());
        m_ui->b_reset->setDisabled(true);
        role = KColorScheme::NormalText;
    }

    QPalette pal = m_ui->l_name->palette();
    pal.setColor(QPalette::Foreground, scheme.foreground(role).color());
    m_ui->l_name->setPalette(pal);
    emit configChanged();
}

// Derived class constructors

MesonOptionArrayView::MesonOptionArrayView(MesonOptionPtr option, QWidget* parent)
    : MesonOptionBaseView(option, parent)
    , m_option(dynamic_pointer_cast<MesonOptionArray>(option))
{
    Q_ASSERT(m_option);

    m_input = new QPushButton(this);
    connect(m_input, &QPushButton::clicked, this, [this]() {
        MesonListEditor editor(m_option->rawValue(), this);
        if (editor.exec() == QDialog::Accepted) {
            m_option->setValue(editor.content());
            m_input->setText(m_option->value());
            setChanged(m_option->isUpdated());
        }
    });
    setInputWidget(m_input);
}

MesonOptionBoolView::MesonOptionBoolView(MesonOptionPtr option, QWidget* parent)
    : MesonOptionBaseView(option, parent)
    , m_option(dynamic_pointer_cast<MesonOptionBool>(option))
{
    Q_ASSERT(m_option);

    m_input = new QCheckBox(this);
    connect(m_input, &QCheckBox::stateChanged, this, &MesonOptionBoolView::updated);
    setInputWidget(m_input);
}

MesonOptionComboView::MesonOptionComboView(MesonOptionPtr option, QWidget* parent)
    : MesonOptionBaseView(option, parent)
    , m_option(dynamic_pointer_cast<MesonOptionCombo>(option))
{
    Q_ASSERT(m_option);

    m_input = new QComboBox(this);
    m_input->clear();
    m_input->addItems(m_option->choices());
    m_input->setEditable(false);
    connect(m_input, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MesonOptionComboView::updated);
    setInputWidget(m_input);
}

MesonOptionIntegerView::MesonOptionIntegerView(MesonOptionPtr option, QWidget* parent)
    : MesonOptionBaseView(option, parent)
    , m_option(dynamic_pointer_cast<MesonOptionInteger>(option))
{
    Q_ASSERT(m_option);

    m_input = new QSpinBox(this);
    m_input->setMinimum(INT32_MIN);
    m_input->setMaximum(INT32_MAX);
    connect(m_input, QOverload<int>::of(&QSpinBox::valueChanged), this, &MesonOptionIntegerView::updated);
    setInputWidget(m_input);
}

MesonOptionStringView::MesonOptionStringView(MesonOptionPtr option, QWidget* parent)
    : MesonOptionBaseView(option, parent)
    , m_option(dynamic_pointer_cast<MesonOptionString>(option))
{
    Q_ASSERT(m_option);

    m_input = new QLineEdit(this);
    connect(m_input, &QLineEdit::textChanged, this, &MesonOptionStringView::updated);
    setInputWidget(m_input);
}

// Option getters

MesonOptionBase* MesonOptionArrayView::option()
{
    return m_option.get();
}

MesonOptionBase* MesonOptionBoolView::option()
{
    return m_option.get();
}

MesonOptionBase* MesonOptionComboView::option()
{
    return m_option.get();
}

MesonOptionBase* MesonOptionIntegerView::option()
{
    return m_option.get();
}

MesonOptionBase* MesonOptionStringView::option()
{
    return m_option.get();
}

// Updaters for the input widget

void MesonOptionArrayView::updateInput()
{
    SignalBlocker block(m_input);
    m_input->setText(m_option->value());
}

void MesonOptionBoolView::updateInput()
{
    SignalBlocker block(m_input);
    m_input->setCheckState(m_option->rawValue() ? Qt::Checked : Qt::Unchecked);
}

void MesonOptionComboView::updateInput()
{
    SignalBlocker block(m_input);
    m_input->setCurrentText(m_option->rawValue());
}

void MesonOptionIntegerView::updateInput()
{
    SignalBlocker block(m_input);
    m_input->setValue(m_option->rawValue());
}

void MesonOptionStringView::updateInput()
{
    SignalBlocker block(m_input);
    m_input->setText(m_option->rawValue());
}

// Slots to update the option value

void MesonOptionBoolView::updated()
{
    m_option->setValue(m_input->isChecked());
    setChanged(m_option->isUpdated());
}

void MesonOptionComboView::updated()
{
    m_option->setValue(m_input->currentText());
    setChanged(m_option->isUpdated());
}

void MesonOptionIntegerView::updated()
{
    m_option->setValue(m_input->value());
    setChanged(m_option->isUpdated());
}

void MesonOptionStringView::updated()
{
    m_option->setValue(m_input->text());
    setChanged(m_option->isUpdated());
}
