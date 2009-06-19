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

#include "utilities.h"

#include <duchain/indexedstring.h>
#include <duchain/identifier.h>
#include <duchain/duchainlock.h>
#include <duchain/duchain.h>
#include <duchain/declaration.h>
#include <duchain/forwarddeclaration.h>
#include <duchain/classfunctiondeclaration.h>
#include <KIO/NetAccess>
#include <duchain/types/functiontype.h>

namespace KDevelop
{

namespace CodeGenUtils
{
    
IdentifierValidator::IdentifierValidator( DUContext * context) : QValidator(0), m_context(context)
{
}

IdentifierValidator::~IdentifierValidator(void)
{
}

QValidator::State IdentifierValidator::validate (QString & input, int &) const
{
    //I can't figure out why it wouln't compile when I tried to use Identifier identifier();
    Identifier identifier = Identifier(IndexedString(input));
    
    if(identifier.isUnique())
        return Acceptable;
    
    DUChainReadLocker lock(DUChain::lock(), 10);
    return m_context->allLocalDeclarations(identifier).empty() ? Acceptable : Invalid;
}

IndexedString fetchImplementationFileForClass(const Declaration & targetClass, SearchPriority priority, const QString & extension)
{
    kDebug() << "Looking for implementation file for class:" << targetClass.identifier().toString();
    
    DUContext * context = targetClass.internalContext();
    
    //If this declaration is not a user defined type, then ignore and return empty file
    if(targetClass.kind() != Declaration::Type)
        return IndexedString();
    
    //If this is a forward declaration attempt to resolve it.
    const Declaration * realClass = &targetClass;
    if(const ForwardDeclaration * forward = dynamic_cast<const ForwardDeclaration *>(realClass))
    {
        if(!(realClass = forward->resolve(context->topContext())))
            return IndexedString();
        context = realClass->internalContext();
    }
    
    QVector<Declaration *> declarations = context->localDeclarations();
    
    for(int p = priority; p <= ClassNameExtension; ++p)
        //Go through each priority until one isfound
        switch(priority)
        {
        default:
        case DeclarationOnly:
            return realClass->url();
            break;
            
        case DefaultConstructor:
            {
                foreach(Declaration * decl, declarations)
                    //Examine all function declarations for constructors
                    if(ClassFunctionDeclaration * classFun = dynamic_cast<ClassFunctionDeclaration *>(decl))
                    {
                        TypePtr<FunctionType> funType = classFun->type<FunctionType>();
                        
                        if(classFun->isConstructor() && funType &&
                        classFun->defaultParametersSize() == funType->indexedArgumentsSize() &&
                        classFun->internalFunctionContext())
                            return classFun->internalFunctionContext()->url();
                    }
            }
            break;
            
        case CustomConstructor:
            {
                //Similar code to Default constructor, but we just look for any constructor
                foreach(Declaration * decl, declarations)
                    //Examine all function declarations for constructors
                    if(ClassFunctionDeclaration * classFun = dynamic_cast<ClassFunctionDeclaration *>(decl))
                        if(classFun->isConstructor() &&
                        classFun->internalFunctionContext())
                            return classFun->internalFunctionContext()->url();
            }
            break;
            
        case Destructor:
            {
                foreach(Declaration * decl, declarations)
                    if(ClassFunctionDeclaration * classFun = dynamic_cast<ClassFunctionDeclaration *>(decl))
                        if(classFun->isDestructor() && classFun->internalFunctionContext())
                            return classFun->internalFunctionContext()->url();
            }
            break;
            
        case ClassNameExtension:
            {
                //Try and find a file with the exact same name of the class + extension
                KUrl url = realClass->url().toUrl().directory();
                QString fileName = realClass->toString() + extension;
                
                url.addPath(fileName);
                KIO::UDSEntry u;
                if(KIO::NetAccess::stat(url, u, 0))
                    return IndexedString(url);
                //If not found, try converting file name to all lower case
                else
                {
                    url = url.directory();
                    url.addPath(fileName.toLower());
                    if(KIO::NetAccess::stat(url, u, 0))
                        return IndexedString(url);
                }
            }
            break;
        }
    return IndexedString();
}

}


}
