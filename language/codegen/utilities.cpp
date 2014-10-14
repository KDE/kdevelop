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
#include "util/debug.h"

#include <serialization/indexedstring.h>
#include <duchain/identifier.h>
#include <duchain/duchainlock.h>
#include <duchain/duchain.h>
#include <duchain/declaration.h>
#include <duchain/forwarddeclaration.h>
#include <duchain/functiondefinition.h>
#include <duchain/classfunctiondeclaration.h>
#include <duchain/types/functiontype.h>

namespace KDevelop
{

namespace CodeGenUtils
{

IdentifierValidator::IdentifierValidator( DUContext * context) : QValidator(0), m_context(context)
{
}

IdentifierValidator::~IdentifierValidator()
{
}

QValidator::State IdentifierValidator::validate (QString & input, int &) const
{
    //I can't figure out why it wouln't compile when I tried to use Identifier identifier();
    Identifier identifier = Identifier(IndexedString(input));

    if(identifier.isUnique())
        return Acceptable;

    DUChainReadLocker lock(DUChain::lock(), 10);
    return m_context->findLocalDeclarations(identifier, CursorInRevision::invalid(), 0, AbstractType::Ptr(), DUContext::NoFiltering).empty() ? Acceptable : Invalid;
}

IndexedString fetchImplementationFileForClass(const Declaration & targetClass)
{
    DUChainReadLocker lock(DUChain::lock());
    qCDebug(LANGUAGE) << "Looking for implementation file for class:" << targetClass.identifier().toString();

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

    QMap<IndexedString, unsigned int > implementationsInFile;

    foreach(Declaration * decl, declarations)
    {
        ///@todo check for static variable instantiation as well
        if(ClassFunctionDeclaration * classFun = dynamic_cast<ClassFunctionDeclaration *>(decl))
            if(FunctionDefinition * def = FunctionDefinition::definition(classFun))
            {
                qCDebug(LANGUAGE) << "Definition For declaration in:" << def->url().toUrl();
                ++implementationsInFile[def->url()];
            }
    }

    QMultiMap<unsigned int, IndexedString> sorter;
    foreach(const IndexedString& file, implementationsInFile.keys())
        sorter.insert(implementationsInFile[file], file);

    QList<IndexedString> sortedFiles = sorter.values();

    //If there are no methods, then just return the file the declaration is in
    if(sortedFiles.empty())
        return context->url();

    if(sortedFiles.size() == 1)
        return sortedFiles[0];

    if(sorter.values(sorter.end().key()).size() > 1)
    {
        //Return the file that has the most uses
        QList<IndexedString> tiedFiles = sorter.values(sorter.end().key());
        QMap<IndexedString, QList<RangeInRevision> > uses = realClass->uses();

        IndexedString mostUsesFile;
        unsigned int mostUses = 0;
        foreach(const IndexedString& currentFile, tiedFiles)
            if(static_cast<unsigned int>(uses[currentFile].size()) > mostUses)
            {
                mostUses = uses[currentFile].size();
                mostUsesFile = currentFile;
            }

        return mostUsesFile;
    }
    else
        return sortedFiles.back();

}

}


}
