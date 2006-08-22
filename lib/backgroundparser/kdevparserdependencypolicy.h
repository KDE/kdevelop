/*
* This file is part of KDevelop
*
* Copyright (c) 2006 Hamish Rodda <rodda@kde.org>
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

#ifndef KDEV_PARSER_DEPENDENCYPOLICY_H
#define KDEV_PARSER_DEPENDENCYPOLICY_H

#include <DependencyPolicy.h>

#include <QMultiMap>

class KDevParseJob;

/// Internal class to manage dependencies between parse jobs.  Thus, not exported.
class KDevParserDependencyPolicy : public ThreadWeaver::DependencyPolicy
{
public:
  /**
    * Attempt to add \a dependency as a dependency of \a actualDependee, which must
    * be a subjob of \a primaryDependee, or null (in which case, the dependency is added
    * to \a primaryDependee).  If a circular dependency
    * is detected, the dependency will not be added and the method will return false.
    */
  bool addDependency(KDevParseJob* dependency, KDevParseJob* primaryDependee, ThreadWeaver::Job* actualDependee = 0);

  virtual void destructed(ThreadWeaver::Job *job);

private:
  void gatherDependencies(KDevParseJob* job, QSet<KDevParseJob*>& encountered) const;

  QMultiMap<KDevParseJob*, KDevParseJob*> m_dependencies;
};

#endif // KDEV_PARSER_DEPENDENCYPOLICY_H
