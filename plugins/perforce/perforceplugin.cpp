/***************************************************************************
 *   Copyright 2010  Morten Danielsen Volden <mvolden2@gmail.com>          *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "perforceplugin.h"
#include "debug.h"

#include <KPluginFactory>
#include <KPluginLoader>
#include <KLocalizedString>
#include <KAboutData>
#include <KActionCollection>
#include <QFileInfo>
#include <QDateTime>
#include <QDir>
#include <QProcessEnvironment>
#include <QMenu>
#include <QDebug>

#include <kmessagebox.h>
#include <vcs/vcsjob.h>
#include <vcs/vcsrevision.h>
#include <vcs/vcsevent.h>
#include <vcs/dvcs/dvcsjob.h>
#include <vcs/vcsannotation.h>
#include <vcs/widgets/standardvcslocationwidget.h>

#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>

#include <vcs/vcspluginhelper.h>

using namespace KDevelop;

namespace
{
QString toRevisionName(const KDevelop::VcsRevision& rev, QString currentRevision=QString())
{
    bool ok;
    int previous = currentRevision.toInt(&ok);
    previous--;
    QString tmp;
    switch(rev.revisionType()) {
        case VcsRevision::Special:
            switch(rev.revisionValue().value<VcsRevision::RevisionSpecialType>()) {
                case VcsRevision::Head:
                    return QStringLiteral("#head");
                case VcsRevision::Base:
                    return QStringLiteral("#have");
                case VcsRevision::Working:
                    return QStringLiteral("#have");
                case VcsRevision::Previous:
                    Q_ASSERT(!currentRevision.isEmpty());
                    tmp.setNum(previous);
                    tmp.prepend("#");
                    return tmp;
                case VcsRevision::Start:
                    return QString();
                case VcsRevision::UserSpecialType: //Not used
                    Q_ASSERT(false && "i don't know how to do that");
            }
            break;
        case VcsRevision::GlobalNumber:
            tmp.append("#");
            tmp.append(rev.revisionValue().toString());
            return tmp;
        case VcsRevision::Date:
        case VcsRevision::FileNumber:
        case VcsRevision::Invalid:
        case VcsRevision::UserSpecialType:
            Q_ASSERT(false);
    }
    return QString();
}


VcsItemEvent::Actions actionsFromString(QString const& changeDescription)
{
    if(changeDescription == "add")
        return VcsItemEvent::Added;
    if(changeDescription == "delete")
        return VcsItemEvent::Deleted;
    return VcsItemEvent::Modified;
}

QDir urlDir(const QUrl& url)
{
    QFileInfo f(url.toLocalFile());
    if(f.isDir())
        return QDir(url.toLocalFile());
    else
        return f.absoluteDir();
}

}

Q_LOGGING_CATEGORY(PLUGIN_PERFORCE, "kdevplatform.plugins.perforce")

PerforcePlugin::PerforcePlugin(QObject* parent, const QVariantList&):
    KDevelop::IPlugin("kdevperforce", parent)
    , m_common(new KDevelop::VcsPluginHelper(this, this))
    , m_perforcemenu(nullptr)
    , m_perforceConfigName("p4config.txt")
    , m_perforceExecutable("p4")
    , m_edit_action(nullptr)
{
    QProcessEnvironment currentEviron(QProcessEnvironment::systemEnvironment());
    QString tmp(currentEviron.value("P4CONFIG"));
    if (tmp.isEmpty()) {
        // We require the P4CONFIG variable to be set because the perforce command line client will need it
        setErrorDescription(i18n("The variable P4CONFIG is not set. Is perforce installed on the system?"));
        return;
    } else {
        m_perforceConfigName = tmp;
    }
    qCDebug(PLUGIN_PERFORCE) << "The value of P4CONFIG is : " << tmp;
}

PerforcePlugin::~PerforcePlugin()
{
}

QString PerforcePlugin::name() const
{
    return i18n("Perforce");
}

KDevelop::VcsImportMetadataWidget* PerforcePlugin::createImportMetadataWidget(QWidget* /*parent*/)
{
    return nullptr;
}

bool PerforcePlugin::isValidRemoteRepositoryUrl(const QUrl& remoteLocation)
{
    Q_UNUSED(remoteLocation);
    // TODO
    return false;
}

