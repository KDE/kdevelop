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

#ifndef MAKEIMPLEMENTATIONPRIVATE_H
#define MAKEIMPLEMENTATIONPRIVATE_H

#include <language/codegen/codegenerator.h>
#include <language/codegen/coderepresentation.h>
#include <parsesession.h>

#include <bitset>

class CppNewClass;
namespace KDevelop
{
class DocumentChange;

class ClassMemberDeclaration;
/*!
 * @brief   Applies the PIMPL pattern to a class, hiding all it's private members in a private class
 * @todo    Add support for non-default constructors
 */
class MakeImplementationPrivate : public CodeGenerator<ParseSession>
{
    typedef QMap<ClassMemberDeclaration *, QMap<IndexedString, QList<KTextEditor::Range> > > UseList;
    
  public:
    
    ///Add policy for declaring in unnamed namespace
    enum Policy
    {
        EmptyPolicy = 0x0,
        ContainerIsClass = 0x1,             //Indicates the container type will be class, otherwise struct will be used
        MoveInitializationToPrivate = 0x2,  //Moves the initialization of variables to the initialization list of private implementation
        MoveMethodsToPrivate = 0x4         //Moves the private methods into the private implementation
    };
    Q_DECLARE_FLAGS(Policies, Policy)
    
    MakeImplementationPrivate() : m_classContext(0) {}
    ~MakeImplementationPrivate() {}
    // Implementations from CodeGenerator
    virtual bool process();
    virtual bool gatherInformation();
    virtual bool checkPreconditions(DUContext* context, const DocumentRange& position);
    
    //Options for auto generation
    void setStructureName(const QString & name) { m_structureName = name; };
    void setPointerName(const QString & name)   { m_privatePointerName = name; };
    void setPolicies(Policies newPolicy)        { m_policies = newPolicy; };
    
  
  private:
    DUContext * m_classContext;
    DeclarationPointer m_classDeclaration;
    
    QString m_privatePointerName;
    QString m_structureName;
    
    QList<ClassMemberDeclaration *> m_members;
    QHash<IndexedString, CodeRepresentation::Ptr> m_representations;
    
    Policies m_policies;
    
    bool classHasPrivateImplementation();
    void gatherPrivateMembers();
    void updateConstructors(const KDevelop::Declaration &);
    void updateDestructor();
    void updateAllUses(UseList & alluses);
    CodeRepresentation::Ptr representationFor(IndexedString url);
    void addDeclarationsToPrivate(CppNewClass &);
    //DocumentChange insertConstructorInitializations(ClassFunctionDeclaration * constructor, const QList<ClassMemberDeclaration *>)
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(KDevelop::MakeImplementationPrivate::Policies)

#endif // MAKEIMPLEMENTATIONPRIVATE_H
