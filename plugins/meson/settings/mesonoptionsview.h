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
    KJob* repopulateFromBuildDir(KDevelop::IProject *project, Meson::BuildDir const& buildDir);
    KJob* repopulateFromMesonFile(KDevelop::IProject *project, KDevelop::Path mesonExe);

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