bool PerforcePlugin::isValidDirectory(const QUrl & dirPath)
{
    const QFileInfo finfo(dirPath.toLocalFile());
    QDir dir = finfo.isDir() ? QDir(dirPath.toLocalFile()) : finfo.absoluteDir();

    do {
        if (dir.exists(m_perforceConfigName)) {
            return true;
        }
    } while (dir.cdUp());
    return false;
}

bool PerforcePlugin::isVersionControlled(const QUrl& localLocation)
{
    QFileInfo fsObject(localLocation.toLocalFile());
    if (fsObject.isDir()) {
        return isValidDirectory(localLocation);
    }
    return parseP4fstat(fsObject, KDevelop::OutputJob::Silent);
}

DVcsJob* PerforcePlugin::p4fstatJob(const QFileInfo& curFile, OutputJob::OutputJobVerbosity verbosity)
{
    DVcsJob* job = new DVcsJob(curFile.absolutePath(), this, verbosity);
    setEnvironmentForJob(job, curFile);
    *job << m_perforceExecutable << "fstat" << curFile.fileName();
    return job;
}

bool PerforcePlugin::parseP4fstat(const QFileInfo& curFile, OutputJob::OutputJobVerbosity verbosity)
{
    QScopedPointer<DVcsJob> job(p4fstatJob(curFile, verbosity));
    if (job->exec() && job->status() == KDevelop::VcsJob::JobSucceeded) {
        qCDebug(PLUGIN_PERFORCE) << "Perforce returned: " << job->output();
        if (!job->output().isEmpty())
            return true;
    }
    return false;
}

QString PerforcePlugin::getRepositoryName(const QFileInfo& curFile)
{
    static const QString DEPOT_FILE_STR("... depotFile ");
    QString ret;
    QScopedPointer<DVcsJob> job(p4fstatJob(curFile, KDevelop::OutputJob::Silent));
    if (job->exec() && job->status() == KDevelop::VcsJob::JobSucceeded) {
        if (!job->output().isEmpty()) {
            QStringList outputLines = job->output().split('\n', QString::SkipEmptyParts);
            foreach(const QString & line, outputLines) {
                int idx(line.indexOf(DEPOT_FILE_STR));
                if (idx != -1) {
                    ret = line.right(line.size() - DEPOT_FILE_STR.size());
                    return ret;
                }
            }
        }
    }

    return ret;
}

KDevelop::VcsJob* PerforcePlugin::repositoryLocation(const QUrl& /*localLocation*/)
{
    return nullptr;
}

KDevelop::VcsJob* PerforcePlugin::add(const QList<QUrl>& localLocations, KDevelop::IBasicVersionControl::RecursionMode /*recursion*/)
{
    QFileInfo curFile(localLocations.front().toLocalFile());
    DVcsJob* job = new DVcsJob(curFile.dir(), this, KDevelop::OutputJob::Verbose);
    setEnvironmentForJob(job, curFile);
    *job << m_perforceExecutable << "add" << localLocations;

    return job;
}

KDevelop::VcsJob* PerforcePlugin::remove(const QList<QUrl>& /*localLocations*/)
{
    return nullptr;
}

KDevelop::VcsJob* PerforcePlugin::copy(const QUrl& /*localLocationSrc*/, const QUrl& /*localLocationDstn*/)
{
    return nullptr;
}

KDevelop::VcsJob* PerforcePlugin::move(const QUrl& /*localLocationSrc*/, const QUrl& /*localLocationDst*/)
{
    return nullptr;
}

KDevelop::VcsJob* PerforcePlugin::status(const QList<QUrl>& localLocations, KDevelop::IBasicVersionControl::RecursionMode /*recursion*/)
{
    if (localLocations.count() != 1) {
        KMessageBox::error(nullptr, i18n("Please select only one item for this operation"));
        return nullptr;
    }

    QFileInfo curFile(localLocations.front().toLocalFile());

    DVcsJob* job = new DVcsJob(curFile.dir(), this, KDevelop::OutputJob::Verbose);
    setEnvironmentForJob(job, curFile);
    *job << m_perforceExecutable << "fstat" << curFile.fileName();
    connect(job, &DVcsJob::readyForParsing, this, &PerforcePlugin::parseP4StatusOutput);

    return job;
}

