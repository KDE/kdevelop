/* KDevelop CMake Support
 *
 * Copyright 2006-2007 Aleix Pol <aleixpol@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef CMAKEBUILDER_H
#define CMAKEBUILDER_H

#include <interfaces/iplugin.h>
#include <QList>
#include <QVariantList>
#include <QUrl>
#include <project/interfaces/iprojectbuilder.h>

class QStringList;
class KDialog;

namespace KDevelop{
    class ProjectBaseItem;
}

/**
 * @author Aleix Pol
*/
class CMakeBuilder : public KDevelop::IPlugin, public KDevelop::IProjectBuilder
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IProjectBuilder )

public:
    explicit CMakeBuilder(QObject *parent = nullptr, const QVariantList &args = QVariantList());
    ~CMakeBuilder() override;

    KJob* build(KDevelop::ProjectBaseItem *dom) override;
    KJob* install(KDevelop::ProjectBaseItem *dom, const QUrl &installPrefix) override;
    KJob* clean(KDevelop::ProjectBaseItem *dom) override;
    KJob* configure(KDevelop::IProject*) override;
    KJob* prune(KDevelop::IProject*) override;

	QList< KDevelop::IProjectBuilder* > additionalBuilderPlugins( KDevelop::IProject* project ) const override;

//     bool updateConfig( KDevelop::IProject* project );

    int configPages() const override;
    KDevelop::ConfigPage* configPage(int number, QWidget* parent) override;

Q_SIGNALS:
    void built(KDevelop::ProjectBaseItem*);
    void failed(KDevelop::ProjectBaseItem*);
    void installed(KDevelop::ProjectBaseItem*);
    void cleaned(KDevelop::ProjectBaseItem*);
    void configured(KDevelop::IProject*);
    void pruned(KDevelop::IProject*);

private:
    KJob* checkConfigureJob(KDevelop::IProject* project, bool& valid);

    void addBuilder(const QString& neededfile, const QStringList& generator, KDevelop::IPlugin* i);
    KDevelop::IProjectBuilder* builderForProject(KDevelop::IProject* p) const;
    QMap<QString, KDevelop::IProjectBuilder*> m_builders;
    QMap<QString, IProjectBuilder*> m_buildersForGenerator;
};

#endif // CMAKEBUILDER_H

