/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef INCLUDEPATHCOMPUTER_H
#define INCLUDEPATHCOMPUTER_H

#include <KUrl>

#include "includepathresolver.h"

class IncludePathComputer
{
public:
  IncludePathComputer(const KUrl& file);
  ///Must be called in the foreground thread, before calling computeBackground().
  void computeForeground();
  ///Can be called from within background thread, but does not have to. May lock for a long time.
  void computeBackground();

  KUrl::List result() const
  {
    return m_ret;
  }

  QHash<QString,QString> defines() const
  {
    return m_defines;
  }

  KDevelop::ModificationRevisionSet m_includePathDependency;

private:
  void addInclude(KUrl url);

  QHash<QString,QString> m_defines;
  KUrl m_source;
  KUrl::List m_ret;
  QSet<KUrl> m_hasPath;
  bool m_ready;

  KUrl m_effectiveBuildDirectory;
  KUrl m_buildDirectory;
  KUrl m_projectDirectory;
  QString m_projectName;
  bool m_gotPathsFromManager;
  CppTools::IncludePathResolver m_includeResolver;
};
#endif // INCLUDEPATHCOMPUTER_H
