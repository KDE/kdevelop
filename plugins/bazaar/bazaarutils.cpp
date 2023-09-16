/*
    SPDX-FileCopyrightText: 2013-2014 Maciej Poleski

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "bazaarutils.h"

#include <QDateTime>
#include <QDebug>

#include <vcs/vcsrevision.h>
#include <vcs/vcsstatusinfo.h>
#include <vcs/vcsevent.h>

QDir BazaarUtils::toQDir(const QUrl& url)
{
    return QDir(url.toLocalFile());
}

QDir BazaarUtils::workingCopy(const QUrl& path)
{
    QDir dir = BazaarUtils::toQDir(path);
    while (!dir.exists(QStringLiteral(".bzr")) && dir.cdUp());

    return dir;
}

QString BazaarUtils::getRevisionSpec(const KDevelop::VcsRevision& revision)
{
    if (revision.revisionType() == KDevelop::VcsRevision::Special) {
        if (revision.specialType() == KDevelop::VcsRevision::Head)
            return QStringLiteral("-rlast:1");
        else if (revision.specialType() == KDevelop::VcsRevision::Base)
            return QString();  // Workaround strange KDevelop behaviour
        else if (revision.specialType() == KDevelop::VcsRevision::Working)
            return QString();
        else if (revision.specialType() == KDevelop::VcsRevision::Start)
            return QStringLiteral("-r1");
        else
            return QString(); // Don't know how to handle this situation
    } else if (revision.revisionType() == KDevelop::VcsRevision::GlobalNumber)
        return QLatin1String("-r") + QString::number(revision.revisionValue().toLongLong());
    else
        return QString(); // Don't know how to handle this situation
}

QString BazaarUtils::getRevisionSpecRange(const KDevelop::VcsRevision& end)
{
    if (end.revisionType() == KDevelop::VcsRevision::Special) {
        if (end.specialType() == KDevelop::VcsRevision::Head) {
            return QStringLiteral("-r..last:1");
        } else if (end.specialType() == KDevelop::VcsRevision::Base) {
            return QStringLiteral("-r..last:1"); // Workaround strange KDevelop behaviour
        } else if (end.specialType() == KDevelop::VcsRevision::Working) {
            return QString();
        } else if (end.specialType() == KDevelop::VcsRevision::Start) {
            return QStringLiteral("-..r1");
        } else {
            return QString(); // Don't know how to handle this situation
        }
    } else if (end.revisionType() == KDevelop::VcsRevision::GlobalNumber) {
        return QStringLiteral("-r") + QString::number(end.revisionValue().toLongLong());
    }

    return QString();    // Don't know how to handle this situation
}

QString BazaarUtils::getRevisionSpecRange(const KDevelop::VcsRevision& begin,
                             const KDevelop::VcsRevision& end)
{
    if (begin.revisionType() == KDevelop::VcsRevision::Special) {
        if (begin.specialType() == KDevelop::VcsRevision::Previous) {
            if (end.revisionType() == KDevelop::VcsRevision::Special) {
                if (end.specialType() == KDevelop::VcsRevision::Base ||
                        end.specialType() == KDevelop::VcsRevision::Head)
                    return QStringLiteral("-rlast:2..last:1");
                else if (end.specialType() == KDevelop::VcsRevision::Working)
                    return QString();
                else if (end.specialType() == KDevelop::VcsRevision::Start)
                    return QStringLiteral("-r0..1");        // That's wrong revision range
            } else if (end.revisionType() == KDevelop::VcsRevision::GlobalNumber)
                return QStringLiteral("-r") +
                       QString::number(end.revisionValue().toLongLong() - 1)
                       + QLatin1String("..") + QString::number(end.revisionValue().toLongLong());
            else
                return QString(); // Don't know how to handle this situation
        } else if (begin.specialType() == KDevelop::VcsRevision::Base ||
                   begin.specialType() == KDevelop::VcsRevision::Head) {
            // Only one possibility: comparing working copy to last commit
            return QString();
        }
    } else if (begin.revisionType() == KDevelop::VcsRevision::GlobalNumber) {
        if (end.revisionType() == KDevelop::VcsRevision::Special) {
            // Assuming working copy
            return QLatin1String("-r") + QString::number(begin.revisionValue().toLongLong());
        } else {
            return QLatin1String("-r") + QString::number(begin.revisionValue().toLongLong())
                   + QLatin1String("..") + QString::number(end.revisionValue().toLongLong());
        }
    }
    return QString(); // Don't know how to handle this situation
}

bool BazaarUtils::isValidDirectory(const QUrl& dirPath)
{
    QDir dir = BazaarUtils::workingCopy(dirPath);

    return dir.cd(QStringLiteral(".bzr")) && dir.exists(QStringLiteral("branch"));
}

KDevelop::VcsStatusInfo BazaarUtils::parseVcsStatusInfoLine(const QString& line)
{
    const auto tokens = QStringView(line).split(QLatin1Char(' '), Qt::SkipEmptyParts);
    KDevelop::VcsStatusInfo result;
    if (tokens.size() < 2) // Don't know how to handle this situation (it is an error)
        return result;
    result.setUrl(QUrl::fromLocalFile(tokens.back().toString()));
    if (tokens[0] == QLatin1String("M")) {
        result.setState(KDevelop::VcsStatusInfo::ItemModified);
    } else if (tokens[0] == QLatin1String("C")) {
        result.setState(KDevelop::VcsStatusInfo::ItemHasConflicts);
    } else if (tokens[0] == QLatin1String("+N")) {
        result.setState(KDevelop::VcsStatusInfo::ItemAdded);
    } else if (tokens[0] == QLatin1String("?")) {
        result.setState(KDevelop::VcsStatusInfo::ItemUnknown);
    } else if (tokens[0] == QLatin1String("D")) {
        result.setState(KDevelop::VcsStatusInfo::ItemDeleted);
    } else {
        result.setState(KDevelop::VcsStatusInfo::ItemUserState);
        qWarning() << "Unsupported status: " << tokens[0];
    }
    return result;
}

QString BazaarUtils::concatenatePath(const QDir& workingCopy, const QUrl& pathInWorkingCopy)
{
    return QFileInfo(workingCopy.absolutePath() + QDir::separator()
                     + pathInWorkingCopy.toLocalFile()).absoluteFilePath();
}

KDevelop::VcsEvent BazaarUtils::parseBzrLogPart(const QString& output)
{
    const QStringList outputLines = output.split(QLatin1Char('\n'));
    KDevelop::VcsEvent commitInfo;
    bool atMessage = false;
    QString message;
    bool afterMessage = false;
    QHash<QString, KDevelop::VcsItemEvent::Actions> fileToActionsMapping;
    KDevelop::VcsItemEvent::Action currentAction;
    for (const QString &line : outputLines) {
        if (!atMessage) {
            if (line.startsWith(QLatin1String("revno"))) {
                QString revno = line.mid(QStringLiteral("revno: ").length());
                revno = revno.left(revno.indexOf(QLatin1Char(' ')));
                KDevelop::VcsRevision revision;
                revision.setRevisionValue(revno.toLongLong(), KDevelop::VcsRevision::GlobalNumber);
                commitInfo.setRevision(revision);
            } else if (line.startsWith(QLatin1String("committer: "))) {
                QString commiter = line.mid(QStringLiteral("committer: ").length());
                commitInfo.setAuthor(commiter);     // Author goes after committer, but only if is different
            } else if (line.startsWith(QLatin1String("author"))) {
                QString author = line.mid(QStringLiteral("author: ").length());
                commitInfo.setAuthor(author);       // It may override committer (In fact committer is not supported by VcsEvent)
            } else if (line.startsWith(QLatin1String("timestamp"))) {
                const QString formatString = QStringLiteral("yyyy-MM-dd hh:mm:ss");
                QString timestamp = line.mid(QStringLiteral("timestamp: ddd ").length(), formatString.length());
                commitInfo.setDate(QDateTime::fromString(timestamp, formatString));
            } else if (line.startsWith(QLatin1String("message"))) {
                atMessage = true;
            }
        } else if (atMessage && !afterMessage) {
            if (!line.isEmpty() && line[0].isSpace()) {
                message += line.trimmed() + QLatin1Char('\n');
            } else if (!line.isEmpty()) {
                afterMessage = true;
                // leave atMessage = true
                currentAction = BazaarUtils::parseActionDescription(line);
            } // if line is empty - ignore and get next
        } else if (afterMessage) {
            if (!line.isEmpty() && !line[0].isSpace()) {
                currentAction = BazaarUtils::parseActionDescription(line);
            } else if (!line.isEmpty()) {
                fileToActionsMapping[line.trimmed()] |= currentAction;
            } // if line is empty - ignore and get next
        }
    }
    if (atMessage)
        commitInfo.setMessage(message.trimmed());
    for (auto i = fileToActionsMapping.begin(); i != fileToActionsMapping.end(); ++i) {
        KDevelop::VcsItemEvent itemEvent;
        itemEvent.setRepositoryLocation(i.key());
        itemEvent.setActions(i.value());
        commitInfo.addItem(itemEvent);
    }
    return commitInfo;
}

KDevelop::VcsItemEvent::Action BazaarUtils::parseActionDescription(const QString& action)
{
    if (action == QLatin1String("added:")) {
        return KDevelop::VcsItemEvent::Added;
    } else if (action == QLatin1String("modified:")) {
        return KDevelop::VcsItemEvent::Modified;
    } else if (action == QLatin1String("removed:")) {
        return KDevelop::VcsItemEvent::Deleted;
    } else if (action == QLatin1String("kind changed:")) {
        return KDevelop::VcsItemEvent::Replaced; // Best approximation
    } else if (action.startsWith(QLatin1String("renamed"))) {
        return KDevelop::VcsItemEvent::Modified; // Best approximation
    } else {
        qCritical("Unsupported action: %s", action.toLocal8Bit().constData());
        return KDevelop::VcsItemEvent::Action();
    }
}

QList<QUrl> BazaarUtils::handleRecursion(const QList<QUrl>& listOfUrls, KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    if (recursion == KDevelop::IBasicVersionControl::Recursive) {
        return listOfUrls;      // Nothing to do
    } else {
        QList<QUrl> result;
        for (const auto& url : listOfUrls) {
            if (url.isLocalFile() && QFileInfo(url.toLocalFile()).isFile()) {
                result.push_back(url);
            }
        }
        return result;
    }
}