KDevelop::VcsJob* PerforcePlugin::revert(const QList<QUrl>& localLocations, KDevelop::IBasicVersionControl::RecursionMode /*recursion*/)
{
    if (localLocations.count() != 1) {
        KMessageBox::error(nullptr, i18n("Please select only one item for this operation"));
        return nullptr;
    }

    QFileInfo curFile(localLocations.front().toLocalFile());

    DVcsJob* job = new DVcsJob(curFile.dir(), this, KDevelop::OutputJob::Verbose);
    setEnvironmentForJob(job, curFile);
    *job << m_perforceExecutable << "revert" << curFile.fileName();

    return job;

}

KDevelop::VcsJob* PerforcePlugin::update(const QList<QUrl>& localLocations, const KDevelop::VcsRevision& /*rev*/, KDevelop::IBasicVersionControl::RecursionMode /*recursion*/)
{
    QFileInfo curFile(localLocations.front().toLocalFile());

    DVcsJob* job = new DVcsJob(curFile.dir(), this, KDevelop::OutputJob::Verbose);
    setEnvironmentForJob(job, curFile);
    //*job << m_perforceExecutable << "-p" << "127.0.0.1:1666" << "info"; - Let's keep this for now it's very handy for debugging
    QString fileOrDirectory;
    if (curFile.isDir())
        fileOrDirectory = curFile.absolutePath() + "/...";
    else
        fileOrDirectory = curFile.fileName();
    *job << m_perforceExecutable << "sync" << fileOrDirectory;
    return job;
}

KDevelop::VcsJob* PerforcePlugin::commit(const QString& message, const QList<QUrl>& localLocations, KDevelop::IBasicVersionControl::RecursionMode /*recursion*/)
{
    if (localLocations.empty() || message.isEmpty())
        return errorsFound(i18n("No files or message specified"));


    QFileInfo curFile(localLocations.front().toLocalFile());

    DVcsJob* job = new DVcsJob(curFile.dir(), this, KDevelop::OutputJob::Verbose);
    setEnvironmentForJob(job, curFile);
    *job << m_perforceExecutable << "submit" << "-d" << message << localLocations;

    return job;
}

KDevelop::VcsJob* PerforcePlugin::diff(const QUrl& fileOrDirectory, const KDevelop::VcsRevision& srcRevision, const KDevelop::VcsRevision& dstRevision, KDevelop::VcsDiff::Type , KDevelop::IBasicVersionControl::RecursionMode /*recursion*/)
{
    QFileInfo curFile(fileOrDirectory.toLocalFile());
    QString depotSrcFileName = getRepositoryName(curFile);
    QString depotDstFileName = depotSrcFileName;
    depotSrcFileName.append(toRevisionName(srcRevision, dstRevision.prettyValue())); // dstRevision acutally contains the number that we want to take previous of

    DVcsJob* job = new DVcsJob(curFile.dir(), this, KDevelop::OutputJob::Verbose);
    setEnvironmentForJob(job, curFile);
    switch (dstRevision.revisionType()) {
    case VcsRevision::FileNumber:
    case VcsRevision::GlobalNumber:
        depotDstFileName.append("#");
        depotDstFileName.append(dstRevision.prettyValue());
        *job << m_perforceExecutable << "diff2" << "-u" << depotSrcFileName << depotDstFileName;
        break;
    case VcsRevision::Special:
        switch (dstRevision.revisionValue().value<VcsRevision::RevisionSpecialType>()) {
        case VcsRevision::Working:
            *job << m_perforceExecutable << "diff" << "-du" << depotSrcFileName;
            break;
        case VcsRevision::Start:
        case VcsRevision::UserSpecialType:
        default:
            break;
        }
    default:
        break;
    }

    connect(job, &DVcsJob::readyForParsing, this, &PerforcePlugin::parseP4DiffOutput);
    return job;
}

KDevelop::VcsJob* PerforcePlugin::log(const QUrl& localLocation, const KDevelop::VcsRevision& rev, long unsigned int limit)
{
    static QString lastSeenChangeList;
    QFileInfo curFile(localLocation.toLocalFile());
    QString localLocationAndRevStr = localLocation.toLocalFile(); 
    
    DVcsJob* job = new DVcsJob(urlDir(localLocation), this, KDevelop::OutputJob::Verbose);
    setEnvironmentForJob(job, curFile);
    *job << m_perforceExecutable << "filelog" << "-lit";
    if(limit > 0)
        *job << QStringLiteral("-m %1").arg(limit);
    
    if (curFile.isDir()) {
        localLocationAndRevStr.append("/...");
    } 
    QString revStr = toRevisionName(rev, QString());
    if(!revStr.isEmpty()) {
        // This is not too nice, but perforce argument for restricting output from filelog does not Work :-(
        // So putting this in so we do not end up in infinite loop calling log,
        if(revStr == lastSeenChangeList) {
            localLocationAndRevStr.append("#none");
            lastSeenChangeList.clear();
        } else { 
            localLocationAndRevStr.append(revStr);
            lastSeenChangeList = revStr;
        }
    }
    *job << localLocationAndRevStr;
    qCDebug(PLUGIN_PERFORCE) << "Issuing the following command to p4: " << job->dvcsCommand();
    connect(job, &DVcsJob::readyForParsing, this, &PerforcePlugin::parseP4LogOutput);
    return job;
}

