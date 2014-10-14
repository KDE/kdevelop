/* This file is part of KDevelop
    Copyright 2010 Milian Wolff <mail@milianw.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "helper.h"
#include "path.h"

#include <KTemporaryFile>
#include <KIO/NetAccess>
#include <kio/job.h>
#include <kio/copyjob.h>
#include <KMessageBox>
#include <KLocalizedString>
#include <KTextEditor/Document>
#include <kparts/mainwindow.h>

#include <QApplication>
#include <QFileInfo>
#include <interfaces/iproject.h>
#include <vcs/interfaces/ibasicversioncontrol.h>
#include <interfaces/iplugin.h>
#include <vcs/vcsjob.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>

bool KDevelop::removeUrl(const KDevelop::IProject* project, const QUrl& url, const bool isFolder)
{
    QWidget* window(ICore::self()->uiController()->activeMainWindow()->window());

    if ( !KIO::NetAccess::exists(url, KIO::NetAccess::DestinationSide, window) ) {
        qWarning() << "tried to remove non-existing url:" << url << project << isFolder;
        return true;
    }

    IPlugin* vcsplugin=project->versionControlPlugin();
    if(vcsplugin) {
        IBasicVersionControl* vcs=vcsplugin->extension<IBasicVersionControl>();

        // We have a vcs and the file/folder is controller, need to make the rename through vcs
        if(vcs->isVersionControlled(url)) {
            VcsJob* job=vcs->remove(QList<QUrl>() << url);
            if(job) {
                return job->exec();
            }
        }
    }

    //if we didn't find a VCS, we remove using KIO (if the file still exists, the vcs plugin might have simply deleted the url without returning a job
    if ( !KIO::NetAccess::del( url, window ) && url.isLocalFile() && (QFileInfo(url.toLocalFile())).exists() ) {
        KMessageBox::error( window,
            isFolder ? i18n( "Cannot remove folder <i>%1</i>.", url.toDisplayString(QUrl::PreferLocalFile) )
                        : i18n( "Cannot remove file <i>%1</i>.", url.toDisplayString(QUrl::PreferLocalFile) ) );
        return false;
    }
    return true;
}

bool KDevelop::removePath(const KDevelop::IProject* project, const KDevelop::Path& path, const bool isFolder)
{
    return removeUrl(project, path.toUrl(), isFolder);
}

bool KDevelop::createFile(const QUrl& file)
{
    if (KIO::NetAccess::exists( file, KIO::NetAccess::DestinationSide, QApplication::activeWindow() )) {
        KMessageBox::error( QApplication::activeWindow(),
                            i18n( "The file <i>%1</i> already exists.", file.toDisplayString(QUrl::PreferLocalFile) ) );
        return false;
    }

    {
        KTemporaryFile temp;
        if ( !temp.open() || temp.write("\n") == -1 ) {
            KMessageBox::error( QApplication::activeWindow(),
                                i18n( "Cannot create temporary file to create <i>%1</i>.", file.toDisplayString(QUrl::PreferLocalFile) ) );
            return false;
        }
        if ( !KIO::NetAccess::upload( temp.fileName(), file, QApplication::activeWindow() ) ) {
            KMessageBox::error( QApplication::activeWindow(),
                                i18n( "Cannot create file <i>%1</i>.", file.toDisplayString(QUrl::PreferLocalFile) ) );
            return false;
        }
    }
    return true;
}

bool KDevelop::createFile(const KDevelop::Path& file)
{
    return createFile(file.toUrl());
}

bool KDevelop::createFolder(const QUrl& folder)
{
    if ( !KIO::NetAccess::mkdir( folder, QApplication::activeWindow() ) ) {
        KMessageBox::error( QApplication::activeWindow(), i18n( "Cannot create folder <i>%1</i>.", folder.toDisplayString(QUrl::PreferLocalFile) ) );
        return false;
    }
    return true;
}

bool KDevelop::createFolder(const KDevelop::Path& folder)
{
    return createFolder(folder.toUrl());
}

bool KDevelop::renameUrl(const KDevelop::IProject* project, const QUrl& oldname, const QUrl& newname)
{
    bool wasVcsMoved = false;
    IPlugin* vcsplugin = project->versionControlPlugin();
    if (vcsplugin) {
        IBasicVersionControl* vcs = vcsplugin->extension<IBasicVersionControl>();

        // We have a vcs and the file/folder is controller, need to make the rename through vcs
        if (vcs->isVersionControlled(oldname)) {
            VcsJob* job = vcs->move(oldname, newname);
            if (job && !job->exec()) {
                return false;
            }
            wasVcsMoved = true;
        }
    }
    // Fallback for the case of no vcs, or not-vcs-managed file/folder

    // try to save-as the text document, so users can directly continue to work
    // on the renamed url as well as keeping the undo-stack intact
    IDocument* document = ICore::self()->documentController()->documentForUrl(oldname);
    if (document && document->textDocument()) {
        if (!document->textDocument()->saveAs(newname)) {
            return false;
        }
        if (!wasVcsMoved) {
            // unlink the old file
            removeUrl(project, oldname, false);
        }
        return true;
    } else if (!wasVcsMoved) {
        // fallback for non-textdocuments (also folders e.g.)
        KIO::CopyJob* job = KIO::move(oldname, newname);
        return job->exec();
    } else {
        return true;
    }
}

bool KDevelop::renamePath(const KDevelop::IProject* project, const KDevelop::Path& oldName, const KDevelop::Path& newName)
{
    return renameUrl(project, oldName.toUrl(), newName.toUrl());
}

bool KDevelop::copyUrl(const KDevelop::IProject* project, const QUrl& source, const QUrl& target)
{
    IPlugin* vcsplugin=project->versionControlPlugin();
    if(vcsplugin) {
        IBasicVersionControl* vcs=vcsplugin->extension<IBasicVersionControl>();

        // We have a vcs and the file/folder is controller, need to make the rename through vcs
        if(vcs->isVersionControlled(source)) {
            VcsJob* job=vcs->copy(source, target);
            if(job) {
                return job->exec();
            }
        }
    }
    // Fallback for the case of no vcs, or not-vcs-managed file/folder
    KIO::CopyJob* job=KIO::copy(source, target);
    return KIO::NetAccess::synchronousRun(job, 0);
}

bool KDevelop::copyPath(const KDevelop::IProject* project, const KDevelop::Path& source, const KDevelop::Path& target)
{
    return copyUrl(project, source.toUrl(), target.toUrl());
}
