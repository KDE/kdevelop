/*
 * Copyright 2015 Laszlo Kis-Adam
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


#ifndef IPROBLEM_H
#define IPROBLEM_H

#include <QExplicitlySharedDataPointer>
#include <QString>
#include <QVector>

#include <language/editor/documentrange.h>

namespace KDevelop
{
class IAssistant;

/// Interface for the Problem classes
class IProblem : public QSharedData
{
public:
    typedef QExplicitlySharedDataPointer<IProblem> Ptr;

    /// The source of the problem. That is which tool / which part found this problem.
    enum Source {
        Unknown,
        Disk,
        Preprocessor,
        Lexer,
        Parser,
        DUChainBuilder,
        SemanticAnalysis,
        ToDo,
        Plugin             /// The source is a problem checker plugin
    };

    /// Severity of the problem. That is, how serious is the found problem.
    enum Severity {
        Error,
        Warning,
        Hint
    };


    IProblem(){}
    virtual ~IProblem(){}

    /// Returns the source of the problem
    virtual Source source() const = 0;

    /// Sets the source of the problem
    virtual void setSource(Source source) = 0;

    /// Returns a string containing the source of the problem
    virtual QString sourceString() const = 0;

    /// Returns the location of the problem (path, line, column)
    virtual KDevelop::DocumentRange finalLocation() const = 0;

    /// Sets the location of the problem (path, line, column)
    virtual void setFinalLocation(const KDevelop::DocumentRange& location) = 0;

    /// Returns the short description of the problem.
    virtual QString description() const = 0;

    /// Sets the short description of the problem
    virtual void setDescription(const QString& description) = 0;

    /// Returns the detailed explanation of the problem.
    virtual QString explanation() const = 0;

    /// Sets the detailed explanation of the problem
    virtual void setExplanation(const QString& explanation) = 0;

    /// Returns the severity of the problem
    virtual Severity severity() const = 0;

    /// Sets the severity of the problem
    virtual void setSeverity(Severity severity) = 0;

    /// Returns a string containing the severity of the problem
    virtual QString severityString() const = 0;

    /// Returns the diagnostics of the problem.
    virtual QVector<Ptr> diagnostics() const = 0;

    /// Sets the diagnostics of the problem
    virtual void setDiagnostics(const QVector<Ptr> &diagnostics) = 0;

    /// Adds a diagnostic line to the problem
    virtual void addDiagnostic(const Ptr &diagnostic) = 0;

    /// Clears all diagnostics
    virtual void clearDiagnostics() = 0;

    /// Returns a solution assistant for the problem, if applicable that is.
    virtual QExplicitlySharedDataPointer<KDevelop::IAssistant> solutionAssistant() const = 0;
};

}

#endif
