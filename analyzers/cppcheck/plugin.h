/* This file is part of KDevelop
   Copyright 2013 Christoph Thielecke <crissi99@gmx.de>
   Copyright 2016-2017 Anton Anikin <anton.anikin@htower.ru>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef CPPCHECK_PLUGIN_H
#define CPPCHECK_PLUGIN_H

#include "job.h"

#include <interfaces/iplugin.h>
#include <QScopedPointer>

class KJob;

namespace KDevelop
{
class IProject;
}

namespace cppcheck
{

class ProblemModel;

class Plugin : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    explicit Plugin(QObject* parent, const QVariantList& = QVariantList());

    ~Plugin() override;

    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context) override;

    int configPages() const override { return 1; }
    KDevelop::ConfigPage* configPage(int number, QWidget* parent) override;

    int perProjectConfigPages() const override { return 1; }
    KDevelop::ConfigPage* perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent) override;

    void runCppcheck(KDevelop::IProject* project, const QString& path);
    bool isRunning();

private:
    void killCppcheck();

    void raiseProblemsView();
    void raiseOutputView();

    void updateActions();
    void projectClosed(KDevelop::IProject* project);

    void runCppcheck(bool checkProject);

    void result(KJob* job);

    Job* m_job;

    KDevelop::IProject* m_currentProject;

    QAction* m_actionFile;
    QAction* m_actionProject;
    QAction* m_actionProjectItem;

    QScopedPointer<ProblemModel> m_model;
};

}

#endif
