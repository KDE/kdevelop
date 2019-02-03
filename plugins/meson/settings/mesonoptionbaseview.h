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

#pragma once

#include "mesonoptions.h"

#include <QWidget>

namespace Ui
{
class MesonOptionBaseView;
}

class QCheckBox;
class QComboBox;
class QLineEdit;
class QPushButton;
class QSpinBox;

class MesonOptionBaseView : public QWidget
{
    Q_OBJECT

public:
    explicit MesonOptionBaseView(MesonOptionPtr option, QWidget* parent);
    virtual ~MesonOptionBaseView();

    int nameWidth();
    void setMinNameWidth(int width);

    virtual MesonOptionBase* option() = 0;
    virtual void updateInput() = 0;

    void setInputWidget(QWidget* input);

protected:
    void setChanged(bool changed);

public Q_SLOTS:
    void reset();

Q_SIGNALS:
    void configChanged();

private:
    Ui::MesonOptionBaseView* m_ui = nullptr;
};

class MesonOptionArrayView : public MesonOptionBaseView
{
    Q_OBJECT

public:
    MesonOptionArrayView(MesonOptionPtr option, QWidget* parent);

    MesonOptionBase* option() override;
    void updateInput() override;

private:
    std::shared_ptr<MesonOptionArray> m_option;
    QPushButton* m_input = nullptr;
};

class MesonOptionBoolView : public MesonOptionBaseView
{
    Q_OBJECT

public:
    MesonOptionBoolView(MesonOptionPtr option, QWidget* parent);

    MesonOptionBase* option() override;
    void updateInput() override;

public Q_SLOTS:
    void updated();

private:
    std::shared_ptr<MesonOptionBool> m_option;
    QCheckBox* m_input = nullptr;
};

class MesonOptionComboView : public MesonOptionBaseView
{
    Q_OBJECT

public:
    MesonOptionComboView(MesonOptionPtr option, QWidget* parent);

    MesonOptionBase* option() override;
    void updateInput() override;

public Q_SLOTS:
    void updated();

private:
    std::shared_ptr<MesonOptionCombo> m_option;
    QComboBox* m_input = nullptr;
};

class MesonOptionIntegerView : public MesonOptionBaseView
{
    Q_OBJECT

public:
    MesonOptionIntegerView(MesonOptionPtr option, QWidget* parent);

    MesonOptionBase* option() override;
    void updateInput() override;

public Q_SLOTS:
    void updated();

private:
    std::shared_ptr<MesonOptionInteger> m_option;
    QSpinBox* m_input = nullptr;
};

class MesonOptionStringView : public MesonOptionBaseView
{
    Q_OBJECT

public:
    MesonOptionStringView(MesonOptionPtr option, QWidget* parent);

    MesonOptionBase* option() override;
    void updateInput() override;

public Q_SLOTS:
    void updated();

private:
    std::shared_ptr<MesonOptionString> m_option;
    QLineEdit* m_input = nullptr;
};
