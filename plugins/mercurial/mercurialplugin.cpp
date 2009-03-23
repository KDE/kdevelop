/***************************************************************************
 *   This file was taken from KDevelop's git plugin                        *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   Adapted for Mercurial                                                 *
 *   Copyright 2009 Fabian Wiesel <fabian.wiesel@fu-berlin.de>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "mercurialplugin.h"

#include <algorithm>
#include <memory>

#include <QtCore/QDir>
#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>

#include <KDE/KPluginFactory>
#include <KDE/KPluginLoader>
#include <KDE/KLocalizedString>
#include <KDE/KAboutData>
#include <KDE/KDateTime>
#include <KDE/KDebug>

#include <interfaces/icore.h>

#include <vcs/vcsjob.h>
#include <vcs/vcsevent.h>
#include <vcs/vcsrevision.h>
#include <vcs/vcsannotation.h>
#include <vcs/dvcs/dvcsjob.h>
#include <shell/core.h>


K_PLUGIN_FACTORY(KDevMercurialFactory, registerPlugin<MercurialPlugin>();)
K_EXPORT_PLUGIN(KDevMercurialFactory(KAboutData("kdevmercurial", "kdevmercurial", ki18n("Mercurial"), "0.1", ki18n("A plugin to support Mercurial version control systems"), KAboutData::License_GPL)))

using namespace KDevelop;

MercurialPlugin::MercurialPlugin(QObject *parent, const QVariantList &)
        : DistributedVersionControlPlugin(parent, KDevMercurialFactory::componentData())
{
    KDEV_USE_EXTENSION_INTERFACE(KDevelop::IBasicVersionControl)
    KDEV_USE_EXTENSION_INTERFACE(KDevelop::IDistributedVersionControl)

    core()->uiController()->addToolView(i18n("Mercurial"), dvcsViewFactory());
    if (!(KDevelop::Core::self()->setupFlags() & KDevelop::Core::NoUi))
        setXMLFile("kdevmercurial.rc");
}

MercurialPlugin::~MercurialPlugin()
{
}

QString MercurialPlugin::name() const
{
    return QLatin1String("Mercurial");
}

bool MercurialPlugin::isValidDirectory(const KUrl & directory)
{
    // Mercurial uses the same test, so we don't lose any functionality
    static const QString hgDir(".hg");
    const QString initialPath(directory.toLocalFile());
    const QFileInfo finfo(initialPath);
    QDir dir;
    if (finfo.isFile()) {
        dir = finfo.absoluteDir();
    } else {
        dir = QDir(initialPath);
        dir.makeAbsolute();
    }

    while (!dir.cd(hgDir) && dir.cdUp()) {} // cdUp, until there is a sub-directory called .hg

    return hgDir == dir.dirName();
}

bool MercurialPlugin::isVersionControlled(const KUrl & url)
{
    const QFileInfo fsObject(url.path());

    if (!fsObject.isFile()) {
        return isValidDirectory(url);
    }

    // Clean, Added, Modified. Escape possible files starting with "-"
    static const QStringList versionControlledFlags(QString("-c -a -m --").split(' '));
    const QString absolutePath = fsObject.absolutePath();
    QStringList listFile(versionControlledFlags);
    listFile.push_back(fsObject.fileName());
    const QStringList filesInDir = getLsFiles(absolutePath, listFile);

    return !filesInDir.empty();
}

VcsJob* MercurialPlugin::init(const KUrl &directory)
{
    std::auto_ptr<DVcsJob> job(new DVcsJob(this));

    if (!prepareJob(job.get(), directory.toLocalFile(), MercurialPlugin::Init)) {
        return NULL;
    }

    *job << "hg" << "init";

    return job.release();
}

VcsJob* MercurialPlugin::clone(const VcsLocation & localOrRepoLocationSrc, const KUrl& directory)
{
    std::auto_ptr<DVcsJob> job(new DVcsJob(this));

    if (!prepareJob(job.get(), directory.toLocalFile(), MercurialPlugin::Init)) {
        return NULL;
    }

    *job << "hg" << "clone" << "--" <<  localOrRepoLocationSrc.localUrl().pathOrUrl();

    return job.release();
}

VcsJob* MercurialPlugin::pull(const VcsLocation & otherRepository, const KUrl& workingRepository)
{
    std::auto_ptr<DVcsJob> job(new DVcsJob(this));

    if (!prepareJob(job.get(), workingRepository.toLocalFile(), MercurialPlugin::Init)) {
        return NULL;
    }

    *job << "hg" << "fetch" << "--";

    QString pathOrUrl = otherRepository.localUrl().pathOrUrl();

    if (!pathOrUrl.isEmpty())
        *job << pathOrUrl;

    return job.release();
}

VcsJob* MercurialPlugin::push(const KUrl &workingRepository, const VcsLocation & otherRepository)
{
    std::auto_ptr<DVcsJob> job(new DVcsJob(this));

    if (!prepareJob(job.get(), workingRepository.toLocalFile(), MercurialPlugin::Init)) {
        return NULL;
    }

    *job << "hg" << "push" << "--";

    QString pathOrUrl = otherRepository.localUrl().pathOrUrl();

    if (!pathOrUrl.isEmpty())
        *job << pathOrUrl;

    return job.release();
}

VcsJob* MercurialPlugin::add(const KUrl::List& localLocations, IBasicVersionControl::RecursionMode recursion)
{
    if (localLocations.empty())
        return NULL;

    std::auto_ptr<DVcsJob> job(new DVcsJob(this));

    if (!prepareJob(job.get(), localLocations.front().path())) {
        return NULL;
    }

    *job << "hg" << "add" << "--";

    if (!addDirsConditionally(job.get(), localLocations, recursion)) {
        return NULL;
    }

    return job.release();
}

VcsJob* MercurialPlugin::copy(const KUrl& localLocationSrc, const KUrl& localLocationDst)
{
    std::auto_ptr<DVcsJob> job(new DVcsJob(this));

    if (!prepareJob(job.get(), localLocationSrc.path())) {
        return NULL;
    }

    *job << "hg" << "cp" << "--" << localLocationSrc.path() << localLocationDst.path();

    return job.release();
}

VcsJob* MercurialPlugin::move(const KUrl& localLocationSrc,
                const KUrl& localLocationDst)
{
    std::auto_ptr<DVcsJob> job(new DVcsJob(this));

    if (!prepareJob(job.get(), localLocationSrc.path())) {
        return NULL;
    }

    *job << "hg" << "mv" << "--" << localLocationSrc.path() << localLocationDst.path();

    return job.release();
}

//If no files specified then commit already added files
VcsJob* MercurialPlugin::commit(const QString& message,
                                  const KUrl::List& localLocations,
                                  IBasicVersionControl::RecursionMode recursion)
{
    if (localLocations.empty() || message.isEmpty())
        return NULL;

    std::auto_ptr<DVcsJob> job(new DVcsJob(this));

    if (!prepareJob(job.get(), localLocations.front().path())) {
        return NULL;
    }

    //Note: the message is quoted somewhere else, so if we quote here then we have quotes in the commit log
    *job << "hg" << "commit" << "-m" << message << "--";

    if (!addDirsConditionally(job.get(), localLocations, recursion)) {
        return NULL;
    }

    return job.release();
}

VcsJob* MercurialPlugin::diff(const VcsLocation & localOrRepoLocationSrc,
                const VcsLocation & localOrRepoLocationDst,
                const VcsRevision & srcRevision,
                const VcsRevision & dstRevision,
                VcsDiff::Type diffType,
                IBasicVersionControl::RecursionMode recursionMode)
{
    Q_UNUSED(diffType)
    Q_UNUSED(recursionMode)
    //TODO: Honour recursionmode and diffType, handle non-local diffs and a working revision as a src element
    if (VcsLocation::LocalLocation != localOrRepoLocationSrc.type()
            || VcsLocation::LocalLocation != localOrRepoLocationDst.type()
            || localOrRepoLocationSrc.localUrl() != localOrRepoLocationDst.localUrl()) {
        return NULL;
    }

    QString srcRev = toMercurialRevision(srcRevision);
    QString dstRev = toMercurialRevision(dstRevision);

    if (QString::null == srcRev
        || QString::null == dstRev
        || srcRev.isEmpty())    // We cannot handle working-directory file as src argument
        return NULL;

    std::auto_ptr<DVcsJob> job(new DVcsJob(this));

    const QString srcPath = localOrRepoLocationSrc.localUrl().path();
//     const QString dstPath = localOrRepoLocationDst.localUrl().path();

    if (!prepareJob(job.get(), srcPath, MercurialPlugin::Init)) {
        return NULL;
    }

    *job << "hg" << "diff";

    if (diffType == VcsDiff::DiffUnified)
        *job << "-U" << "3";    // Default from GNU diff

//     *job << "-r" << srcRev;
//     if ("" != dstRev)
//         *job << "-r" << dstRev;
    *job << "--";
    *job << srcPath;

    connect(job.get(), SIGNAL(readyForParsing(DVcsJob*)), SLOT(parseDiff(DVcsJob*)));

    return job.release();
}

VcsJob* MercurialPlugin::remove(const KUrl::List& files)
{
    if (files.empty())
        return NULL;

    std::auto_ptr<DVcsJob> job(new DVcsJob(this));

    if (!prepareJob(job.get(), files.front().path())) {
        return NULL;
    }

    *job << "hg" << "rm" << "--";

    addFileList(job.get(), files);
    return job.release();
}

VcsJob* MercurialPlugin::status(const KUrl::List& localLocations, IBasicVersionControl::RecursionMode recursion)
{
    std::auto_ptr<DVcsJob> job(new DVcsJob(this));

    if (!prepareJob(job.get(), localLocations.front().path())) {
        return NULL;
    }

    *job << "hg" << "status" << "-A" << "--";
    if (!addDirsConditionally(job.get(), localLocations, recursion)) {
        return NULL;
    }

    connect(job.get(), SIGNAL(readyForParsing(DVcsJob*)), SLOT(parseStatus(DVcsJob*)));

    return job.release();
}

bool MercurialPlugin::parseStatus(DVcsJob *job) const
{
    if (job->status() != VcsJob::JobSucceeded) {
        kDebug() << job->output();
        return false;
    }

    const QString dir = job->getDirectory().absolutePath().append(QDir::separator());
    const QStringList output = job->output().split('\n', QString::SkipEmptyParts);
    QList<QVariant> filestatus;
    foreach(const QString &line, output) {
        QChar stCh = line.at(0);

        KUrl file(line.mid(2).prepend(dir));

        kDebug() << dir;

        VcsStatusInfo status;
        status.setUrl(file);
        status.setState(charToState(stCh.toAscii()));

        filestatus.append(qVariantFromValue(status));
    }

    job->setResults(qVariantFromValue(filestatus));
    return true;
}

VcsJob* MercurialPlugin::revert(const KUrl::List& localLocations,
                                   IBasicVersionControl::RecursionMode recursion)
{
    if (localLocations.empty())
        return NULL;

    std::auto_ptr<DVcsJob> job(new DVcsJob(this));

    if (!prepareJob(job.get(), localLocations.front().path())) {
        return NULL;
    }

    *job << "hg" << "revert" << "--";
    if (!addDirsConditionally(job.get(), localLocations, recursion)) {
        return NULL;
    }

    return job.release();
}

VcsJob* MercurialPlugin::log(const KUrl& localLocation,
                const VcsRevision& rev,
                unsigned long limit)
{
    Q_UNUSED(rev)
    Q_UNUSED(limit)
    std::auto_ptr<DVcsJob> job(new DVcsJob(this));

    if (!prepareJob(job.get(), localLocation.path())) {
        return NULL;
    }

    *job << "hg" << "log" << "--template" << "{file_copies}\\0{file_dels}\\0{file_adds}\\0{file_mods}\\0{desc}\\0{date|isodate}\\0{author}\\0{parents}\\0{node}\\0{rev}\\0" << "--";

    addFileList(job.get(), localLocation);
    connect(job.get(), SIGNAL(readyForParsing(DVcsJob*)), SLOT(parseLogOutputBasicVersionControl(DVcsJob*)));
    return job.release();
}

VcsJob* MercurialPlugin::log(const KUrl& localLocation,
                const VcsRevision& rev,
                const VcsRevision& limit)
{
    Q_UNUSED(limit)
    return log(localLocation, rev, 0);
}

VcsJob* MercurialPlugin::annotate(const KUrl& localLocation,
                            const VcsRevision& rev)
{
    if (!localLocation.isLocalFile())
        return NULL;

    std::auto_ptr<DVcsJob> job(new DVcsJob(this));

    if (!prepareJob(job.get(), localLocation.path())) {
        return NULL;
    }

    *job << "hg" << "annotate" << "-n" << "-u" << "-d";
    QString srev = toMercurialRevision(rev);

    if (srev != QString::null && !srev.isEmpty())
        *job << "-r" << srev;

    *job << "--";

    *job << localLocation.path();
    connect(job.get(), SIGNAL(readyForParsing(DVcsJob*)), SLOT(parseAnnotations(DVcsJob*)));

    return job.release();
}


DVcsJob* MercurialPlugin::switchBranch(const QString &repository, const QString &branch)
{
    std::auto_ptr<DVcsJob> job(new DVcsJob(this));

    if (!prepareJob(job.get(), repository)) {
        return NULL;
    }

    *job << "hg" << "update" << "--" << branch;

    return job.release();
}

DVcsJob* MercurialPlugin::branch(const QString &repository, const QString &basebranch, const QString &branch,
                                   const QStringList &args)
{
    Q_UNUSED(repository)
    Q_UNUSED(basebranch)
    Q_UNUSED(branch)

    if (args.size() > 0) // Mercurial doesn't support rename or delete operations, which are hidden in the args of the function call
        return NULL;

    std::auto_ptr<DVcsJob> job(new DVcsJob(this));

    if (!prepareJob(job.get(), repository)) {
        return NULL;
    }

    if (basebranch != curBranch(repository)) { // I'm to lazy to support branching from different branch (which isn't used the GUI)
        return NULL;
    }

    *job << "hg" << "branch" << "--" << branch;

    return job.release();
}

VcsJob* MercurialPlugin::reset(const KUrl &repository, const QStringList &args, const KUrl::List& files)
{
    std::auto_ptr<DVcsJob> job(new DVcsJob(this));

    if (!prepareJob(job.get(), repository.path())) {
        return NULL;
    }

    *job << "hg" << "revert";

    if (!args.isEmpty())
        *job << args;

    if (!files.isEmpty()) {
        *job << "--";
        addFileList(job.get(), files);
    } else
        *job << "-a";

    return job.release();
}

QString MercurialPlugin::curBranch(const QString &repository)
{
    std::auto_ptr<DVcsJob> job(new DVcsJob(this));

    if (!prepareJob(job.get(), repository)) {
        return NULL;
    }

    *job << "hg" << "branch";

    if (!job->exec() || job->status() != VcsJob::JobSucceeded)
        return QString();

    return job->output().simplified();  // Strip the final newline. Mercurial does not allow whitespaces at beginning or end
}

QStringList MercurialPlugin::branches(const QString &repository)
{
    std::auto_ptr<DVcsJob> job(new DVcsJob(this));

    if (!prepareJob(job.get(), repository)) {
        return QStringList();
    }

    *job << "hg" << "branches" << "-q";

    if (!job->exec() || job->status() != VcsJob::JobSucceeded)
        return QStringList();

    return job->output().split('\n', QString::SkipEmptyParts);
}

QList<DVcsEvent> MercurialPlugin::getAllCommits(const QString &repo)
{
    std::auto_ptr<DVcsJob> job(new DVcsJob(this));
    job->setAutoDelete(false);

    if (!prepareJob(job.get(), repo)) {
        return QList<DVcsEvent>();
    }

    *job << "hg" << "log" << "--template" << "{desc}\\0{date|isodate}\\0{author}\\0{parents}\\0{node}\\0{rev}\\0";

    if (!job->exec() || job->status() != VcsJob::JobSucceeded)
        return QList<DVcsEvent>();

    QList<DVcsEvent> commits;

    parseLogOutput(job.get(), commits);

    return commits;
}

void MercurialPlugin::parseDiff(DVcsJob* job) const
{
    VcsDiff diff;
    diff.setType(VcsDiff::DiffUnified);
    diff.setDiff(job->output());
    job->setResults(qVariantFromValue(diff));
}


bool MercurialPlugin::parseAnnotations(DVcsJob *job) const
{
    if (job->status() != VcsJob::JobSucceeded)
        return false;

    QStringList lines = job->output().split('\n', QString::SkipEmptyParts); // Drops the final empty line, all other are prefixed
    QList<QVariant> result;
    static const QString reAnnotPat("\\s*(\\S+)\\s+(\\d+)\\s+(\\w+ \\w+ \\d\\d \\d\\d:\\d\\d:\\d\\d \\d\\d\\d\\d .\\d\\d\\d\\d): ([^\n]*)");
    QRegExp reAnnot(reAnnotPat, Qt::CaseSensitive, QRegExp::RegExp2);
    unsigned int lineNumber = 0;
    foreach(const QString & line, lines) {
        if (!reAnnot.exactMatch(line)) {
            kDebug() << "Could not parse annotation line: \"" << line << '\"';
            return false;
        }
        VcsAnnotationLine annotation;
        annotation.setLineNumber(lineNumber++);
        annotation.setAuthor(reAnnot.cap(1));
        annotation.setText(reAnnot.cap(4));

        bool success = false;
        qlonglong rev = reAnnot.cap(2).toLongLong(&success);
        if (!success) {
            kDebug() << "Could not parse revision in annotation line: \"" << line << '\"';
            return false;
        }

        // TODO: Doesn't work, but it should be a RFCDate as described in KDateString
        KDateTime dt = KDateTime::fromString(reAnnot.cap(3), KDateTime::RFCDate);
        annotation.setDate(dt.dateTime());

        VcsRevision vcsrev;
        vcsrev.setRevisionValue(rev, VcsRevision::GlobalNumber);
        annotation.setRevision(vcsrev);
        result.push_back(qVariantFromValue(annotation));
    }

    job->setResults(result);

    return true;
}

void MercurialPlugin::parseLogOutputBasicVersionControl(DVcsJob* job) const
{
    QList<QVariant> events;
    static unsigned int entriesPerCommit = 10;
    QList<QByteArray> items = job->rawOutput().split('\0');

    if (uint(items.size()) < entriesPerCommit || 1 != (items.size() % entriesPerCommit)) {
        kDebug() << "Cannot parse commit log: unexpected number of entries";
        return;
    }

    bool success = false;

    QString const & lastRev = items.at(entriesPerCommit - 1);
    qlonglong id = lastRev.toLongLong(&success);

    if (!success) {
        kDebug() << "Could not parse last revision \"" << lastRev << '"';
        id = 1024;
    }

    typedef std::reverse_iterator<QList<QByteArray>::const_iterator> QStringListReverseIterator;
    QStringListReverseIterator rbegin(items.end() - 1), rend(items.begin());  // Skip the final 0
    qlonglong lastId;

    while (rbegin != rend) {
        QString rev = *rbegin++;
        QString node = *rbegin++;
        QString parents = *rbegin++;
        QString author = *rbegin++;
        QString date = *rbegin++;
        QString desc = *rbegin++;
        QString mods = *rbegin++;
        QString adds = *rbegin++;
        QString dels = *rbegin++;
        QString copies = *rbegin++;
        lastId = id;
        id = rev.toLongLong(&success);

        if (!success) {
            kDebug() << "Could not parse revision \"" << rev << '"';
            return;
        }

        VcsEvent event;
        VcsRevision revision;
        revision.setRevisionValue(id, VcsRevision::GlobalNumber);
        event.setRevision(revision);
        event.setAuthor(author);
        event.setDate(QDateTime::fromString( date, Qt::ISODate ));
        event.setMessage(desc);

        QList<VcsItemEvent> items;

        // TODO: Convince Mercurial to separate the files with newlines, in order to allow whitespaces in filenames
        foreach (const QString & file, mods.split(QChar(' '), QString::SkipEmptyParts)) {
            VcsItemEvent item;
            item.setActions(VcsItemEvent::ContentsModified);
            item.setRevision(revision);
            item.setRepositoryLocation(file);
            items.push_back(item);
        }

        foreach (const QString & file, adds.split(QChar(' '), QString::SkipEmptyParts)) {
            VcsItemEvent item;
            item.setActions(VcsItemEvent::Added);
            item.setRevision(revision);
            item.setRepositoryLocation(file);
            items.push_back(item);
        }

        foreach (const QString & file, dels.split(QChar(' '), QString::SkipEmptyParts)) {
            VcsItemEvent item;
            item.setActions(VcsItemEvent::Deleted);
            item.setRevision(revision);
            item.setRepositoryLocation(file);
            items.push_back(item);
        }
#if 0   // Currently not implemented due to lack of a new version of mercurial
        foreach (const QString & file, copies.split(QChar(' '), QString::SkipEmptyParts)) {
            VcsItemEvent item;
            item.setActions(VcsItemEvent::Copied);
            item.setRevision(revision);
            item.setRepositoryLocation(file);
//             item.setRepositoryCopySourceLocation( const QString& );
//             item.setRepositoryCopySourceRevision( const VcsRevision& );
            items.push_back(item);
        }
#endif
        event.setItems(items);
        events.push_front(qVariantFromValue(event));
    }
    job->setResults(QVariant(events));
}

void MercurialPlugin::parseLogOutput(const DVcsJob * job, QList<DVcsEvent>& commits) const
{
    static unsigned int entriesPerCommit = 6;
    QList<QByteArray> items = job->rawOutput().split('\0');

    if (uint(items.size()) < entriesPerCommit || 1 != (items.size() % entriesPerCommit)) {
        kDebug() << "Cannot parse commit log: unexpected number of entries";
        return;
    }

    bool success = false;

    QString const & lastRev = items.at(entriesPerCommit - 1);
    unsigned int id = lastRev.toUInt(&success);

    if (!success) {
        kDebug() << "Could not parse last revision \"" << lastRev << '"';
        id = 1024;
    }

    QVector<QString> fullIds(id + 1);

    typedef std::reverse_iterator<QList<QByteArray>::const_iterator> QStringListReverseIterator;
    QStringListReverseIterator rbegin(items.end() - 1), rend(items.begin());  // Skip the final 0
    unsigned int lastId;

    while (rbegin != rend) {
        QString rev = *rbegin++;
        QString node = *rbegin++;
        QString parents = *rbegin++;
        QString author = *rbegin++;
        QString date = *rbegin++;
        QString desc = *rbegin++;
        lastId = id;
        id = rev.toUInt(&success);

        if (!success) {
            kDebug() << "Could not parse revision \"" << rev << '"';
            return;
        }

        if (uint(fullIds.size()) <= id) {
            fullIds.resize(id*2);
        }

        fullIds[id] = node;

        DVcsEvent commit;
        commit.setCommit(node);
        commit.setAuthor(author);
        commit.setDate(date);
        commit.setLog(desc);

        if (id == 0) {
            commit.setType(DVcsEvent::INITIAL);
        } else {
            if (parents.isEmpty() && id != 0) {
                commit.setParents(QStringList(fullIds[lastId]));
            } else {
                QStringList parentList;
                QStringList unparsedParentList = parents.split(QChar(' '), QString::SkipEmptyParts);
                // id:Short-node
                static const unsigned int shortNodeSuffixLen = 13;
                foreach(QString p, unparsedParentList) {
                    QString ids = p.left(p.size() - shortNodeSuffixLen);
                    id = ids.toUInt(&success);

                    if (!success) {
                        kDebug() << "Could not parse parent-revision \"" << ids << "\" of revision " << rev;
                        return;
                    }

                    parentList.push_back(fullIds[id]);
                }

                commit.setParents(parentList);
            }
        }

        commits.push_front(commit);
    }
}

QStringList MercurialPlugin::getLsFiles(const QString &directory, const QStringList &args)
{
    std::auto_ptr<DVcsJob> job(new DVcsJob(this));

    if (!prepareJob(job.get(), directory)) {
        return QStringList();
    }

    *job << "hg" << "status" << "-n";

    if (!args.isEmpty())
        *job << args;

    if (!job->exec() || job->status() != VcsJob::JobSucceeded)
        return QStringList();

    const QString prefix = directory.endsWith(QDir::separator()) ? directory : directory + QDir::separator();
    QStringList fileList = job->output().split('\n', QString::SkipEmptyParts);
    for (QStringList::iterator it = fileList.begin(); it != fileList.end(); ++it) {
        it->prepend(prefix);
    }
    return fileList;
}

struct isDirectory
{
    bool operator()(KUrl const & url) const {
        return QFileInfo(url.toLocalFile()).isDir();
    }
};

bool MercurialPlugin::addDirsConditionally(DVcsJob* job, const KUrl::List & locations, IBasicVersionControl::RecursionMode recursion)
{
    if (locations.empty())
        return IBasicVersionControl::Recursive == recursion;

    if (IBasicVersionControl::Recursive == recursion) {   // hg operates recursively on directories by default
        return addFileList(job, locations);
    }

    KUrl::List localFileLocations;
    std::remove_copy_if(locations.begin(), locations.end(), std::back_inserter(localFileLocations), isDirectory());
    if (localFileLocations.empty())
        return false;

    return addFileList(job, localFileLocations);
}

VcsStatusInfo::State MercurialPlugin::charToState(const char ch)
{
    switch (ch) {
    case 'M':
        return VcsStatusInfo::ItemModified;
    case 'A':
        return VcsStatusInfo::ItemAdded;
    case 'R':
        return VcsStatusInfo::ItemDeleted;
    case 'C':
        return VcsStatusInfo::ItemUpToDate;
    case '!':   // Missing
        return VcsStatusInfo::ItemUserState;
        //ToDo: hasConflicts
    default:
        return VcsStatusInfo::ItemUnknown;
    }
}

QString MercurialPlugin::toMercurialRevision(const VcsRevision & vcsrev)
{
    switch (vcsrev.revisionType()) {
    case VcsRevision::Special:
        switch (VcsRevision::RevisionSpecialType(vcsrev.revisionValue().toInt())) {
        case VcsRevision::Head:
        case VcsRevision::Base:
            return QString("tip");
        case VcsRevision::Working:
            return QString("");
        case VcsRevision::Previous:
            return QString();   // TODO: needs to be implemented
        case VcsRevision::Start:
            return QString("0");
        default:
            return QString();
        }
    case VcsRevision::GlobalNumber:
        return QString::number(vcsrev.revisionValue().toLongLong());
    case VcsRevision::Date:
    case VcsRevision::FileNumber:   // No file number for mercurial
    default:
        return QString();
    }
}

// #include "mercurialplugin.moc"
