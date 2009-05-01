/***************************************************************************
 *   Copyright 2008 Aleix Pol <aleixpol@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "duchainreader.h"
#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/delayedtype.h>
#include <language/duchain/types/enumerationtype.h>
#include <language/duchain/types/enumeratortype.h>
#include <language/duchain/abstractfunctiondeclaration.h>
#include <language/duchain/classmemberdeclaration.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/classdeclaration.h>
#include <KDE/KDebug>
#include <QtCore/QQueue>

using namespace KDevelop;

QList<const Declaration*> extractTypes(const DUContext* ctx, int ind=1)
{
    QList<const Declaration*> ret;
    qDebug() << qPrintable(QString(ind*3, '-')) << "ctx" << ctx << ctx->childContexts();
    foreach(const Declaration* decl, ctx->localDeclarations())
    {
//         qDebug() << qPrintable(QString(ind*3, '-')) << "decl" << decl->toString()
//                  << (decl->kind()==Declaration::Type);
        
        if(dynamic_cast<const ClassDeclaration*>(decl) && !decl->identifier().toString().isEmpty())
        {
            ret += decl;
        }
    }
    qDebug() << qPrintable(QString(ind*3, '+'));
    foreach(const DUContext* ct, ctx->childContexts())
        ret += extractTypes(ct, ind+1);
    return ret;
}

QString DUChainReader::printType(const TypePtr<AbstractType>& type)
{
    QString ret;
    if(type.cast<DelayedType>())
    {
        TypePtr<DelayedType> del=type.cast<DelayedType>();
        ret=del->identifier().toString();
    }
    else
        ret=type->toString();
//     qDebug() << "yyyyyyyyyy" << type->toString() << ret;
    return ret;
}


bool isTemplateDeclaration(const KDevelop::Declaration* decl)
{
    KDevelop::DUContext* current = decl->internalContext();
    while(current)
    {
        if(current->type() == KDevelop::DUContext::Template)
            return true;
        if(current->importedParentContexts().isEmpty())
            return false;
        current = current->importedParentContexts()[0].context(decl->topContext());
    }
    return false;
}

void DUChainReader::foundClass(const Declaration* decl)
{
    QString baseClass;
    const ClassDeclaration *cdecl=dynamic_cast<const ClassDeclaration*>(decl);
//     qDebug() << "lalalalalalalala" << cdecl->baseClassesSize()
//              << cdecl->baseClasses()[0].access << cdecl->baseClasses()[0].baseClass.type()->toString()
//              << definedClasses;
    if(cdecl->baseClassesSize()>=1 &&
       cdecl->baseClasses()[0].access==KDevelop::Declaration::Public &&
       definedClasses.contains(cdecl->baseClasses()[0].baseClass.abstractType()->toString()))
        baseClass=cdecl->baseClasses()[0].baseClass.abstractType()->toString();
    
    QString inClass;
    bool isInClass=decl->context()->type() == DUContext::Class;
    
    inNamespace=decl->context()->scopeIdentifier(true).toString();
    if(isInClass) { //Only 1 level
        int lastColons = inNamespace.lastIndexOf("::"), nssize=inNamespace.size();
        inClass+=inNamespace.right(nssize-lastColons-2)+"::";
        inNamespace.resize(lastColons);
    }
    
    QList<QStringList> enums;
    foreach(const Declaration* declEnum, decl->internalContext()->localDeclarations())
    {
        if(declEnum->kind()==Declaration::Type && declEnum->internalContext())
        {
            const AbstractType::Ptr t = declEnum->abstractType();
            const EnumerationType* en = dynamic_cast<const EnumerationType*>(t.unsafeData());
        
            if(en)
            {
                QStringList anEnum;
                if(!declEnum->identifier().identifier().isEmpty()) {
                    anEnum+=en->qualifiedIdentifier().toString();
                    
                    foreach(const Declaration* declFlag, declEnum->internalContext()->localDeclarations())
                    {
                        const AbstractType::Ptr t = declFlag->abstractType();
                        const EnumeratorType* enor = dynamic_cast<const EnumeratorType*>(t.unsafeData());
        //                 Q_ASSERT(enor);
                        if(!enor) {
                            qDebug() << "found a null enumerator";
                            continue;
                        }
                        
                        anEnum += enor->qualifiedIdentifier().toString();
                    }
                    enums += anEnum;
                }
            }
        }
    }
    
    sonsPerClass[baseClass].append(cdecl->abstractType()->toString());
    definedClasses.append(cdecl->abstractType()->toString());
    writeClass(cdecl->abstractType()->toString(), baseClass, enums);
    
    //Looking for methods
    DUContext* ctx=decl->internalContext();
    QSet<QString> forbidden;
    forbidden.insert("metaObject");
    forbidden.insert("tr");
    forbidden.insert("trUtf8");
    forbidden.insert("operator{...cast...}");
    forbidden.insert("operator<<");
    foreach(const Declaration* func, ctx->localDeclarations())
    {
        const ClassMemberDeclaration* memberDecl=dynamic_cast<const ClassMemberDeclaration*>(func);
//         if(memberDecl) qDebug() << "bibibibibibi" << memberDecl->toString()
//                                 << memberDecl << memberDecl->accessPolicy() << (memberDecl->accessPolicy()==Declaration::Public);
        if(!memberDecl || memberDecl->accessPolicy()!=Declaration::Public)
            continue;
//         qDebug() << "++++++++++" << func->toString();
        if(func->isFunctionDeclaration())
        {
            QualifiedIdentifier qid=func->qualifiedIdentifier();
            const AbstractType::Ptr atype=func->abstractType();
            
            const FunctionType::Ptr ftype=atype.cast<FunctionType>();
            const ClassFunctionDeclaration* cdec=dynamic_cast<const ClassFunctionDeclaration*>(func);
            const AbstractFunctionDeclaration* dec=dynamic_cast<const AbstractFunctionDeclaration*>(func);
            Q_ASSERT(dec && cdec && func && memberDecl);
            
            if(memberDecl->accessPolicy()!=Declaration::Public)
               continue;
            
            QString funcname=func->identifier().toString();
            
            if(funcname.startsWith("qt_") || funcname.startsWith('~') || forbidden.contains(funcname))
                continue;
            
            bool isConstructor=ftype->returnType().isNull();
            
            QString rettype= isConstructor ? QString() : printType(ftype->returnType());
            if(isTemplateDeclaration(func)) //we disable templated functions
                continue;
            
            bool isConst=ftype->modifiers() & AbstractType::ConstModifier;
            bool isVirtual=dec->isVirtual();
            bool isAbstract=cdec->isAbstract();
            
            const IndexedString* idx=dec->defaultParameters();
            int notDefCount = ftype->arguments().count()-dec->defaultParametersSize();
            int i=0;
            qDebug() << "foundfunc" << func->internalContext()->localDeclarations().size() << funcname
                     << "isConst " << isConst << "isVirtual" << isVirtual;
            
            method currentMethod;
            currentMethod.funcname=funcname;
            currentMethod.returnType=rettype;
            currentMethod.isConst=isConst;
            currentMethod.isVirtual=isVirtual;
            currentMethod.isAbstract=isAbstract;
            currentMethod.isConstructor=isConstructor;
            foreach(const TypePtr<AbstractType>& argtype, ftype->arguments())
            {   
                notDefCount--;
                method::argument arg;
                arg.type=printType(argtype);
                arg.name=QString("x%1").arg(i++);
//                 qDebug() << "arg" << notDefCount << arg.name;
                if(notDefCount<0)
                {
                    arg.def=idx->str();
                    QString scopeClass=decl->internalContext()->scopeIdentifier(true).toString();
                    QString scope=decl->internalContext()->scopeIdentifier().toString();
                    qDebug () << "default value" << arg.type << arg.def
                              << scope
                              << (!arg.def.startsWith(scope))
                              << (arg.type.startsWith(scope))
                              << dynamic_cast<const EnumerationType*>(argtype.unsafeData());
                    
                    int end = arg.def.indexOf('(');
                    if(end == -1)
                        end = arg.def.length();
                    
                    QString constr=arg.def.left(end);
                    
                    QList<Declaration*> decls = ctx->findDeclarations(QualifiedIdentifier(constr));
                    if(!decls.isEmpty())
                        arg.def = decls.first()->qualifiedIdentifier().toString() + arg.def.mid(end);
                    
//                     if(dynamic_cast<const CppEnumerationType*>(argtype.unsafeData()) && !scopeClass.isEmpty() &&
//                             !arg.def.startsWith(scopeClass) && arg.type.startsWith(scopeClass))
//                         arg.def.prepend(scopeClass+"::");
//                     else if(arg.type.endsWith("::"+constr+'&') || arg.type.endsWith("::"+constr))
//                         arg.def.prepend(scope+"::");
                    idx++;
                }
                currentMethod.args += arg;
            }
            
            writeEndFunction(currentMethod);
        }
        else if(dynamic_cast<const ClassMemberDeclaration*>(func) && func->kind() == Declaration::Instance)
        {
            const AbstractType::Ptr atype=func->abstractType();
            const ClassMemberDeclaration* member=dynamic_cast<const ClassMemberDeclaration*>(func);
            qDebug() << "member var" << member->toString()
                     << "name" << member->identifier().toString()
                     << "type" << member->abstractType()->toString();
            
            bool isReadOnly=atype->modifiers() & AbstractType::ConstModifier;
            QString type=member->abstractType()->toString();
            if(type.contains("const ")) {
                type=type.remove("const ");
                isReadOnly=true;
            }
            
            writeVariable(member->identifier().toString(), type, isReadOnly);
        }
    }
    
    writeEndClass();
}

int DUChainReader::start()
{
    inclass=0;
    
    QList<const Declaration*> decls=extractTypes(m_top);
    writeDocument();
    foreach(const Declaration* decl, decls)
    {
        if(!decl->identifier().toString().startsWith("QMetaTypeId"))
            foundClass(decl);
        
    }
    writeEndDocument();
    
    return 0;
}
