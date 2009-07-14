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

#include "cpputils.h"

#include "cppduchain/sourcemanipulation.h"
#include "cppnewclass.h"
#include <cppeditorintegrator.h>

#include <language/duchain/ducontext.h>
#include <language/duchain/declarationdata.h>
#include <language/duchain/classmemberdeclaration.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/structuretype.h>
#include <language/codegen/utilities.h>
#include <language/codegen/documentchangeset.h>
#include <language/codegen/coderepresentation.h>


#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>

#include <kinputdialog.h>
#include <kparts/mainwindow.h>
#include <ktexteditor/smartrange.h>
#include <KMessageBox>
#include <ast.h>
#include <astutilities.h>

namespace KDevelop
{

bool MakeImplementationPrivate::process()
{
    updateDestructor();
    
    //Create container for private implementation
    CppNewClass classGenerator;
    classGenerator.setType(m_policies[ContainerIsClass] ? CppNewClass::Class : CppNewClass::Struct);
    foreach(Declaration * decl, m_members)
    {
        classGenerator.addDeclaration(DeclarationPointer(decl));
        DocumentChange removeDeclarations(decl->url(), decl->range(), decl->toString(), QString());
        documentChangeSet().addChange(removeDeclarations);
    }
    classGenerator.identifier(m_structureName);
    //classGenerator.generateHeader();
    
    //Create private implementation pointer member in the class
    SourceCodeInsertion pointerInsertion(m_classContext->topContext());
    pointerInsertion.setContext(m_classContext);
    pointerInsertion.setAccess(KDevelop::Declaration::Private);
    pointerInsertion.insertVariableDeclaration(Identifier(m_privatePointerName), AbstractType::Ptr::staticCast<StructureType>(classGenerator.objectType()));
    addChangeSet(pointerInsertion.changes());
    
    //Add private implementation struct forward declaration before the class
    Cpp::SourceCodeInsertion forwardDeclare(m_classContext->topContext());
    forwardDeclare.setInsertBefore(m_classDeclaration->range().start);
    /*{
        DUChainReadLocker lock(DUChain::self()->lock());
        DeclarationData decl;
        decl.m_identifier = IndexedIdentifier(Identifier(classGenerator.identifier()));
        decl.m_kind = Declaration::Type;
        decl.m_type = classGenerator.objectType()->indexed();
        decl.m_isTypeAlias = false;
        Declaration d (decl);
        //TODO declaration needs context forwardDeclare.insertForwardDeclaration(&d);
         
        updateConstructors(decl);
    }
    addChangeSet(forwardDeclare.changes());*/
    
    //Gather all Uses of this class' members
    UseList allUses;
    foreach(ClassMemberDeclaration * declaration, m_members)
    {
        if(!declaration->type<FunctionType>())
            allUses << declaration->uses();
    }
    
    updateAllUses(allUses);
    
    return true;
}

bool MakeImplementationPrivate::gatherInformation()
{
    gatherPrivateMembers();
    
    Ui::PrivateImplementationDialog privateDialog;
    KDialog dialog(KDevelop::ICore::self()->uiController()->activeMainWindow());
    
    privateDialog.setupUi(&dialog);
    
    CodeGenUtils::IdentifierValidator localValidator(m_classContext);
    CodeGenUtils::IdentifierValidator globalValidator(m_classContext->topContext());
    
    privateDialog.structureName->setValidator(&globalValidator);
    privateDialog.pointerName->setValidator(&localValidator);
    
    privateDialog.structureName->setText(m_classContext->scopeIdentifier(true).last().toString() + "Private");
    
    //If any of the members is either a reference or has non-default constructor then initialization
    //must bemoved to the private implementation constructor
//     foreach(ClassMemberDeclaration * declaration, m_members)
//     {
//         AbstractType::Ptr type = declaration->abstractType();
//         if(AbstractType::Ptr::dynamicCast<ReferenceType>(type) ||
//            (type->whichType() == AbstractType::TypeStructure ))//TODO && !Hav4e default constructor TODO where is the best place to put returning of all constructors ) )
//         {
//             privateDialog.variableOption->setChecked();
//             privateDialog.variableOption->setDisabled();
//             break;
//         }
//     }
    
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
    if(!context)
    {
        setErrorText("Could not get the context for text selection");
        return false;
    }
    m_classContext = context;
    //TODO check that it doesn't already have a private implementation
    
    while(m_classContext && m_classContext->type() != DUContext::Class)
        m_classContext = m_classContext->parentContext();
    
    if(!m_classContext)
    {
        setErrorText("Selected Context does not belong to a Class");
        return false;
    }
    m_classDeclaration = m_classContext->owner();
    
    return true;
}

void MakeImplementationPrivate::gatherPrivateMembers()
{
    foreach(Declaration * declaration, m_classContext->localDeclarations())
    {
        ClassMemberDeclaration * decl = dynamic_cast<ClassMemberDeclaration *>(declaration);
        Q_ASSERT(decl);
        if(decl->accessPolicy() == ClassMemberDeclaration::Private)
        {
            if(decl->type<FunctionType>() && !m_policies[MoveMethodsToPrivate] )
                continue;
            m_members << decl;
        }
    }
}

void MakeImplementationPrivate::updateConstructors(const Declaration & privateStruct)
{
    //Gather constructors
    QList<ClassFunctionDeclaration *> constructors;
    ClassFunctionDeclaration * assignmentOp = 0;
    
    foreach(Declaration * declaration, m_classContext->localDeclarations())
    {
        ClassFunctionDeclaration * fun = dynamic_cast<ClassFunctionDeclaration *>(declaration);
        if(fun)
        {
            if(fun->isConstructor())
                constructors << fun;
            else if(!assignmentOp)
            {
                QString signature = fun->toString();
                if(signature.contains("operator=") && fun->type<FunctionType>()->arguments().contains(m_classDeclaration->abstractType()))
                    assignmentOp = fun;
            }
        }
    }
    
    kDebug() << "Found the following constructors: " << constructors;
    
    if(m_policies[MoveInitializationToPrivate])
    {
        if(constructors.size() > 1)
            KMessageBox::warningContinueCancel(0, "Warning. It is not recommended to move initialization lists to private constructor when multiple constructors are defined.",
                                               "PIMPL Generation", KStandardGuiItem::cont(), KStandardGuiItem::cancel(), "PIMPL multiple constructor warning");
        //foreach(ClassFunctionDecla)
        //{
        //}//TODO
    }
    
    if(constructors.empty())
    {
        //TODO Create a default constructor
    }
    
    foreach(ClassFunctionDeclaration * constructor, constructors)
    {
        ParseSession::Ptr astPtr = astContainer(constructor->internalFunctionContext()->url());
        
        FunctionDefinitionAST * construct = AstUtils::node_cast<FunctionDefinitionAST>(astPtr->astNodeFromDeclaration(constructor));
        Q_ASSERT(construct);
        
        //Check for constructors without initializer list
        
        CppEditorIntegrator integrator(astPtr.data());
        SimpleCursor insertionPoint = integrator.findPosition(construct->constructor_initializers->colon);
        
        DocumentChange constructorChange(constructor->url(), SimpleRange(insertionPoint, 0), QString(), m_privatePointerName + "(" + CppUtils::insertMemoryAllocation(privateStruct) + ") ");
        documentChangeSet().addChange(constructorChange);
    }
    
    //TODO Handle assignment operator here as well, and check selection logic
}

void MakeImplementationPrivate::updateDestructor(void)
{
    //Find destructor if available
    ClassFunctionDeclaration * destructor = 0;
    foreach(Declaration * declaration, m_classContext->localDeclarations())
    {
        ClassFunctionDeclaration * fun = dynamic_cast<ClassFunctionDeclaration *>(declaration);
        if(fun && fun->isDestructor())
        {
            destructor = fun;
            break;
        }
    }
   
    if(!destructor)
    {
        //Create a Destructor
    } 
    
    DocumentChange destructorChange(destructor->url(), SimpleRange(destructor->logicalInternalContext(destructor->topContext())->range().start, -1),
                                    QString(), /*CppUtils::insertMemoryDeallocation()*/ "delete this->" + m_privatePointerName + ";\n");
    
    documentChangeSet().addChange(destructorChange);
}

void MakeImplementationPrivate::updateAllUses(UseList & allUses)
{
    //For all uses gathered from all members change to access through pointer
    for(UseList::iterator it = allUses.begin();
        it != allUses.end(); ++it)
    {
        for(QMap<IndexedString, QList<SimpleRange> >::iterator mapIt = it->begin();
            mapIt != it->end(); ++mapIt)
        {
            foreach(SimpleRange range, *mapIt)
            {
                CodeRepresentation * rangeRepresentation = createCodeRepresentation(mapIt.key());
                QString use = dynamic_cast<DynamicCodeRepresentation *>(rangeRepresentation)->rangeText(range.textRange());
                //! @todo check properly if the pointer is being hidden, and add this-> only if necessary
                DocumentChange useChange(mapIt.key(), range, use, "this->" + m_privatePointerName + "->" + use);
                
                Q_ASSERT(documentChangeSet().addChange(useChange));
                
                delete rangeRepresentation;
            }
        }
    }
}


}
