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
    DetectedProblemPrivate()
        : m_severity(KDevelop::IProblem::Error)
        , m_source(KDevelop::IProblem::Unknown)
    {
    }

    QString m_description;
    QString m_explanation;
    KDevelop::IProblem::Severity m_severity;
    KDevelop::IProblem::Source m_source;
    KDevelop::DocumentRange m_range;
    QVector<KDevelop::IProblem::Ptr> m_diagnostics;
};

namespace KDevelop
{

DetectedProblem::DetectedProblem()
    :d(new DetectedProblemPrivate())
{
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
    QString s;

    switch(d->m_source)
    {
    case Unknown: s = i18n("Unknown"); break;
    case Disk: s = i18n("Disk"); break;
    case Preprocessor: s = i18n("Preprocessor"); break;
    case Lexer: s = i18n("Lexer"); break;
    case Parser: s = i18n("Parser"); break;
    case DUChainBuilder: s = i18n("DuchainBuilder"); break;
    case SemanticAnalysis: s = i18n("Semantic analysis"); break;
    case ToDo: s = i18n("Todo"); break;
    case Plugin: s = i18n("Plugin"); break;
    }

    return s;
}

DocumentRange DetectedProblem::finalLocation() const
{
    return d->m_range;
}

void DetectedProblem::setFinalLocation(const DocumentRange &location)
{
    d->m_range = location;
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

