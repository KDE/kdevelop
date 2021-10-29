/*
    SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QJsonValue>
#include <QWidget>
#include <memory>

class MesonOptionBaseView;
class MesonRewriterOptionContainer;
using MesonOptViewPtr = std::shared_ptr<MesonOptionBaseView>;
using MesonOptContainerPtr = std::shared_ptr<MesonRewriterOptionContainer>;

namespace Ui
{
class MesonRewriterInputBase;
class MesonRewriterOptionContainer;
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
    explicit MesonRewriterInputBase(const QString& name, const QString& kwarg, QWidget* parent);
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
    virtual void resetValue(const QJsonValue& val) = 0;
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
    explicit MesonRewriterInputString(const QString& name, const QString& kwarg, QWidget* parent);
    virtual ~MesonRewriterInputString();

    MesonRewriterInputBase::Type type() const override;

protected:
    void doReset() override;
    QWidget* inputWidget() override;
    bool hasValueChanged() const override;
    void resetValue(const QJsonValue& val) override;
    QJsonValue value() override;

private:
    QString m_initialValue;
    QLineEdit* m_lineEdit = nullptr;
};

class MesonRewriterOptionContainer : public QWidget
{
    Q_OBJECT

public:
    MesonRewriterOptionContainer(MesonOptViewPtr optView, QWidget* parent);

    bool shouldDelete() const;
    bool hasChanged() const;
    MesonOptViewPtr view();

public Q_SLOTS:
    void deleteMe();

Q_SIGNALS:
    void configChanged();

private:
    Ui::MesonRewriterOptionContainer* m_ui = nullptr;
    MesonOptViewPtr m_optView = nullptr;

    bool m_markedForDeletion = false;
};
