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
#include <QDebug>
#include <QFile>
#include <QStandardPaths>
#include <KLocalizedString>
#include <krunner_version.h>
#include <KDirWatch>
#include <KToolInvocation>
#include <DataEngine>
#include <DataContainer>

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
    m_engine = dataEngine(QStringLiteral("org.kde.kdevelopsessions"));

    connect(this, &KDevelopSessions::prepare, this, &KDevelopSessions::loadSessions);
    Plasma::RunnerSyntax s(QStringLiteral(":q:"), i18n("Finds KDevelop sessions matching :q:."));
    s.addExampleQuery(QStringLiteral("kdevelop :q:"));
    addSyntax(s);

    setDefaultSyntax(Plasma::RunnerSyntax(QStringLiteral("kdevelop"), i18n("Lists all the KDevelop editor sessions in your account.")));
}

KDevelopSessions::~KDevelopSessions() = default;

void KDevelopSessions::loadSessions()
{
    m_sessions.clear();
    // Switch kdevelop session: -u
    // Should we add a match for this option or would that clutter the matches too much?
    const QStringList list = m_engine->sources();
    m_sessions.reserve(list.size());
    for (const QString& sessionFile : list) {
        const Plasma::DataEngine::Data data = m_engine->containerForSource(sessionFile)->data();
        Session session;
        session.id = sessionFile;
        session.name = data.value(QStringLiteral("sessionString")).toString();
        m_sessions << session;
    }
    suspendMatching(m_sessions.isEmpty());
}

void KDevelopSessions::match(Plasma::RunnerContext &context)
{
    QString term = context.query();
    if (!term.startsWith(QLatin1String("kdevelop"), Qt::CaseInsensitive)) {
        return;
    }

    bool listAll = false;
    if (term.trimmed().compare(QLatin1String("kdevelop"), Qt::CaseInsensitive) == 0) {
        listAll = true;
        term.clear();
    } else if (term.at(8) == QLatin1Char(' ') ) {
        term.remove(0, 8);
        term = term.trimmed();
    } else {
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
    const QString sessionId = match.data().toString();
    qDebug() << "Open KDevelop session" << sessionId;
    const QStringList args = {QStringLiteral("--open-session"), sessionId};
    KToolInvocation::kdeinitExec(QStringLiteral("kdevelop"), args);
}

#include "kdevelopsessions.moc"
