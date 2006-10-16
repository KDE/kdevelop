/* KDevelop Problem Reporting Interface
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
#ifndef KDEVPROBLEMREPORTINGINTERFACE_H
#define KDEVPROBLEMREPORTINGINTERFACE_H

struct KDevProblemReport
{
    enum ProblemType { Warning, Error, UserType = 0x1000 };
     
    ///The url that the problem occured in
    KUrl url;

    ///Line number the problem is on
    quint32 line;

    ///The description of the problem
    QString problem;

    ///The area the problem is in. Can be used for filtering.
    QString area;

    ///The type of problem described by this report
    ProblemType type;
};


/**
 * An interface for reporting issues
 * @author Matt Rogers <mattr@kde.org>
 */
class KDevProblemReportingInterface
{
public:
    KDevProblemReportingInterface();
    ~KDevProblemReportingInterface();

    /**
     * Add a problem to the problem reporter.
     */
    void addProblem( const KDevProblemReport&  ) = 0;

    /**
     * Remove a problem from the problem reporter.
     */
    void removeProblem( const KDevProblemReport& ) = 0;
};

#endif
