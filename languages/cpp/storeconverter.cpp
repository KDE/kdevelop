/***************************************************************************
*   Copyright (C) 2003-2004 by Alexander Dymo                             *
*   adymo@kdevelop.org                                                    *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#include "storeconverter.h"

#include <qvaluelist.h>

#include <kdebug.h>

#include <kdevcoderepository.h>

#include "cppcodecompletion.h"
#include "codeinformationrepository.h"
#include "cppsupportpart.h"

StoreConverter::StoreConverter(CppSupportPart *part, CodeModel *model)
    :m_part(part), m_model(model)
{
}

void StoreConverter::PCSClassToCodeModel(const QString &className, const QStringList &classScope)
{
    QValueList<Catalog*> catalogs = m_part->codeRepository()->registeredCatalogs();
    for (QValueList<Catalog*>::iterator it = catalogs.begin();
        it != catalogs.end(); ++it)
    {
        Catalog *catalog = *it;
        kdDebug() << "looking in catalog: " << catalog->dbName() << endl;

        QValueList<Catalog::QueryArgument> args;
        args << Catalog::QueryArgument( "kind", Tag::Kind_Class );
        args << Catalog::QueryArgument( "name", className );
        QValueList<Tag> tags = catalog->query(args);
        for (QValueList<Tag>::iterator it = tags.begin(); it != tags.end(); ++it )
        {
            Tag& tag = *it;
            kdDebug() << "TAG: " << tag.name() << " in file " << tag.fileName() << endl;
            FileDom file;
            if (m_model->hasFile(tag.name()))
                file = m_model->fileByName(tag.name());
            else
            {
                file = m_model->create<FileModel>();
                file->setName(tag.fileName());
                m_model->addFile(file);
            }
            if (!file->hasClass(tag.name()))
                parseClass(tag, file);
        }
    }       
}

void StoreConverter::parseClass(Tag &classTag, FileDom file)
{
    ClassDom klass = m_model->create<ClassModel>();
    klass->setName(classTag.name());
    klass->setFileName(classTag.fileName());
    
    QStringList scope;
    scope.append(classTag.name());
    QValueList<Tag> symbolTags = m_part->codeCompletion()->repository()->
        getTagsInScope(scope, false);
    
    kdDebug() << "got tags: " << endl;
    for (QValueList<Tag>::iterator sit = symbolTags.begin(); sit != symbolTags.end(); ++sit )
    {
        Tag &symbol = *sit;
        kdDebug() << symbol.name() << endl;
        
        switch (symbol.kind()) 
        {
            case Tag::Kind_FunctionDeclaration:
                parseFunctionDeclaration(symbol, klass);
                break;
            case Tag::Kind_Variable:
                parseVariable(symbol, klass);
        }
    }

    QValueList<Tag> baseClassTags = m_part->codeCompletion()->repository()->getBaseClassList(classTag.name());
    for (QValueList<Tag>::iterator bit = baseClassTags.begin();
        bit != baseClassTags.end(); ++bit)
        klass->addBaseClass((*bit).name());
}

void StoreConverter::parseFunctionDeclaration(Tag &fun, ClassDom klass)
{
}

void StoreConverter::parseVariable(Tag &var, ClassDom klass)
{
}
