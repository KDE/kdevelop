/* KDevelop QMake Support
 *
 * Copyright 2006-2007 Andreas Pakulat <apaku@gmx.de>
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

#ifndef QMAKEBUILDER_H
#define QMAKEBUILDER_H

#include <interfaces/iplugin.h>
#include "iqmakebuilder.h"

#include <project/builderjob.h>

#include <QVariantList>

/**
@author Andreas Pakulat
*/
class QMakeBuilder : public KDevelop::IPlugin, public IQMakeBuilder
{
    Q_OBJECT
    Q_INTERFACES( IQMakeBuilder )
    Q_INTERFACES( KDevelop::IProjectBuilder )

public:
    explicit QMakeBuilder(QObject *parent = nullptr, const QVariantList &args = QVariantList());
    ~QMakeBuilder() override;

    KJob* build(KDevelop::ProjectBaseItem *dom) override;
    KJob* clean(KDevelop::ProjectBaseItem *dom) override;
    KJob* install(KDevelop::ProjectBaseItem *dom, const QUrl& /* prefix */) override;
    KJob* configure(KDevelop::IProject *dom) override;
    KJob* prune(KDevelop::IProject *dom) override;

    int perProjectConfigPages() const override;
    KDevelop::ConfigPage* perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent) override;

    QList<IProjectBuilder*> additionalBuilderPlugins(KDevelop::IProject* project) const override;

Q_SIGNALS:
    void built(KDevelop::ProjectBaseItem*);
    void failed(KDevelop::ProjectBaseItem*);
    void installed(KDevelop::ProjectBaseItem*);
    void cleaned(KDevelop::ProjectBaseItem*);
    void configured(KDevelop::ProjectBaseItem*);
    void pruned(KDevelop::ProjectBaseItem*);

private:
    /**
     * If @p dom needs a configure run, return a composite job consisting of configure job + @p job
     *
     * Otherwise just return @p job
     */
    KJob* maybePrependConfigureJob(KDevelop::ProjectBaseItem* project, KJob* job, KDevelop::BuilderJob::BuildType type);

    KDevelop::IPlugin* m_makeBuilder;
};

#endif // QMAKEBUILDER_H

