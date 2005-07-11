//
// C++ Interface: 
//
// Description: 
//
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef PROJECTCONFIGURATIONDLG_H
#define PROJECTCONFIGURATIONDLG_H

#include "projectconfigurationdlgbase.h"
class QMakeFolderModel;
class ProjectConfigurationDlg : public ProjectConfigurationDlgBase
{
  Q_OBJECT

public:
  ProjectConfigurationDlg(QMakeFolderModel *model, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
  ~ProjectConfigurationDlg();
  /*$PUBLIC_FUNCTIONS$*/

public slots:
  /*$PUBLIC_SLOTS$*/
  virtual void          editCustomValueClicked();
  virtual void          customVariables_selectionChanged();
  virtual void          newCustomVariableActive();
  virtual void          downCustomValueClicked();
  virtual void          upCustomValueClicked();
  virtual void          removeCustomValueClicked();
  virtual void          addCustomValueClicked();
  virtual void          extMoveDown_button_clicked();
  virtual void          extMoveUp_button_clicked();
  virtual void          extEdit_button_clicked();
  virtual void          extRemove_button_clicked();
  virtual void          extAdd_button_clicked();
  virtual void          intMoveDown_button_clicked();
  virtual void          intMoveUp_button_clicked();
  virtual void          slotInstallTargetClicked();
  virtual void          slotStaticLibClicked(int);
  virtual void          prjDepsMoveDownClicked();
  virtual void          prjDepsMoveUpClicked();
  virtual void          outsideIncEditClicked();
  virtual void          outsideLibEditClicked();
  virtual void          outsideLibDirEditClicked();
  virtual void          outsideLibDirRemoveClicked();
  virtual void          outsideLibDirAddClicked();
  virtual void          outsideLibDirMoveUpClicked();
  virtual void          outsideLibDirMoveDownClicked();
  virtual void          outsideLibRemoveClicked();
  virtual void          outsideLibAddClicked();
  virtual void          outsideIncRemoveClicked();
  virtual void          outsideIncAddClicked();
  virtual void          insideLibMoveDownClicked();
  virtual void          insideLibMoveUpClicked();
  virtual void          outsideLibMoveDownClicked();
  virtual void          outsideLibMoveUpClicked();
  virtual void          insideIncMoveDownClicked();
  virtual void          insideIncMoveUpClicked();
  virtual void          outsideIncMoveDownClicked();
  virtual void          outsideIncMoveUpClicked();
  virtual void          buildorderMoveDownClicked();
  virtual void          buildorderMoveUpClicked();
  virtual void          templateLibraryClicked(int);
  virtual void          updateProjectConfiguration();
  virtual void          clickSubdirsTemplate();
  virtual void          browseTargetPath();

protected:
  /*$PROTECTED_FUNCTIONS$*/

protected slots:
  /*$PROTECTED_SLOTS$*/
  virtual void          reject();
  virtual void          accept();
private:
  QMakeFolderModel *m_model;
};

#endif