KDevelop::VcsJob* PerforcePlugin::log(const QUrl& localLocation, const KDevelop::VcsRevision& /*rev*/, const KDevelop::VcsRevision& /*limit*/)
{
    QFileInfo curFile(localLocation.toLocalFile());
    if (curFile.isDir()) {
        KMessageBox::error(nullptr, i18n("Please select a file for this operation"));
        return errorsFound(i18n("Directory not supported for this operation"));
    }

    DVcsJob* job = new DVcsJob(curFile.dir(), this, KDevelop::OutputJob::Verbose);
    setEnvironmentForJob(job, curFile);
    *job << m_perforceExecutable << "filelog" << "-lit" << localLocation;

    connect(job, &DVcsJob::readyForParsing, this , &PerforcePlugin::parseP4LogOutput);
    return job;
}

KDevelop::VcsJob* PerforcePlugin::annotate(const QUrl& localLocation, const KDevelop::VcsRevision& /*rev*/)
{
    QFileInfo curFile(localLocation.toLocalFile());
    if (curFile.isDir()) {
        KMessageBox::error(nullptr, i18n("Please select a file for this operation"));
        return errorsFound(i18n("Directory not supported for this operation"));
    }

    DVcsJob* job = new DVcsJob(curFile.dir(), this, KDevelop::OutputJob::Verbose);
    setEnvironmentForJob(job, curFile);
    *job << m_perforceExecutable << "annotate" << "-qi" << localLocation;

    connect(job, &DVcsJob::readyForParsing, this , &PerforcePlugin::parseP4AnnotateOutput);
    return job;
}

KDevelop::VcsJob* PerforcePlugin::resolve(const QList<QUrl>& /*localLocations*/, KDevelop::IBasicVersionControl::RecursionMode /*recursion*/)
{
    return nullptr;
}

KDevelop::VcsJob* PerforcePlugin::createWorkingCopy(const KDevelop::VcsLocation& /*sourceRepository*/, const QUrl& /*destinationDirectory*/, KDevelop::IBasicVersionControl::RecursionMode /*recursion*/)
{
    return nullptr;
}

KDevelop::VcsLocationWidget* PerforcePlugin::vcsLocation(QWidget* parent) const
{
    return new StandardVcsLocationWidget(parent);
}


KDevelop::VcsJob* PerforcePlugin::edit(const QList<QUrl>& localLocations)
{
    QFileInfo curFile(localLocations.front().toLocalFile());

    DVcsJob* job = new DVcsJob(curFile.dir(), this, KDevelop::OutputJob::Verbose);
    setEnvironmentForJob(job, curFile);
    *job << m_perforceExecutable << "edit" << localLocations;

    return job;
}

KDevelop::VcsJob* PerforcePlugin::edit(const QUrl& /*localLocation*/)
{
    return nullptr;
}

KDevelop::VcsJob* PerforcePlugin::unedit(const QUrl& /*localLocation*/)
{
    return nullptr;
}

KDevelop::VcsJob* PerforcePlugin::localRevision(const QUrl& /*localLocation*/, KDevelop::VcsRevision::RevisionType)
{
    return nullptr;
}

KDevelop::VcsJob* PerforcePlugin::import(const QString& /*commitMessage*/, const QUrl& /*sourceDirectory*/, const KDevelop::VcsLocation& /*destinationRepository*/)
{
    return nullptr;
}

