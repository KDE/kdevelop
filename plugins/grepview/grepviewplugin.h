/*
    SPDX-FileCopyrightText: 1999-2001 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 2010 Julien Desgats <julien.desgats@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_GREPVIEWPLUGIN_H
#define KDEVPLATFORM_PLUGIN_GREPVIEWPLUGIN_H

#include <interfaces/iplugin.h>

#include <QVector>
#include <QPointer>
#include <QVariant>

class KJob;
class GrepDialog;
class GrepJob;
class GrepOutputViewFactory;

class GrepViewPlugin : public KDevelop::IPlugin
{
    Q_OBJECT
    Q_CLASSINFO( "D-Bus Interface", "org.kdevelop.GrepViewPlugin" )

public:
    explicit GrepViewPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& = QVariantList());
    ~GrepViewPlugin() override;

    void unload() override;

    void rememberSearchDirectory(QString const & directory);
    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;
    void showDialog(bool setLastUsed = false, const QString& pattern = QString(), bool show = true);

    /**
     * Returns a new instance of GrepJob. Since the plugin supports only one job at the same time,
     * previous job, if any, is killed before creating a new job.
     */
    GrepJob *newGrepJob();
    GrepJob *grepJob();
    GrepOutputViewFactory* toolViewFactory() const;
public Q_SLOTS:
    ///@param pattern the pattern to search
    ///@param directory the directory, or a semicolon-separated list of files
    ///@param show whether the search dialog should be shown. if false,
    ///            the parameters of the last search will be used.
    Q_SCRIPTABLE void startSearch(const QString& pattern, const QString& directory, bool show);

private Q_SLOTS:
    void showDialogFromMenu();
    void showDialogFromProject();

private:
    QPointer<GrepJob> m_currentJob;
    QVector<QPointer<GrepDialog>> m_currentDialogs;
    QString m_directory;
    QString m_contextMenuDirectory;
    GrepOutputViewFactory* m_factory;
};

#endif
