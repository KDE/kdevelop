/*
 * This file is part of KDevelop
 * Copyright 2020  Morten Danielsen Volden <mvolden2@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SHELLCHECK_SHELLCHECKPROBLEM_H
#define SHELLCHECK_SHELLCHECKPROBLEM_H

#include "fixit.h"

#include <shell/problem.h>
#include <interfaces/iassistant.h>


#include <QVector>

namespace shellcheck {

/**
 * @todo write docs
 */
class ShellCheckProblem : public KDevelop::DetectedProblem
{
public:
    using Ptr = QExplicitlySharedDataPointer<ShellCheckProblem>;

     /**
     * Creates an empty ShellCheckProblem .
     */
    ShellCheckProblem ();

    void setFixits(const QVector<Fixit>& fixits);

    /**
     * Get a solution assistant if there are any fixits for the problem
     *
     * @return a solution assitant for shellcheck fixes
     */
    KDevelop::IAssistant::Ptr solutionAssistant() const override;
private:
    QVector<Fixit> m_fixits;
};

}

#endif // SHELLCHECK_SHELLCHECKPROBLEM_H
