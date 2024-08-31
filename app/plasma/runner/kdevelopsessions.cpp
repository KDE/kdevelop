/*
    SPDX-FileCopyrightText: 2008, 2011 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kdevelopsessions.h"

// KDevelopSessionsWatch
#include <kdevelopsessionswatch.h>
// KF
#include <KLocalizedString>
#include <KPluginFactory>
#include <krunner_version.h>
// Qt
#include <QDebug>
#include <QCollator>

K_PLUGIN_CLASS_WITH_JSON(KDevelopSessions, "kdevelopsessions.json")

KDevelopSessions::KDevelopSessions(QObject* parent, const KPluginMetaData& metaData)
    : KRunner::AbstractRunner(parent, metaData)
{
    setObjectName(QStringLiteral("KDevelop Sessions"));

    KRunner::RunnerSyntax s({QStringLiteral(":q:"), QStringLiteral("kdevelop :q:")},
                            i18n("Finds KDevelop sessions matching :q:."));
    addSyntax(s);

    addSyntax(KRunner::RunnerSyntax(QStringLiteral("kdevelop"),
                                    i18n("Lists all the KDevelop editor sessions in your account.")));
}

KDevelopSessions::~KDevelopSessions()
{
    KDevelopSessionsWatch::unregisterObserver(this);
}

void KDevelopSessions::init()
{
    KDevelopSessionsWatch::registerObserver(this);

    KRunner::AbstractRunner::init();
}

void KDevelopSessions::setSessionDataList(const QVector<KDevelopSessionData>& sessionDataList)
{
    m_sessionDataList = sessionDataList;
}

void KDevelopSessions::match(KRunner::RunnerContext& context)
{
    QString term = context.query();
    if (term.size() < 3) {
        return;
    }

    bool listAll = false;
    if (term.startsWith(QLatin1String("kdevelop"), Qt::CaseInsensitive)) {
        const auto trimmedStrippedTerm = QStringView{term}.sliced(8).trimmed();
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

    for (const auto& session : std::as_const(m_sessionDataList)) {
        if (!context.isValid()) {
            return;
        }

        if (listAll || (!term.isEmpty() && session.description.contains(term, Qt::CaseInsensitive))) {
            KRunner::QueryMatch match(this);
            if (listAll) {
                // All sessions listed, but with a low priority
                match.setCategoryRelevance(KRunner::QueryMatch::CategoryRelevance::Highest);
                match.setRelevance(0.8);
            } else {
                if (session.description.compare(term, Qt::CaseInsensitive) == 0) {
                    // parameter to kdevelop matches session exactly, bump it up!
                    match.setCategoryRelevance(KRunner::QueryMatch::CategoryRelevance::Highest);
                    match.setRelevance(1.0);
                } else {
                    // fuzzy match of the session in "kdevelop $session"
                    match.setCategoryRelevance(KRunner::QueryMatch::CategoryRelevance::Moderate);
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

void KDevelopSessions::run(const KRunner::RunnerContext& context, const KRunner::QueryMatch& match)
{
    Q_UNUSED(context)
    const QString sessionId = match.data().toString();
    qDebug() << "Open KDevelop session" << sessionId;
    KDevelopSessionsWatch::openSession(sessionId);
}

#include "kdevelopsessions.moc"
#include "moc_kdevelopsessions.cpp"
