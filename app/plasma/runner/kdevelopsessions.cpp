/*
    SPDX-FileCopyrightText: 2008, 2011 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kdevelopsessions.h"

// KDevelopSessionsWatch
#include <kdevelopsessionswatch.h>
// KF
#include <KLocalizedString>
#include <KRunner/krunner_version.h>
// Qt
#include <QDebug>
#include <QCollator>

K_PLUGIN_CLASS_WITH_JSON(KDevelopSessions, "kdevelopsessions.json")

KDevelopSessions::KDevelopSessions(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args)
    : Plasma::AbstractRunner(parent, metaData, args)
{
    setObjectName(QStringLiteral("KDevelop Sessions"));

#if KRUNNER_VERSION < QT_VERSION_CHECK(5, 106, 0)
    Plasma::RunnerSyntax s(QStringLiteral(":q:"), i18n("Finds KDevelop sessions matching :q:."));
    s.addExampleQuery(QStringLiteral("kdevelop :q:"));
#else
    Plasma::RunnerSyntax s({QStringLiteral(":q:"), QStringLiteral("kdevelop :q:")},
                           i18n("Finds KDevelop sessions matching :q:."));
#endif
    addSyntax(s);

    addSyntax(Plasma::RunnerSyntax(QStringLiteral("kdevelop"), i18n("Lists all the KDevelop editor sessions in your account.")));
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
        const auto trimmedStrippedTerm = QStringView(term).mid(8).trimmed();
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
#include "moc_kdevelopsessions.cpp"
