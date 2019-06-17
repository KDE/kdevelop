/*
 *   Copyright 2008,2011 Sebastian Kügler <sebas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "kdevelopsessions.h"

#include <QCollator>
#include <QDir>
#include <KLocalizedString>


#include <QDebug>
#include <QFile>
#include <KDirWatch>
#include <KToolInvocation>
#include <KConfig>
#include <KConfigGroup>
#include <QStandardPaths>

K_EXPORT_PLASMA_RUNNER(kdevelopsessions, KDevelopSessions)

bool kdevelopsessions_runner_compare_sessions(const Session &s1, const Session &s2)
{
    QCollator c;
    return c.compare(s1.name, s2.name) < 0;
}

KDevelopSessions::KDevelopSessions(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
{
    setObjectName(QStringLiteral("KDevelop Sessions"));
    setIgnoredTypes(Plasma::RunnerContext::File | Plasma::RunnerContext::Directory | Plasma::RunnerContext::NetworkLocation);

    loadSessions();

    // listen for changes to the list of kdevelop sessions
    auto *historyWatch = new KDirWatch(this);
    const QStringList sessiondirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                              QStringLiteral("kdevelop/sessions"), QStandardPaths::LocateDirectory);
    for (const QString& dir : sessiondirs) {
        historyWatch->addDir(dir);
    }
    connect(historyWatch, &KDirWatch::dirty, this, &KDevelopSessions::loadSessions);
    connect(historyWatch, &KDirWatch::created, this, &KDevelopSessions::loadSessions);
    connect(historyWatch, &KDirWatch::deleted, this, &KDevelopSessions::loadSessions);

    Plasma::RunnerSyntax s(QStringLiteral(":q:"), i18n("Finds KDevelop sessions matching :q:."));
    s.addExampleQuery(QStringLiteral("kdevelop :q:"));
    addSyntax(s);

    setDefaultSyntax(Plasma::RunnerSyntax(QStringLiteral("kdevelop"), i18n("Lists all the KDevelop editor sessions in your account.")));
}

KDevelopSessions::~KDevelopSessions() = default;

QStringList findSessions()
{
    const QStringList sessionDirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                              QStringLiteral("kdevelop/sessions"),
                                                              QStandardPaths::LocateDirectory);
    QStringList sessionrcs;
    for (const QString& dir : sessionDirs) {
        QDir d(dir);
        const auto dirEntries = d.entryList(QDir::Dirs);
        for (const QString& sessionDir : dirEntries) {
            QDir sd(d.absoluteFilePath(sessionDir));
            QString path(sd.filePath(QStringLiteral("sessionrc")));
            if(QFile::exists(path)) {
                sessionrcs += path;
            }
        }
    }
    return sessionrcs;
}

void KDevelopSessions::loadSessions()
{
    m_sessions.clear();
    // Switch kdevelop session: -u
    // Should we add a match for this option or would that clutter the matches too much?
    const QStringList list = findSessions();
    m_sessions.reserve(list.size());
    for (const QString& sessionfile : list) {
        Session session;
        session.id = sessionfile.section(QLatin1Char('/'), -2, -2);
        KConfig cfg(sessionfile, KConfig::SimpleConfig);
        KConfigGroup group = cfg.group(QString());
        session.name = group.readEntry("SessionPrettyContents");
        m_sessions << session;
    }
    std::sort(m_sessions.begin(), m_sessions.end(), kdevelopsessions_runner_compare_sessions);
}

void KDevelopSessions::match(Plasma::RunnerContext &context)
{
    if (m_sessions.isEmpty()) {
        return;
    }

    QString term = context.query();
    if (term.length() < 3) {
        return;
    }

    bool listAll = false;

    if (term.startsWith(QStringLiteral("kdevelop"), Qt::CaseInsensitive)) {
        if (term.trimmed().compare(QStringLiteral("kdevelop"), Qt::CaseInsensitive) == 0) {
            listAll = true;
            term.clear();
        } else if (term.at(8) == QLatin1Char(' ') ) {
            term.remove(QStringLiteral("kdevelop"), Qt::CaseInsensitive);
            term = term.trimmed();
        } else {
            term.clear();
        }
    }

    if (term.isEmpty() && !listAll) {
        return;
    }

    for (const Session& session : qAsConst(m_sessions)) {
        if (!context.isValid()) {
            return;
        }

        if (listAll || (!term.isEmpty() && session.name.contains(term, Qt::CaseInsensitive))) {
            Plasma::QueryMatch match(this);
            if (listAll) {
                // All sessions listed, but with a low priority
                match.setType(Plasma::QueryMatch::ExactMatch);
                match.setRelevance(0.8);
            } else {
                if (session.name.compare(term, Qt::CaseInsensitive) == 0) {
                    // parameter to kdevelop matches session exactly, bump it up!
                    match.setType(Plasma::QueryMatch::ExactMatch);
                    match.setRelevance(1.0);
                } else {
                    // fuzzy match of the session in "kdevelop $session"
                    match.setType(Plasma::QueryMatch::PossibleMatch);
                    match.setRelevance(0.8);
                }
            }
            match.setIconName(QStringLiteral("kdevelop"));
            match.setData(session.id);
            match.setText(session.name);
            match.setSubtext(i18n("Open KDevelop Session"));
            context.addMatch(match);
        }
    }
}

void KDevelopSessions::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)
    QString sessionId = match.data().toString();
    if (sessionId.isEmpty()) {
        qWarning() << "No KDevelop session id in match!";
        return;
    }
    qDebug() << "Open KDevelop session" << sessionId;
    const QStringList args = {QStringLiteral("--open-session"), sessionId};
    KToolInvocation::kdeinitExec(QStringLiteral("kdevelop"), args);
}

#include "kdevelopsessions.moc"