KDevelop::ContextMenuExtension PerforcePlugin::contextMenuExtension(KDevelop::Context* context)
{
    m_common->setupFromContext(context);

    const QList<QUrl> & ctxUrlList  = m_common->contextUrlList();

    bool hasVersionControlledEntries = false;
    for( const QUrl& url : ctxUrlList) {
        if (isValidDirectory(url)) {
            hasVersionControlledEntries = true;
            break;
        }
    }

    if (!hasVersionControlledEntries)
        return IPlugin::contextMenuExtension(context);

    QMenu * perforceMenu = m_common->commonActions();
    perforceMenu->addSeparator();

    perforceMenu->addSeparator();
    if (!m_edit_action) {
         m_edit_action = new QAction(i18n("Edit"), this);
         connect(m_edit_action, &QAction::triggered, this, & PerforcePlugin::ctxEdit);
     }
     perforceMenu->addAction(m_edit_action);

    ContextMenuExtension menuExt;
    menuExt.addAction(ContextMenuExtension::VcsGroup, perforceMenu->menuAction());

    return menuExt;
}

void PerforcePlugin::ctxEdit()
{
    QList<QUrl> const & ctxUrlList = m_common->contextUrlList();
    KDevelop::ICore::self()->runController()->registerJob(edit(ctxUrlList));
}

void PerforcePlugin::setEnvironmentForJob(DVcsJob* job, const QFileInfo& curFile)
{    
    KProcess* jobproc = job->process();
    jobproc->setEnv("P4CONFIG", m_perforceConfigName);
    if (curFile.isDir()) {
        jobproc->setEnv("PWD", curFile.filePath());
    } else {
        jobproc->setEnv("PWD", curFile.absolutePath());        
    }
}

QList<QVariant> PerforcePlugin::getQvariantFromLogOutput(QStringList const& outputLines)
{
    static const QString LOGENTRY_START("... #");
    static const QString DEPOTMESSAGE_START("... .");
    QMap<int, VcsEvent> changes;
    QList<QVariant> commits;
    QString currentFileName;
    QString changeNumberStr, author,changeDescription, commitMessage;
    VcsEvent currentVcsEvent;
    VcsItemEvent currentRepoFile;
    VcsRevision rev;
    int indexofAt;
    int changeNumber = 0;
    
    foreach(const QString & line, outputLines) {
        if (!line.startsWith(LOGENTRY_START) && !line.startsWith(DEPOTMESSAGE_START)  && !line.startsWith('\t')) {
            currentFileName = line;
        }
        if(line.indexOf(LOGENTRY_START) != -1)
        {
            // expecting the Logentry line to be of the form:
            //... #5 change 10 edit on 2010/12/06 12:07:31 by mvo@testbed (text)
            changeNumberStr = line.section(' ', 3, 3 ); // We use global change number
            changeNumber = changeNumberStr.toInt();
            author = line.section(' ', 9, 9);
            changeDescription = line.section(' ' , 4, 4 );
            indexofAt = author.indexOf('@');
            author.remove(indexofAt, author.size()); // Only keep the username itself
            rev.setRevisionValue(changeNumberStr, KDevelop::VcsRevision::GlobalNumber);
            
            changes[changeNumber].setRevision(rev);
            changes[changeNumber].setAuthor(author);
            changes[changeNumber].setDate(QDateTime::fromString(line.section(' ', 6, 7), "yyyy/MM/dd hh:mm:ss"));
            currentRepoFile.setRepositoryLocation(currentFileName);
            currentRepoFile.setActions( actionsFromString(changeDescription) );
            changes[changeNumber].addItem(currentRepoFile);
            commitMessage.clear(); // We have a new entry, clear message
        }
        if (line.startsWith('\t') || line.startsWith(DEPOTMESSAGE_START)) {
            commitMessage += line.trimmed() + '\n';
            changes[changeNumber].setMessage(commitMessage);
        }       
        
    }
    
    for(auto item : changes) {
        commits.prepend(QVariant::fromValue(item));
    }
    return commits;
}


void PerforcePlugin::parseP4StatusOutput(DVcsJob* job)
{
    QStringList outputLines = job->output().split('\n', QString::SkipEmptyParts);
    QVariantList statuses;
    QList<QUrl> processedFiles;
    static const QString ACTION_STR("... action ");
    static const QString CLIENT_FILE_STR("... clientFile ");



    VcsStatusInfo status;
    status.setState(VcsStatusInfo::ItemUserState);
    foreach(const QString & line, outputLines) {
        int idx(line.indexOf(ACTION_STR));
        if (idx != -1) {
            QString curr = line.right(line.size() - ACTION_STR.size());

            if (curr == "edit") {
                status.setState(VcsStatusInfo::ItemModified);
            } else if (curr == "add") {
                status.setState(VcsStatusInfo::ItemAdded);
            } else {
                status.setState(VcsStatusInfo::ItemUserState);
            }
            continue;
        }
        idx = line.indexOf(CLIENT_FILE_STR);
        if (idx != -1) {
            QUrl fileUrl = QUrl::fromLocalFile(line.right(line.size() - CLIENT_FILE_STR.size()));

            status.setUrl(fileUrl);
        }
    }
    statuses.append(qVariantFromValue<VcsStatusInfo>(status));
    job->setResults(statuses);
}

