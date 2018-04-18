/***************************************************************************
 *   Copyright (C) 2008 by Andreas Pakulat <apaku@gmx.de                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_OPENPROJECTDIALOG_H
#define KDEVPLATFORM_OPENPROJECTDIALOG_H

#include <QUrl>

#include <KAssistantDialog>
#include <KIO/UDSEntry>
#include <KPluginMetaData>

class KPageWidgetItem;
class QFileDialog;
namespace KIO
{
class Job;
}

namespace KDevelop
{
class ProjectSourcePage;
class OpenProjectPage;
class IPlugin;

class OpenProjectDialog : public KAssistantDialog
{
    Q_OBJECT

public:
    OpenProjectDialog(bool fetch, const QUrl& startUrl,
                      const QUrl& repoUrl = QUrl(), IPlugin* vcsOrProviderPlugin = nullptr,
                      QWidget* parent = nullptr);

    /**
    * Return a QUrl pointing to the project's .kdev file.
    */
    QUrl projectFileUrl() const;
    /**
    * Return a QUrl pointing to the file, that was selected by the user.
    * Unlike projectFileUrl(), this can be a .kdev file, as well
    * as build system file (e.g. CMakeLists.txt).
    */
    QUrl selectedUrl() const;
    QString projectName() const;
    QString projectManager() const;

    int exec() override;

    QStringList projectManagerForFile(const QString& file) const;

private Q_SLOTS:
    void validateSourcePage( bool );
    void validateOpenUrl( const QUrl& );
    void validateProjectName( const QString& );
    void validateProjectManager( const QString&, const QString & );
    void storeFileList(KIO::Job*, const KIO::UDSEntryList&);
    void openPageAccepted();

private:
    bool execNativeDialog();
    void validateProjectInfo();
    QUrl m_url;
    QUrl m_selected;
    QString m_projectName;
    QString m_projectManager;
    bool m_urlIsDirectory;
    /// Used to select files when we aren't in KDE
    QFileDialog* nativeDialog = nullptr;
    KPageWidgetItem* sourcePage;
    KPageWidgetItem* openPage;
    KPageWidgetItem* projectInfoPage;
    QStringList m_fileList;
    QMap<QString, QStringList> m_projectFilters;
    QMap<QString, KPluginMetaData> m_projectPlugins;
    QStringList m_genericProjectPlugins;

    KDevelop::OpenProjectPage* openPageWidget = nullptr;
    KDevelop::ProjectSourcePage* sourcePageWidget = nullptr;
};

}

#endif
