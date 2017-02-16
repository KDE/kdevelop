/* This file is part of KDevelop
    Copyright 2012 Aleix Pol Gonzalez <aleixpol@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#ifndef KDEVNINJABUILDERPLUGIN_H
#define KDEVNINJABUILDERPLUGIN_H

#include "ninjajob.h"

#include <interfaces/iplugin.h>
#include <project/interfaces/iprojectbuilder.h>
#include <util/objectlist.h>
#include <QUrl>
#include <QVariantList>

class KDevNinjaBuilderPlugin
    : public KDevelop::IPlugin
    , public KDevelop::IProjectBuilder
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IProjectBuilder)

public:
    KDevNinjaBuilderPlugin(QObject* parent = nullptr, const QVariantList& args = QVariantList());

    KJob* build(KDevelop::ProjectBaseItem* item) override;
    KJob* clean(KDevelop::ProjectBaseItem* item) override;
    KJob* install(KDevelop::ProjectBaseItem* dom, const QUrl& installPath) override;
    KJob* install(KDevelop::ProjectBaseItem* item);

    NinjaJob* runNinja(KDevelop::ProjectBaseItem* item, NinjaJob::CommandType commandType,
                       const QStringList& args, const QByteArray& signal);

    int perProjectConfigPages() const override;
    KDevelop::ConfigPage* perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent) override;

Q_SIGNALS:
    void built(KDevelop::ProjectBaseItem* item);
    void failed(KDevelop::ProjectBaseItem* item);
    void installed(KDevelop::ProjectBaseItem* item);
    void cleaned(KDevelop::ProjectBaseItem* item);

private:
    KDevelop::ObjectList<NinjaJob> m_activeNinjaJobs;
};

#endif // KDEVNINJABUILDERPLUGIN_H
