/*
   Copyright 2009 Ramón Zarazúa <killerfox512+kde@gmail.com>

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

#include "makeimplementationprivate.h"
#include "ui_privateimplementation.h"

#include <language/duchain/ducontext.h>
#include <language/codegen/utilities.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>

#include <kinputdialog.h>
#include <kparts/mainwindow.h>

namespace KDevelop
{

bool MakeImplementationPrivate::process()
{
    //Add private implementation struct forward declaration before the class
    
    //Create private implementation pointer member in the class
    
    //Initialize private implementation pointer in constructor
    
    //Destroy private implementation pointer in destructor
    
    //Create container for private implementation
    
    //Gather private member declarations 
    //Move member declarations to private implementation
    
    //Gather all Uses of this class' members
    
    //For all uses gathered change to access through pointer
    
    //Verify that implementation is private
    return true;
}

bool MakeImplementationPrivate::gatherInformation()
{
    Ui::PrivateImplementationDialog privateDialog;
    KDialog dialog(KDevelop::ICore::self()->uiController()->activeMainWindow());
    
    privateDialog.setupUi(&dialog);
    
    CodeGenUtils::IdentifierValidator localValidator(m_classContext);
    CodeGenUtils::IdentifierValidator globalValidator(m_classContext->topContext());
    
    privateDialog.structureName->setValidator(&globalValidator);
    privateDialog.pointerName->setValidator(&localValidator);
    
    privateDialog.structureName->setText(m_classContext->scopeIdentifier().toString() + "Private");
    
    int ret = dialog.exec();
    
    if(ret == QDialog::Accepted)
    { 
        //Save the names, and options set
        m_privatePointerName = privateDialog.pointerName->text();
        m_structureName = privateDialog.structureName->text();
        
        m_policies[ContainerIsClass] = privateDialog.classOption->isChecked();
        m_policies[MoveInitializationToPrivate] = privateDialog.variableOption->isChecked();
        m_policies[MoveMethodsToPrivate] = privateDialog.methodOption->isChecked();
        return true;
    }
    else
    {
        setErrorText("User Abort");
        return false;
    }
}

bool MakeImplementationPrivate::checkPreconditions(KDevelop::DUContext * context, const KDevelop::DocumentRange &)
{
    m_classContext = context;
    //TODO check that it doesn't already have a private implementation
    if(m_classContext->type() != DUContext::Class)
    {
        setErrorText("Selected Context does not belong to a Class");
        return false;
    }
    else
        return true;
}

}
