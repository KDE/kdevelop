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
#include <KMessageBox>
#include <ast.h>
#include <astutilities.h>
#include <language/duchain/functiondefinition.h>

namespace KDevelop
{

/**
 * @param objectToAllocate The declaration of the object to allocate memory for
 * @return the stringirized form of the user's preferred method of mamory allocation
 */
QString insertMemoryAllocation(const KDevelop::Declaration & objectToAllocate)
{
  return "new " + objectToAllocate.toString();
}

QString insertMemoryDeallocation(const KDevelop::Declaration& objectToDeallocate)
{
  return "delete " + objectToDeallocate.toString();
}

bool MakeImplementationPrivate::process()
{
    //If invoked through auto generation, then gatherPrivateMembers wan't called
    if(autoGeneration())
        gatherPrivateMembers();
    
    //Create container for private implementation
    CppNewClass classGenerator;
    classGenerator.setType(m_policies.testFlag(ContainerIsClass) ? CppNewClass::Class : CppNewClass::Struct);
    addDeclarationsToPrivate(classGenerator);
    classGenerator.identifier(m_structureName);
    
    IndexedString implementationFile = CodeGenUtils::fetchImplementationFileForClass(*m_classDeclaration);
    classGenerator.setHeaderUrl(implementationFile.str());
    //Set the best matching position before the first use
    //classGenerator.setHeaderPosition()
    DocumentChangeSet classChange = classGenerator.generateHeader();
    classChange.setReplacementPolicy(DocumentChangeSet::StopOnFailedChange);
    
    //Temporarily apply the new class into a separate temp file so we can get a chain for it, then merge it
    classChange.applyToTemp(implementationFile);
    KDevelop::ReferencedTopDUContext generatedClass = DUChain::self()->waitForUpdate(classChange.tempNameForFile(implementationFile), TopDUContext::AllDeclarationsContextsUsesAndAST);
    documentChangeSet() << classChange;
    
    //Create private implementation pointer member in the class
    DUChainReadLocker lock(DUChain::lock());
    SourceCodeInsertion pointerInsertion(m_classContext->topContext());
    pointerInsertion.setContext(m_classContext);
    pointerInsertion.setAccess(KDevelop::Declaration::Private);
    PointerType::Ptr pointer(new PointerType);
    pointer->setBaseType(AbstractType::Ptr::staticCast<StructureType>(classGenerator.objectType()));
    pointer->setModifiers(PointerType::ConstModifier);
    pointerInsertion.insertVariableDeclaration(Identifier(m_privatePointerName), AbstractType::Ptr::dynamicCast<PointerType>(pointer));
    
    //Temporarily apply the pointer insertion so that more changes can be made
    addChangeSet(pointerInsertion.changes());
    
    //Add private implementation struct forward declaration before the class
    Cpp::SourceCodeInsertion forwardDeclare(m_classContext->topContext());
    forwardDeclare.setInsertBefore(m_classDeclaration->range().start);
    kDebug() << "Looking for declaration of private class";
    QList<Declaration *> decls = generatedClass->findDeclarations(Identifier(classGenerator.identifier()));
    kDebug() << "Found: ";
    foreach(Declaration * decl, decls)
        kDebug() << decl->toString();
    
    if(!decls.empty())
    {
        forwardDeclare.insertForwardDeclaration(decls[0]);
        
        lock.unlock();
        updateConstructors(*decls[0]);
        updateDestructor();
    }
    addChangeSet(forwardDeclare.changes());
    
    //Gather all Uses of this class' members
    lock.lock();
    UseList allUses;
    foreach(ClassMemberDeclaration * declaration, m_members)
    {
        if(!declaration->type<FunctionType>())
            allUses[declaration] = declaration->uses();
    }
    
    updateAllUses(allUses);
    
    return true;
}

namespace
{
//TODO Find best place for this convenience function
bool hasDefaultConstructor(const Declaration * decl)
{
    DUContext * context = decl->internalContext();
    
    //take into account compiler generated default constructors
    bool constructorFound = false;
    
    foreach(Declaration * member, context->localDeclarations())
    {
        if(ClassFunctionDeclaration * classFun = dynamic_cast<ClassFunctionDeclaration *>(member))
        {
            TypePtr<FunctionType> funType = classFun->type<FunctionType>();
            
            //Check also for all default parameters, counts as default constructor
            if(classFun->isConstructor())
            {
                if(!constructorFound)
                    constructorFound = true;
                if(funType && classFun->defaultParametersSize() == funType->indexedArgumentsSize() &&
                   classFun->internalFunctionContext())
                    return true;
            }
        }
    }
  
    return !constructorFound;
}

}

bool MakeImplementationPrivate::gatherInformation()
{
    gatherPrivateMembers();
    
    Ui::PrivateImplementationDialog privateDialog;
    KDialog dialog(KDevelop::ICore::self()->uiController()->activeMainWindow());
    dialog.setButtons(KDialog::Ok | KDialog::Cancel);
    dialog.setWindowTitle(i18n("Private Class Implementation Options"));
    dialog.setInitialSize(QSize(400, 250));
    
    privateDialog.setupUi(dialog.mainWidget());
    
    CodeGenUtils::IdentifierValidator localValidator(m_classContext);
    CodeGenUtils::IdentifierValidator globalValidator(m_classContext->topContext());
    
    privateDialog.structureName->setValidator(&globalValidator);
    privateDialog.pointerName->setValidator(&localValidator);

    DUChainReadLocker lock(DUChain::lock());
    
    privateDialog.structureName->setText(m_classContext->scopeIdentifier(true).last().toString() + "Private");
    
    //If any of the members is either a reference or has non-default constructor then initialization
    //must bemoved to the private implementation constructor
    foreach(ClassMemberDeclaration * declaration, m_members)
    {
        AbstractType::Ptr type = declaration->abstractType();
        if(type->whichType() == AbstractType::TypeReference ||
           (type->whichType() == AbstractType::TypeStructure && !hasDefaultConstructor(StructureType::Ptr::dynamicCast<AbstractType>(type)->declaration(m_classContext->topContext())) ))
        {
            kDebug() << "Forcing private implementation member initialization, because of member: " << declaration->identifier();
            privateDialog.variableOption->setChecked(true);
            privateDialog.variableOption->setDisabled(true);
            break;
        }
    }
    
    int ret = dialog.exec();
    
    if(ret == QDialog::Accepted)
    { 
        //Save the names, and options set
        setPointerName(privateDialog.pointerName->text());
        setStructureName(privateDialog.structureName->text());
        
        m_policies |= (privateDialog.classOption->isChecked() ? ContainerIsClass : EmptyPolicy);
        m_policies |= (privateDialog.variableOption->isChecked() ? MoveInitializationToPrivate : EmptyPolicy);
        m_policies |= (privateDialog.methodOption->isChecked() ? MoveMethodsToPrivate : EmptyPolicy);
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
    
    DUChainReadLocker lock(DUChain::lock());
    m_classDeclaration = m_classContext->owner();
    
    return true;
}

void MakeImplementationPrivate::gatherPrivateMembers()
{
    DUChainReadLocker lock(DUChain::lock());
    foreach(Declaration * declaration, m_classContext->localDeclarations())
    {
        ClassMemberDeclaration * decl = dynamic_cast<ClassMemberDeclaration *>(declaration);
        Q_ASSERT(decl);
        if(decl->accessPolicy() == ClassMemberDeclaration::Private)
        {
            if(decl->type<FunctionType>() && !m_policies.testFlag(MoveMethodsToPrivate))
                continue;
            m_members << decl;
        }
    }
    
    kDebug() << "Gathered Privates:";
#ifndef NDEBUG
    foreach(ClassMemberDeclaration * decl, m_members)
      kDebug() << decl->toString();
#endif
}

void MakeImplementationPrivate::updateConstructors(const Declaration & privateStruct)
{
    //Gather constructors
    DUChainReadLocker lock(DUChain::lock());
    QList<Declaration *> constructors;
    Declaration * assignmentOp = 0;
    
    foreach(Declaration * declaration, m_classContext->localDeclarations())
    {
        ClassFunctionDeclaration * fun = dynamic_cast<ClassFunctionDeclaration *>(declaration);
        if(fun)
        {
            //Only gather constructors that have a definition
            if(fun->isConstructor())
            {
                Declaration * def = fun->logicalInternalContext(fun->topContext()) ? fun->logicalInternalContext(fun->topContext())->owner() : 0;
                if(def)
                    constructors << def;
#ifndef NDEBUG                
                else
                    kDebug() << "Definition not found for constructor: " << fun->toString();
#endif
            }
            //Gather the definition for the assignment operator
            else if(!assignmentOp)
            {
                QString signature = fun->toString();
                if(signature.contains("operator=") && fun->type<FunctionType>()->arguments().contains(m_classDeclaration->abstractType()))
                     assignmentOp = fun->logicalInternalContext(fun->topContext()) ? fun->logicalInternalContext(fun->topContext())->owner() : 0;
            }
        }
    }
    
    kDebug() << "Found the following constructors: " << constructors;
    
    if(m_policies.testFlag(MoveInitializationToPrivate))
    {
        if(constructors.size() > 1)
            KMessageBox::warningContinueCancel(0, i18n("Warning. It is not recommended to move initialization lists to private constructor when multiple constructors are defined."),
                                               "PIMPL Generation", KStandardGuiItem::cont(), KStandardGuiItem::cancel(), "PIMPL multiple constructor warning");
        foreach(Declaration * constructor, constructors)
        {
            CodeRepresentation::Ptr rangeRepresentation = representationFor(constructor->url());
            
            //Replace the previous constructor 
            QString privateVersion(constructor->toString());
            privateVersion.replace(m_classDeclaration->identifier().toString(), m_structureName);
            
            documentChangeSet().addChange(DocumentChange(constructor->url(), constructor->range(), constructor->toString(), privateVersion));
            
            // Create a "new" version of the previous constructor so that the private one can be called
            //ParseSession::Ptr astPtr = astContainer(constructor->internalFunctionContext()->url());
            //documentChangeSet().addChange(DocumentChange(constructor->internalFunctionContext()->url(), KTextEditor::Range(constructor->internalFunctionContext()->range().start, 0),
                                                        // QString(), constructor->toString() + "\n{\n}\n\n"));
        }
    }
    
    if(constructors.empty())
    {
        //TODO Create a default constructor
    }
    
    QList<IndexedString> filesToUpdate;
    
    lock.unlock();
    foreach(Declaration * constructor, constructors)
    {
        
        //Find the definition of the constructor
        /*lock.lock();
        FunctionDefinition * definition = FunctionDefinition::definition(constructor);
        lock.unlock();*/
        ParseSession::Ptr astPtr = astContainer(constructor->url());
        
        FunctionDefinitionAST * construct = AstUtils::node_cast<FunctionDefinitionAST>(astPtr->astNodeFromDeclaration(constructor));
        if(construct)
        {
            
            QString insertedText;
            
            if(m_policies.testFlag(MoveInitializationToPrivate))
            {
                //Send the parameters this constructor takes into the new one
            }
            CppEditorIntegrator integrator(astPtr.data());
            KTextEditor::Cursor insertionPoint;
            
            //Check for constructors without initializer list
            if(!construct->constructor_initializers)
            {
                insertedText += ":";
                insertionPoint = integrator.findPosition(construct->function_body->start_token);
                if(insertionPoint.column > 0)
                    insertionPoint.column = insertionPoint.column - 1;
            }
            else
              insertionPoint = integrator.findPosition(construct->constructor_initializers->colon);
            insertedText += " " + m_privatePointerName + "(" + insertMemoryAllocation(privateStruct) + ") ";
            
            DocumentChange constructorChange(constructor->url(), KTextEditor::Range(insertionPoint, 0), QString(), insertedText);
            documentChangeSet().addChange(constructorChange);
            
            //Remove the old initializers
            if(construct->constructor_initializers && construct->constructor_initializers->member_initializers->count())
            {
                KTextEditor::Range oldInitializers (integrator.findRange(construct->constructor_initializers->member_initializers->toFront()->element->start_token,
                                                                  construct->constructor_initializers->member_initializers->toBack()->element->end_token));
                DocumentChange initializersChange(constructor->url(), oldInitializers, QString(), QString());
                initializersChange.m_ignoreOldText = true;
                documentChangeSet().addChange(initializersChange);
            }
            if(documentChangeSet().applyToTemp(constructor->url()) && !filesToUpdate.contains(constructor->url()))
                filesToUpdate << constructor->url();
        }
        else
            kWarning() << "A correct AST node for constructor: " << constructor->toString() << " was not found.";
    }
    
    //TODO Handle assignment operator here as well, and check selection logic
    foreach(const IndexedString & update, filesToUpdate)
        DUChain::self()->waitForUpdate(update, static_cast<TopDUContext::Features>(TopDUContext::ForceUpdate | TopDUContext::AllDeclarationsContextsUsesAndAST));
}

void MakeImplementationPrivate::updateDestructor()
{
    //Find destructor if available
    ClassFunctionDeclaration * destructor = 0;
    DUChainReadLocker lock(DUChain::lock());
    
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
        SourceCodeInsertion insertion(m_classContext->topContext());
        insertion.setAccess(KDevelop::Declaration::Public);
        insertion.setContext(m_classContext);
        insertion.setInsertBefore(m_classContext->range().end);
        QString signature("~");
        signature.append(m_classDeclaration->identifier().toString());
        
        ///@todo Allow creation of Destructor body in implementation file
        ///@todo allow for custom memory deallocation set up by the user
        QString body = "{\ndelete " + m_privatePointerName + ";\n};";
        
        bool result = insertion.insertFunctionDeclaration(Identifier(signature), AbstractType::Ptr(),
                                                          QList<SourceCodeInsertion::SignatureItem>(), false, body);
        Q_ASSERT(result);
        documentChangeSet() << insertion.changes();
    } 
    else
    {
        DUContext * internal = destructor->logicalInternalContext(destructor->topContext());
        KTextEditor::Cursor inside(internal->range().end);
        if(inside.column > 0)
            inside.column = inside.column - 1;
        DocumentChange destructorChange(internal->url(), KTextEditor::Range(inside, 0),
                                        QString(), "delete this->" + m_privatePointerName + ";\n");
    
        documentChangeSet().addChange(destructorChange);
    }
}

void MakeImplementationPrivate::updateAllUses(UseList & allUses)
{
    //For all uses gathered from all members change to access through pointer
    for(UseList::iterator it = allUses.begin();
        it != allUses.end(); ++it)
    {
        //! @todo check properly if the pointer is being hidden, and add this-> only if necessary
        QString accessString = it.key()->kind() == Declaration::Instance ? m_privatePointerName + "->" : m_structureName + "::";
        
        for(QMap<IndexedString, QList<KTextEditor::Range> >::iterator mapIt = it->begin();
            mapIt != it->end(); ++mapIt)
        {
            kDebug() << "In file: " << mapIt.key().str();
            //If there is a temporary of this file, then ignore this file, and update the temporary uses
            if(documentChangeSet().tempNameForFile(mapIt.key()) == mapIt.key())
                foreach(KTextEditor::Range range, *mapIt)
                {
                    CodeRepresentation::Ptr rangeRepresentation = representationFor(mapIt.key());
                    QString use = rangeRepresentation->rangeText(range);
                    kDebug() << "Found use: " << use << "at: " << range;
                    DocumentChange useChange(mapIt.key(), range, use, accessString + use);
                    
                    Q_ASSERT(documentChangeSet().addChange(useChange));
                }
        }
    }
}

CodeRepresentation::Ptr MakeImplementationPrivate::representationFor(IndexedString url)
{
    if(!m_representations.contains(url))
        m_representations[url] = createCodeRepresentation(url);
    
    return m_representations[url];
}

void MakeImplementationPrivate::addDeclarationsToPrivate(CppNewClass & classGenerator)
{
    ParseSession::Ptr ast = astContainer(m_classContext->url());
    CppEditorIntegrator integrator(ast.data());
    
    foreach(Declaration * decl, m_members)
    {
        classGenerator.addDeclaration(DeclarationPointer(decl));
        //Get the context that properly encapsulates the declaration through the AST
        AST * node = ast->astNodeFromDeclaration(decl);
        if(node)
        {
            KTextEditor::Range declarationRange = integrator.findRange(node).castToSimpleRange();
            kDebug() << "Found AST node for declaration: " << decl->toString() << ". With range: " << declarationRange;
            
            DocumentChange removeDeclarations(decl->url(), declarationRange, decl->toString(), QString());
            //Verifying the old text might cause conflicts with variables defined after structure declarations
            removeDeclarations.m_ignoreOldText = true;
            documentChangeSet().addChange(removeDeclarations);
        }
        else
            kWarning() << "Did not find an AST node mapped for declarationn: " << decl->toString();
    }
    
    //Remove all the declarations now, so they don't interfere later
    documentChangeSet().applyToTemp(m_classDeclaration->url());
}

}
