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

#ifndef PROBLEM_H
#define PROBLEM_H

#include <interfaces/iproblem.h>

#include <shell/shellexport.h>
#include <language/editor/documentrange.h>
#include <QString>
#include <QList>

struct DetectedProblemPrivate;

namespace KDevelop
{

// Class that represents a problem.
// Provides an interface so problems can be stored in a ProblemStore.
// Which can then be decorated with a model and shown in the problems view.
// It is basically a mirror of DUChain's Problem class.
// However that class is strongly coupled with DUChain's internals due to DUChain's needs (special serialization).
// Everything is public since the class has no behavtior, it just wraps some data that is handled as one unit!
class KDEVPLATFORMSHELL_EXPORT DetectedProblem : public IProblem
{
public:
    DetectedProblem();
    virtual ~DetectedProblem();

    Source source() const override;
    void setSource(Source source) override;
    // Retrieves the string representation of the problem's source
    QString sourceString() const override;

    DocumentRange finalLocation() const override;
    void setFinalLocation(const DocumentRange& location) override;

    QString description() const override;
    void setDescription(const QString& description) override;

    QString explanation() const override;
    void setExplanation(const QString& explanation) override;

    Severity severity() const override;
    void setSeverity(Severity severity) override;

    // Retrives the string representation of the problem's severity
    QString severityString() const override;

    QVector<Ptr> diagnostics() const override;
    void setDiagnostics(const QVector<Ptr> &diagnostics) override;
    void addDiagnostic(const Ptr &diagnostic) override;
    void clearDiagnostics() override;

    // A solution assistant to the problem, implement in subclass!
    virtual QExplicitlySharedDataPointer<IAssistant> solutionAssistant() const override;

private:
    QScopedPointer<DetectedProblemPrivate> d;
};

}

#endif

