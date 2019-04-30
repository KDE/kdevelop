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

#include <QJsonValue>
#include <QWidget>

namespace Ui
{
class MesonRewriterInputBase;
}

class QLineEdit;

class MesonKWARGSInfo;
class MesonKWARGSModify;

class MesonRewriterInputBase : public QWidget
{
    Q_OBJECT

public:
    enum Type { STRING };

public:
    explicit MesonRewriterInputBase(QString name, QString kwarg, QWidget* parent);
    virtual ~MesonRewriterInputBase();

    int nameWidth();
    void setMinNameWidth(int width);
    bool isEnabled() const;
    bool hasChanged() const;

    virtual Type type() const = 0;

    void resetFromAction(MesonKWARGSInfo* action);
    void writeToAction(MesonKWARGSModify* action);

protected:
    void setInputWidget(QWidget* input);

    virtual bool hasValueChanged() const = 0;
    virtual void doReset() = 0;
    virtual QWidget* inputWidget() = 0;
    virtual void resetValue(QJsonValue val) = 0;
    virtual QJsonValue value() = 0;

public Q_SLOTS:
    void reset();
    void remove();
    void add();
    void updateUi();

Q_SIGNALS:
    void configChanged();

private:
    Ui::MesonRewriterInputBase* m_ui = nullptr;
    QString m_name;
    QString m_kwarg;
    bool m_enabled = false;
    bool m_default_enabled = false;
};

class MesonRewriterInputString : public MesonRewriterInputBase
{
    Q_OBJECT

public:
    explicit MesonRewriterInputString(QString name, QString kwarg, QWidget* parent);
    virtual ~MesonRewriterInputString();

    MesonRewriterInputBase::Type type() const override;

protected:
    void doReset() override;
    QWidget* inputWidget() override;
    bool hasValueChanged() const override;
    void resetValue(QJsonValue val) override;
    QJsonValue value() override;

private:
    QString m_initialValue;
    QLineEdit* m_lineEdit = nullptr;
};
