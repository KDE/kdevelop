/*
    SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "mintro/mesonoptions.h"

#include <util/path.h>

#include <QVector>
#include <QWidget>

#include <memory>

namespace Ui
{
class MesonOptionsView;
}

namespace KDevelop
{
class IProject;
}

namespace Meson
{
struct BuildDir;
}

class MesonOptionBaseView;
class MesonIntrospectJob;
class KJob;

using OPT_VIEW_PTR = std::shared_ptr<MesonOptionBaseView>;

class MesonOptionsView : public QWidget
{
    Q_OBJECT

public:
    explicit MesonOptionsView(QWidget* parent = nullptr);
    virtual ~MesonOptionsView();

    void clear();
    void resetAll();
    KJob* repopulateFromBuildDir(KDevelop::IProject* project, const Meson::BuildDir& buildDir);
    KJob* repopulateFromMesonFile(KDevelop::IProject* project, KDevelop::Path mesonExe);

    MesonOptsPtr options();

public Q_SLOTS:
    void emitChanged();

Q_SIGNALS:
    void configChanged();

private:
    Ui::MesonOptionsView* m_ui = nullptr;
    QVector<OPT_VIEW_PTR> m_optViews;
    MesonOptsPtr m_options;

    KJob* repopulate(MesonIntrospectJob* introJob);
};
