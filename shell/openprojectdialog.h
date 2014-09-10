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

#include <kassistantdialog.h>
#include <kio/udsentry.h>

class KPageWidgetItem;
namespace KIO
{
class Job;
}

namespace KDevelop
{
class ProjectSourcePage;
class OpenProjectPage;

class OpenProjectDialog : public KAssistantDialog
{
    Q_OBJECT

public:
    OpenProjectDialog( bool fetch, const QUrl& startUrl, QWidget* parent = 0 );
    QUrl projectFileUrl();
    QString projectName();
    QString projectManager();

private slots:
    void validateSourcePage( bool );
    void validateOpenUrl( const QUrl& );
    void validateProjectName( const QString& );
    void validateProjectManager( const QString& );
    void storeFileList(KIO::Job*, const KIO::UDSEntryList&);
    void openPageAccepted();

private:
    void validateProjectInfo();
    QUrl m_url;
    QString m_projectName;
    QString m_projectManager;
    KPageWidgetItem* sourcePage;
    KPageWidgetItem* openPage;
    KPageWidgetItem* projectInfoPage;
    QStringList m_fileList;

    KDevelop::OpenProjectPage* openPageWidget;
    KDevelop::ProjectSourcePage* sourcePageWidget;
};

}

#endif
