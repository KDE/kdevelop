//
// C++ Implementation: 
//
// Description: 
//
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "projectconfigurationdlg.h"
#include "qmakeprojectmodel.h"
#include <kdebug.h>

#include <qbuttongroup.h>

ProjectConfigurationDlg::ProjectConfigurationDlg(QMakeFolderModel *model, QWidget* parent, const char* name, bool modal, WFlags fl)
: ProjectConfigurationDlgBase(parent,name, modal,fl), m_model(model)
{
    QStringList assns = model->assignmentNames();
    QStringList::ConstIterator it = assns.begin();
    for( ; it != assns.end(); ++it)
    {
        kdDebug(9024) << "Assign " << *it << endl;
        QString mode;
        QStringList settings = model->readAssignment( *it, mode );
        if( *it == "QT" )
        {

        }
        else if( *it == "CONFIG" )
        {

        }
        else if( *it == "TARGET" )
        {

        }
        else if( *it == "TEMPLATE" )
        {
            if( !settings.isEmpty() )
            {
                if( settings[0] == "app" )
                    groupTemplate->setButton(0);
                else if( settings[0] == "lib" )
                    groupTemplate->setButton(1);
                else if( settings[0] == "subdirs" )
                    groupTemplate->setButton(2);
            }
        }
    }
// Populate the GUI from the model 
// store a map of the assignment and the AST for lookup later 

}

ProjectConfigurationDlg::~ProjectConfigurationDlg()
{
}

/*$SPECIALIZATION$*/
void ProjectConfigurationDlg::editCustomValueClicked()
{
}

void ProjectConfigurationDlg::customVariables_selectionChanged()
{
}

void ProjectConfigurationDlg::newCustomVariableActive()
{
}

void ProjectConfigurationDlg::downCustomValueClicked()
{
}

void ProjectConfigurationDlg::upCustomValueClicked()
{
}

void ProjectConfigurationDlg::removeCustomValueClicked()
{
}

void ProjectConfigurationDlg::addCustomValueClicked()
{
}

void ProjectConfigurationDlg::extMoveDown_button_clicked()
{
}

void ProjectConfigurationDlg::extMoveUp_button_clicked()
{
}

void ProjectConfigurationDlg::extEdit_button_clicked()
{
}

void ProjectConfigurationDlg::extRemove_button_clicked()
{
}

void ProjectConfigurationDlg::extAdd_button_clicked()
{
}

void ProjectConfigurationDlg::intMoveDown_button_clicked()
{
}

void ProjectConfigurationDlg::intMoveUp_button_clicked()
{
}

void ProjectConfigurationDlg::slotInstallTargetClicked()
{
}

void ProjectConfigurationDlg::slotStaticLibClicked(int)
{
}

void ProjectConfigurationDlg::prjDepsMoveDownClicked()
{
}

void ProjectConfigurationDlg::prjDepsMoveUpClicked()
{
}

void ProjectConfigurationDlg::outsideIncEditClicked()
{
}

void ProjectConfigurationDlg::outsideLibEditClicked()
{
}

void ProjectConfigurationDlg::outsideLibDirEditClicked()
{
}

void ProjectConfigurationDlg::outsideLibDirRemoveClicked()
{
}

void ProjectConfigurationDlg::outsideLibDirAddClicked()
{
}

void ProjectConfigurationDlg::outsideLibDirMoveUpClicked()
{
}

void ProjectConfigurationDlg::outsideLibDirMoveDownClicked()
{
}

void ProjectConfigurationDlg::outsideLibRemoveClicked()
{
}

void ProjectConfigurationDlg::outsideLibAddClicked()
{
}

void ProjectConfigurationDlg::outsideIncRemoveClicked()
{
}

void ProjectConfigurationDlg::outsideIncAddClicked()
{
}

void ProjectConfigurationDlg::insideLibMoveDownClicked()
{
}

void ProjectConfigurationDlg::insideLibMoveUpClicked()
{
}

void ProjectConfigurationDlg::outsideLibMoveDownClicked()
{
}

void ProjectConfigurationDlg::outsideLibMoveUpClicked()
{
}

void ProjectConfigurationDlg::insideIncMoveDownClicked()
{
}

void ProjectConfigurationDlg::insideIncMoveUpClicked()
{
}

void ProjectConfigurationDlg::outsideIncMoveDownClicked()
{
}

void ProjectConfigurationDlg::outsideIncMoveUpClicked()
{
}

void ProjectConfigurationDlg::buildorderMoveDownClicked()
{
}

void ProjectConfigurationDlg::buildorderMoveUpClicked()
{
}

void ProjectConfigurationDlg::templateLibraryClicked(int)
{
}

void ProjectConfigurationDlg::updateProjectConfiguration()
{
    // Sync the GUI back to the model.
    // Template
    QStringList type;
    if( groupTemplate->selectedId() == 0 ) type += "app";
    else if( groupTemplate->selectedId() == 1 ) type += "lib";
    else if( groupTemplate->selectedId() == 2 ) type += "subdirs";
    m_model->writeScopeID("TEMPLATE", "=", type);
    ProjectConfigurationDlgBase::accept();
}

void ProjectConfigurationDlg::clickSubdirsTemplate()
{
}

void ProjectConfigurationDlg::browseTargetPath()
{
}

void ProjectConfigurationDlg::reject()
{
  QDialog::reject();
}

void ProjectConfigurationDlg::accept()
{

}

#include "projectconfigurationdlg.moc"

//kate: space-indent on; indent-width 4; mixedindent off;
