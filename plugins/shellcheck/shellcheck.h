/*
 * This file is part of KDevelop
 * Copyright 2020  Morten Danielsen Volden <mvolden2@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SHELLCHECK_H
#define SHELLCHECK_H

#include "job.h"

#include <interfaces/iplugin.h>
#include <shell/problemmodel.h>


class QMimeType;

namespace KDevelop
{
class IProject;
}

namespace shellcheck {

class ShellCheck : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    // KPluginFactory-based plugin wants constructor with this signature
    ShellCheck(QObject* parent, const QVariantList&);

    ~ShellCheck() override;
    
    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;

    void runShellcheck(const QString& path);
    bool isRunning() const;
    
    int configPages() const override;
    KDevelop::ConfigPage* configPage(int number, QWidget* parent) override;

private:
    void killShellcheckJob();
    bool isSupportedMimeType(const QMimeType& mimeType) const;
    
    void raiseProblemsView();
    void raiseOutputView();
    
    void updateActions();
    void runShellcheck();
    
    void result(KJob* job);
    
    Job* m_job;

    QAction* m_contextActionFile;
    QAction* m_menuActionFile;

    QScopedPointer<KDevelop::ProblemModel> m_model;
};

}
#endif // SHELLCHECK_H
