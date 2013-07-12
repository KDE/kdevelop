/*
 * KDevelop Class Browser
 *
 * Copyright 2009 Milian Wolff <mail@milianw.de>
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
#ifndef KDEVPLATFORM_PLUGIN_PROJECTFOLDER_H
#define KDEVPLATFORM_PLUGIN_PROJECTFOLDER_H

#include "documentclassesfolder.h"

namespace KDevelop
{
  class IProject;
}

namespace ClassModelNodes
{

/// Special folder
/// It displays all items of a given project.
class ProjectFolder : public DocumentClassesFolder
{
  Q_OBJECT
public:
  ProjectFolder(NodesModelInterface* a_model, KDevelop::IProject* project);
  ProjectFolder(NodesModelInterface* a_model);

  virtual void populateNode();

protected:
  KDevelop::IProject* m_project;
};

/// Filter for the project folder.
/// TODO: can't we share some code with FilteredAllClassesFolder somehow?
class FilteredProjectFolder : public ProjectFolder
{
  Q_OBJECT
public:
  FilteredProjectFolder(NodesModelInterface* a_model, KDevelop::IProject* project);

public: // Operations.
  /// Call this to update the classes filter string.
  void updateFilterString(QString a_newFilterString);

private: // DocumentClassesFolder overrides
  virtual bool isClassFiltered(const KDevelop::QualifiedIdentifier& a_id);

private:
  /// We'll use this string to display only classes that match this string.
  QString m_filterString;
};

}

#endif // KDEVPLATFORM_PLUGIN_PROJECTFOLDER_H
