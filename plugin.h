/* This file is part of KDevelop
 *  Copyright 2013 Christoph Thielecke <crissi99@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

 */

#ifndef CPPCHECKPLUGIN_H
#define CPPCHECKPLUGIN_H

#include <QPointer>
#include <QVariant>

#include <interfaces/icore.h>
#include <interfaces/iplugin.h>
#include <interfaces/istatus.h>

#include <interfaces/iuicontroller.h>
#include <interfaces/contextmenuextension.h>

class KJob;
class QTreeView;

namespace KDevelop
{
class ProblemModel;
}

namespace cppcheck
{
class Control;

class Plugin : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    Plugin(QObject *parent, const QVariantList & = QVariantList());

    ~Plugin() override;

    void unload() override;

    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context) override;

    int configPages() const override { return 1; }
    KDevelop::ConfigPage* configPage(int number, QWidget *parent) override;

    int perProjectConfigPages() const override { return 1; }
    KDevelop::ConfigPage* perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions &options, QWidget *parent) override;

private slots:
    void loadOutput();
    void runCppcheck(bool allFiles);
    void runCppcheckFile();
    void runCppcheckAll();
    void result(KJob *job);

private:
    QString m_lastExec, m_lastParams, m_lastValExec, m_lastValParams,
            m_lastCtExec, m_lastCtParams, m_lastKcExec;

    QScopedPointer<KDevelop::ProblemModel> m_model;
};

}


#endif // CPPCHECKPLUGIN_H
