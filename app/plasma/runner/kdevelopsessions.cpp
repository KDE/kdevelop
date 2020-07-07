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

// KDevelopSessionsWatch
#include <kdevelopsessionswatch.h>
// KF
#include <KLocalizedString>
#include <krunner_version.h>
// Qt
#include <QDebug>
#include <QCollator>

#if KRUNNER_VERSION >= QT_VERSION_CHECK(5, 72, 0)
K_EXPORT_PLASMA_RUNNER_WITH_JSON(KDevelopSessions, "kdevelopsessions.json")
#else
K_EXPORT_PLASMA_RUNNER(kdevelopsessions, KDevelopSessions)
#endif

KDevelopSessions::KDevelopSessions(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
{
    setObjectName(QStringLiteral("KDevelop Sessions"));
    setIgnoredTypes(Plasma::RunnerContext::File | Plasma::RunnerContext::Directory | Plasma::RunnerContext::NetworkLocation);

    Plasma::RunnerSyntax s(QStringLiteral(":q:"), i18n("Finds KDevelop sessions matching :q:."));
    s.addExampleQuery(QStringLiteral("kdevelop :q:"));
    addSyntax(s);

    setDefaultSyntax(Plasma::RunnerSyntax(QStringLiteral("kdevelop"), i18n("Lists all the KDevelop editor sessions in your account.")));
}

KDevelopSessions::~KDevelopSessions()
{
    KDevelopSessionsWatch::unregisterObserver(this);
}

void KDevelopSessions::init()
{
    KDevelopSessionsWatch::registerObserver(this);

    Plasma::AbstractRunner::init();
}

void KDevelopSessions::setSessionDataList(const QVector<KDevelopSessionData>& sessionDataList)
{
    m_sessionDataList = sessionDataList;
}

void KDevelopSessions::match(Plasma::RunnerContext &context)
{
    QString term = context.query();
    if (term.size() < 3) {
        return;
    }

    bool listAll = false;
    if (term.startsWith(QLatin1String("kdevelop"), Qt::CaseInsensitive)) {
        const QStringRef trimmedStrippedTerm = term.midRef(8).trimmed();
        // "kdevelop" -> list all sessions
        if (trimmedStrippedTerm.isEmpty()) {
            listAll = true;
            term.clear();
        }
        // "kdevelop X" -> list all sessions with "X"
        else if (term.at(8) == QLatin1Char(' ') ) {
            term = trimmedStrippedTerm.toString();
        }
        // "kdevelopX" -> list all sessions with "kdevelopX"
        else {
            term = term.trimmed();
        }
    }

    if (term.isEmpty() && !listAll) {
        return;
    }

    for (const auto& session : qAsConst(m_sessionDataList)) {
        if (!context.isValid()) {
            return;
        }

        if (listAll || (!term.isEmpty() && session.description.contains(term, Qt::CaseInsensitive))) {
            Plasma::QueryMatch match(this);
            if (listAll) {
                // All sessions listed, but with a low priority
                match.setType(Plasma::QueryMatch::ExactMatch);
                match.setRelevance(0.8);
            } else {
                if (session.description.compare(term, Qt::CaseInsensitive) == 0) {
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
            match.setText(session.description);
            match.setSubtext(i18n("Open KDevelop Session"));
            context.addMatch(match);
        }
    }
}

void KDevelopSessions::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)
    const QString sessionId = match.data().toString();
    qDebug() << "Open KDevelop session" << sessionId;
    KDevelopSessionsWatch::openSession(sessionId);
}

#include "kdevelopsessions.moc"
