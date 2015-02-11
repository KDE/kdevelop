/*
 * KDevelop Class Browser
 *
 * Copyright 2009 Lior Mualem <lior.m.kde@gmail.com>
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

#ifndef KDEVPLATFORM_PLUGIN_ALLCLASSESFOLDER_H
#define KDEVPLATFORM_PLUGIN_ALLCLASSESFOLDER_H

#include "documentclassesfolder.h"

namespace KDevelop
{
  class IProject;
}

namespace ClassModelNodes
{

/// Special folder.
/// It displays all the classes in the projects by using the IProject
class AllClassesFolder : public DocumentClassesFolder
{
  Q_OBJECT
public:
  AllClassesFolder(NodesModelInterface* a_model);

public: // Node overrides
  virtual void nodeCleared() override;
  virtual void populateNode() override;

private Q_SLOTS:
  // Project watching
  void projectOpened(KDevelop::IProject* project);
  void projectClosing(KDevelop::IProject* project);
};

/// Contains a filter for the all classes folder.
class FilteredAllClassesFolder : public AllClassesFolder
{
  Q_OBJECT
public:
  FilteredAllClassesFolder(NodesModelInterface* a_model);

public: // Operations.
  /// Call this to update the classes filter string.
  void updateFilterString(QString a_newFilterString);

private: // DocumentClassesFolder overrides
  virtual bool isClassFiltered(const KDevelop::QualifiedIdentifier& a_id) override;

private:
  /// We'll use this string to display only classes that match this string.
  QString m_filterString;
};

} // namespace ClassModelNodes

#endif
