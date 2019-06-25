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

#ifndef PROBLEM_H
#define PROBLEM_H

#include <interfaces/iproblem.h>

#include <shell/shellexport.h>

#include <QScopedPointer>

namespace KDevelop
{
class DetectedProblemPrivate;

/**
 * @brief Represents a problem as one unit with the IProblem interface so can be used with anything that can handle IProblem.
 *
 * You should have it wrapped in an IProblem::Ptr which is a shared pointer for it.
 * It is basically a mirror of DUChain's Problem class.
 * However that class is strongly coupled with DUChain's internals due to DUChain's needs (special serialization).
 *
 * Usage example:
 * @code
 * IProblem::Ptr problem(new DetectedProblem());
 * problem->setSource(IProblem::Plugin);
 * problem->setSeverity(IProblem::Warning);
 * problem->setDescription(QStringLiteral("Warning message"));
 * problem->setExplanation(QStringLiteral("Warning explanation"));
 *
 * DocumentRange range;
 * range.document = IndexedString("/path/to/source/file");
 * range.setBothLines(1337);
 * range.setBothColumns(12);
 * problem->setFinalLocation(range);
 * @endcode
 *
 */
class KDEVPLATFORMSHELL_EXPORT DetectedProblem : public IProblem
{
public:
    /// Creates new empty DetectedProblem with default properties:
    /// severity - KDevelop::IProblem::Error;
    /// source - KDevelop::IProblem::Unknown;
    /// finalLocationMode - KDevelop::IProblem::Range.
    DetectedProblem();

    /// Creates new DetectedProblem, produced by some plugin, for example by analyzer plugins
    /// like cppcheck/clang-tydy/valgrind/etc.
    /// Default values of problem properties are:
    /// severity - KDevelop::IProblem::Error;
    /// source - KDevelop::IProblem::Plugin;
    /// sourceString - passed pluginName parameter;
    /// finalLocationMode - KDevelop::IProblem::Range.
    explicit DetectedProblem(const QString& pluginName);

    ~DetectedProblem() override;

    Source source() const override;
    void setSource(Source source) override;
    QString sourceString() const override;

    DocumentRange finalLocation() const override;
    void setFinalLocation(const DocumentRange& location) override;

    FinalLocationMode finalLocationMode() const override;
    void setFinalLocationMode(FinalLocationMode mode) override;

    QString description() const override;
    void setDescription(const QString& description) override;

    QString explanation() const override;
    void setExplanation(const QString& explanation) override;

    Severity severity() const override;
    void setSeverity(Severity severity) override;
    QString severityString() const override;

    QVector<Ptr> diagnostics() const override;
    void setDiagnostics(const QVector<Ptr> &diagnostics) override;
    void addDiagnostic(const Ptr &diagnostic) override;
    void clearDiagnostics() override;

    QExplicitlySharedDataPointer<IAssistant> solutionAssistant() const override;

private:
    const QScopedPointer<class DetectedProblemPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DetectedProblem)
};

}

#endif

