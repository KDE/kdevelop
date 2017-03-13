/*
 * Copyright 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <shell/problem.h>
#include <interfaces/iassistant.h>

#include <KLocalizedString>

struct DetectedProblemPrivate
{
    DetectedProblemPrivate(const QString& pluginName)
        : m_pluginName(pluginName)
        , m_severity(KDevelop::IProblem::Error)
        , m_source(KDevelop::IProblem::Unknown)
        , m_finalLocationMode(KDevelop::IProblem::Range)
    {
    }

    QString m_description;
    QString m_explanation;
    QString m_pluginName;
    KDevelop::IProblem::Severity m_severity;
    KDevelop::IProblem::Source m_source;
    KDevelop::DocumentRange m_range;
    QVector<KDevelop::IProblem::Ptr> m_diagnostics;
    KDevelop::IProblem::FinalLocationMode m_finalLocationMode;
};

namespace KDevelop
{

DetectedProblem::DetectedProblem()
    : d(new DetectedProblemPrivate(i18n("Plugin")))
{
}

DetectedProblem::DetectedProblem(const QString& pluginName)
    : d(new DetectedProblemPrivate(pluginName))
{
    setSource(Plugin);
}

DetectedProblem::~DetectedProblem()
{
    clearDiagnostics();
}

IProblem::Source DetectedProblem::source() const
{
    return d->m_source;
}

void DetectedProblem::setSource(Source source)
{
    d->m_source = source;
}

QString DetectedProblem::sourceString() const
{
    switch(d->m_source)
    {
    case Unknown: return i18n("Unknown");
    case Disk: return i18n("Disk");
    case Preprocessor: return i18n("Preprocessor");
    case Lexer: return i18n("Lexer");
    case Parser: return i18n("Parser");
    case DUChainBuilder: return i18n("DuchainBuilder");
    case SemanticAnalysis: return i18n("Semantic analysis");
    case ToDo: return i18n("Todo");
    case Plugin: return d->m_pluginName;
    }

    return {};
}

DocumentRange DetectedProblem::finalLocation() const
{
    return d->m_range;
}

void DetectedProblem::setFinalLocation(const DocumentRange &location)
{
    d->m_range = location;
}

IProblem::FinalLocationMode DetectedProblem::finalLocationMode() const
{
    return d->m_finalLocationMode;
}

void DetectedProblem::setFinalLocationMode(IProblem::FinalLocationMode mode)
{
    d->m_finalLocationMode = mode;
}

QString DetectedProblem::description() const
{
    return d->m_description;
}

void DetectedProblem::setDescription(const QString &description)
{
    d->m_description = description;
}

QString DetectedProblem::explanation() const
{
    return d->m_explanation;
}

void DetectedProblem::setExplanation(const QString &explanation)
{
    d->m_explanation = explanation;
}

IProblem::Severity DetectedProblem::severity() const
{
    return d->m_severity;
}

void DetectedProblem::setSeverity(Severity severity)
{
    d->m_severity = severity;
}

QString DetectedProblem::severityString() const
{
    QString s;

    switch(d->m_severity)
    {
    case Hint: s = i18n("Hint"); break;
    case Warning: s = i18n("Warning"); break;
    case Error: s = i18n("Error"); break;
    default: break;
    }

    return s;
}

QVector<IProblem::Ptr> DetectedProblem::diagnostics() const
{
    return d->m_diagnostics;
}

void DetectedProblem::setDiagnostics(const QVector<Ptr> &diagnostics)
{
    clearDiagnostics();

    foreach (const Ptr &diagnostic, diagnostics) {
        addDiagnostic(diagnostic);
    }
}

void DetectedProblem::addDiagnostic(const Ptr &diagnostic)
{
    DetectedProblem *dp = dynamic_cast<DetectedProblem*>(diagnostic.data());

    Q_ASSERT(dp);

    d->m_diagnostics.push_back(diagnostic);
}

void DetectedProblem::clearDiagnostics()
{
    d->m_diagnostics.clear();
}


QExplicitlySharedDataPointer<IAssistant> DetectedProblem::solutionAssistant() const
{
    return {};
}

}

