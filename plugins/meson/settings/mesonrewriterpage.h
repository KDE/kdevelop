/*
    SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QAbstractButton>
#include <QVector>
#include <interfaces/configpage.h>

#include "mesonrewriterinput.h"
#include "mintro/mesonoptions.h"

namespace KDevelop
{
class IPlugin;
class IProject;
}

namespace Ui
{
class MesonRewriterPage;
}

class MesonRewriterPage : public KDevelop::ConfigPage
{
    Q_OBJECT
public:
    enum State { START, LOADING, WRITING, READY, ERROR };

public:
    explicit MesonRewriterPage(KDevelop::IPlugin* plugin, KDevelop::IProject* project, QWidget* parent = nullptr);

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

public Q_SLOTS:
    void apply() override;
    void defaults() override;
    void reset() override;

    void emitChanged();
    void recalculateLengths();
    void newOption();

private:
    void setWidgetsDisabled(bool disabled);
    void checkStatus();
    void setStatus(State s);

    QVector<MesonRewriterInputBase*> constructPojectInputs();
    MesonOptContainerPtr constructDefaultOpt(const QString& name, const QString& value);

private:
    KDevelop::IProject* m_project = nullptr;
    Ui::MesonRewriterPage* m_ui = nullptr;
    bool m_configChanged = false;
    State m_state = START;
    MesonOptsPtr m_opts = nullptr;

    QVector<MesonRewriterInputBase*> m_projectKwargs;
    QVector<MesonOptContainerPtr> m_defaultOpts;

    QStringList m_initialDefaultOpts;
};