void PerforcePlugin::parseP4LogOutput(KDevelop::DVcsJob* job)
{
    QList<QVariant> commits(getQvariantFromLogOutput(job->output().split('\n', QString::SkipEmptyParts)));

    job->setResults(commits);
}

void PerforcePlugin::parseP4DiffOutput(DVcsJob* job)
{
    VcsDiff diff;
    diff.setDiff(job->output());
    QDir dir(job->directory());

    do {
        if (dir.exists(m_perforceConfigName)) {
            break;
        }
    } while (dir.cdUp());

    diff.setBaseDiff(QUrl::fromLocalFile(dir.absolutePath()));

    job->setResults(qVariantFromValue(diff));
}

void PerforcePlugin::parseP4AnnotateOutput(DVcsJob *job)
{
    QVariantList results;
    /// First get the changelists for this file
    QStringList strList(job->dvcsCommand());
    QString localLocation(strList.last()); /// ASSUMPTION WARNING - localLocation is the last in the annotate command
    KDevelop::VcsRevision dummyRev;
    QScopedPointer<DVcsJob> logJob(new DVcsJob(job->directory(), this, OutputJob::Silent));
    QFileInfo curFile(localLocation);
    setEnvironmentForJob(logJob.data(), curFile);
    *logJob << m_perforceExecutable << "filelog" << "-lit" << localLocation;

    QList<QVariant> commits;
    if (logJob->exec() && logJob->status() == KDevelop::VcsJob::JobSucceeded) {
        if (!job->output().isEmpty()) {
            commits = getQvariantFromLogOutput(logJob->output().split('\n', QString::SkipEmptyParts));
        }
    }
    
    VcsEvent item;
    QMap<qlonglong, VcsEvent> globalCommits;
    /// Move the VcsEvents to a more suitable data strucure
    for (QList<QVariant>::const_iterator commitsIt = commits.constBegin(), commitsEnd = commits.constEnd(); 
           commitsIt != commitsEnd; ++commitsIt) {
        if(commitsIt->canConvert<VcsEvent>())
        {
            item = commitsIt->value<VcsEvent>();
        }
        globalCommits.insert(item.revision().revisionValue().toLongLong(), item);
    }

    VcsAnnotationLine* annotation;
    QStringList lines = job->output().split('\n');

    size_t lineNumber(0);
    QMap<QString, VcsAnnotationLine> definedRevisions;
    QMap<qlonglong, VcsEvent>::iterator currentEvent;
    bool convertToIntOk(false);
    int globalRevisionInt(0);
    QString globalRevision;
    for (QStringList::const_iterator it = lines.constBegin(), itEnd = lines.constEnd();
            it != itEnd; ++it) {
        if (it->isEmpty()) {
            continue;
        }

        globalRevision = it->left(it->indexOf(':'));

        annotation = new VcsAnnotationLine;
        annotation->setLineNumber(lineNumber);
        VcsRevision rev;
        rev.setRevisionValue(globalRevision, KDevelop::VcsRevision::GlobalNumber);
        annotation->setRevision(rev);
        // Find the other info in the commits list
        globalRevisionInt = globalRevision.toLongLong(&convertToIntOk);
        if(convertToIntOk)
        {
            currentEvent = globalCommits.find(globalRevisionInt);
            annotation->setAuthor(currentEvent->author());
            annotation->setCommitMessage(currentEvent->message());
            annotation->setDate(currentEvent->date());
        }

        results += qVariantFromValue(*annotation);
        ++lineNumber;
    }
    
    job->setResults(results);
}


KDevelop::VcsJob* PerforcePlugin::errorsFound(const QString& error, KDevelop::OutputJob::OutputJobVerbosity verbosity)
{
    DVcsJob* j = new DVcsJob(QDir::temp(), this, verbosity);
    *j << "echo" << i18n("error: %1", error) << "-n";
    return j;
}


