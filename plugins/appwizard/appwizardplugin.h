/*
    SPDX-FileCopyrightText: 2001 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 2004-2005 Sascha Cunz <sascha@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _APPWIZARDPLUGIN_H_
#define _APPWIZARDPLUGIN_H_

#include <interfaces/iplugin.h>
#include <interfaces/itemplateprovider.h>

#include <QHash>
#include <QVariant>

class ProjectTemplatesModel;
class ApplicationInfo;
class KArchiveDirectory;

class AppWizardPlugin: public KDevelop::IPlugin, public KDevelop::ITemplateProvider
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::ITemplateProvider)

public:
    explicit AppWizardPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& = QVariantList());
    ~AppWizardPlugin() override;
    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;

    QAbstractItemModel* templatesModel() const override;
    QString knsConfigurationFile() const override;
    QStringList supportedMimeTypes() const override;
    QString name() const override;
    QIcon icon() const override;
    void loadTemplate(const QString& fileName) override;
    void reload() override;

private Q_SLOTS:
    void slotNewProject();

private:
    ProjectTemplatesModel* model() const;

    QString createProject(const ApplicationInfo& );
    bool unpackArchive(const KArchiveDirectory* dir, const QString& dest, const QStringList& skipList = {});
    bool copyFileAndExpandMacros(const QString &source, const QString &dest);

    // created lazily
    mutable ProjectTemplatesModel* m_templatesModel = nullptr;
    QAction* m_newFromTemplate;
    QHash<QString, QString> m_variables;
};

#endif

