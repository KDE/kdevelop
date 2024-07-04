/*
    SPDX-FileCopyrightText: 2006-2007 Aleix Pol <aleixpol@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CMAKEBUILDER_H
#define CMAKEBUILDER_H

#include <interfaces/iplugin.h>
#include <project/interfaces/iprojectbuilder.h>

#include <QList>
#include <QStringList>
#include <QVariantList>

class KDialog;

namespace KDevelop{
    class ProjectBaseItem;
}

class QUrl;

/**
 * @author Aleix Pol
*/
class CMakeBuilder : public KDevelop::IPlugin, public KDevelop::IProjectBuilder
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IProjectBuilder )

public:
    explicit CMakeBuilder(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args = QVariantList());
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

