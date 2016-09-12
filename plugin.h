/*************************************************************************************
 *  Copyright (C) 2016 by Carlos Nihelton <carlosnsoliveira@gmail.com>               *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#ifndef CLANGTIDYPLUGIN_H
#define CLANGTIDYPLUGIN_H

#include <QPointer>
#include <QVariant>

#include <interfaces/icore.h>
#include <interfaces/iplugin.h>
#include <interfaces/istatus.h>

#include <interfaces/contextmenuextension.h>
#include <interfaces/iuicontroller.h>

class KJob;
class QTreeView;

namespace KDevelop
{
class ProblemModel;
}

namespace ClangTidy
{
class Control;
/**
 * \class
 * \brief implements the support for clang-tidy inside KDevelop.
 */
class Plugin : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    Plugin(QObject* parent, const QVariantList& = QVariantList());

    ~Plugin() override;

    void unload() override;

    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context) override;

    int configPages() const override { return 1; }
    KDevelop::ConfigPage* configPage(int number, QWidget* parent) override;

    int perProjectConfigPages() const override { return 1; }
    KDevelop::ConfigPage* perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options,
                                               QWidget* parent) override;

    QStringList allAvailableChecks() { return m_allChecks; }
protected:
    void collectAllAvailableChecks(QUrl clangtidyPath);

private slots:
    void loadOutput();
    void runClangtidy(bool allFiles);
    void runClangtidyFile();
    void runClangtidyAll();
    void result(KJob* job);

private:
    QString m_lastExec, m_lastParams, m_lastValExec, m_lastValParams, m_lastCtExec, m_lastCtParams, m_lastKcExec;

    QScopedPointer<KDevelop::ProblemModel> m_model;
    QStringList m_allChecks;
    QStringList m_activeChecks;
};
}

#endif // CLANGTIDYPLUGIN_H
