/*
 * Copyright 2014 Kevin Funk <kfunk@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#ifndef CLANGPROBLEM_H
#define CLANGPROBLEM_H

#include "clangprivateexport.h"

#include <language/duchain/problem.h>
#include <language/editor/documentrange.h>
#include <interfaces/iassistant.h>

#include <clang-c/Index.h>

struct KDEVCLANGPRIVATE_EXPORT ClangFixit
{
    QString replacementText;
    KDevelop::DocumentRange range;
    QString description;
    QString currentText;

    bool operator==(const ClangFixit& other) const
    {
        return replacementText == other.replacementText
            && range == other.range
            && description == other.description
            && currentText == other.currentText;
    }
};
Q_DECLARE_TYPEINFO(ClangFixit, Q_MOVABLE_TYPE);

namespace QTest {
template<>
inline char *toString(const ClangFixit& fixit)
{
    QByteArray ba = "ClangFixit[replacementText=" + fixit.replacementText.toUtf8()
        + ", range=" + QByteArray(QTest::toString(fixit.range))
        + ", description=" + fixit.description.toUtf8()
        + ", currentText=" + fixit.currentText.toUtf8()
        + "]";
    return qstrdup(ba.data());
}
}

QDebug KDEVCLANGPRIVATE_EXPORT operator<<(QDebug debug, const ClangFixit& fixit);

using ClangFixits = QVector<ClangFixit>;

class KDEVCLANGPRIVATE_EXPORT ClangProblem : public KDevelop::Problem
{
public:
    using Ptr = QExplicitlySharedDataPointer<ClangProblem>;
    using ConstPtr = QExplicitlySharedDataPointer<const ClangProblem>;

    /**
     * Creates an empty ClangProblem.
     */
    ClangProblem();

    /**
     * Creates a deep copy of a ClangProblem.
     */
    ClangProblem(const ClangProblem& other);

    /**
     * Import @p diagnostic into a ClangProblem object
     *
     * @param[in] diagnostic To-be-imported clang diagnostic
     */
    ClangProblem(CXDiagnostic diagnostic, CXTranslationUnit unit);

    KDevelop::IAssistant::Ptr solutionAssistant() const override;

    ClangFixits fixits() const;
    void setFixits(const ClangFixits& fixits);

    /**
     * Retrieve all fixits of this problem and its child diagnostics
     *
     * @return A mapping of problem pointers to the list of associated fixits
     */
    ClangFixits allFixits() const;

private:
    ClangFixits m_fixits;
};


class KDEVCLANGPRIVATE_EXPORT ClangFixitAssistant : public KDevelop::IAssistant
{
    Q_OBJECT

public:
    explicit ClangFixitAssistant(const ClangFixits& fixits);
    ClangFixitAssistant(const QString& title, const ClangFixits& fixits);

    QString title() const override;

    void createActions() override;

    ClangFixits fixits() const;

private:
    QString m_title;
    ClangFixits m_fixits;
};

class KDEVCLANGPRIVATE_EXPORT ClangFixitAction : public KDevelop::IAssistantAction
{
    Q_OBJECT

public:
    explicit ClangFixitAction(const ClangFixit& fixit);

    QString description() const override;

public Q_SLOTS:
    void execute() override;

private:
    ClangFixit m_fixit;
};

#endif // CLANGPROBLEM_H
