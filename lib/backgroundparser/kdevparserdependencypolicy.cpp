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

#include "kdevparserdependencypolicy.h"

#include <QSet>

#include "kdevparsejob.h"

using namespace ThreadWeaver;

bool KDevParserDependencyPolicy::addDependency(KDevParseJob* dependency, KDevParseJob* primaryDependee, Job* actualDependee)
{
  QSet<KDevParseJob*> encountered;

  gatherDependencies(dependency, encountered);

  if (encountered.contains(primaryDependee))
    return false;

  m_dependencies.insert(dependency, primaryDependee);

  DependencyPolicy::addDependency(dependency, actualDependee ? actualDependee : static_cast<Job*>(primaryDependee));

  return true;
}

void KDevParserDependencyPolicy::destructed(Job *job)
{
  DependencyPolicy::destructed(job);

  m_dependencies.remove(static_cast<KDevParseJob*>(job));
}

void KDevParserDependencyPolicy::gatherDependencies(KDevParseJob* job, QSet<KDevParseJob*>& encountered) const
{
  encountered.insert(job);

  foreach (KDevParseJob* dependency, m_dependencies.values(job))
    gatherDependencies(dependency, encountered);
}
