/* This file is part of the KDE project
   Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2002 F@lk Brettschneider <falkbr@kdevelop.org>
   Copyright (C) 2003 Alexander Dymo <cloudtemple@mksat.net>
   Copyright (C) 2003 Amilcar do Carmo Lucas <amilcar@ida.ing.tu-bs.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include "codemodel.h"

#include <kdebug.h>

#include "kdevdesignerintegration.h"
#include "kdevlanguagesupport.h"

KDevLanguageSupport::KDevLanguageSupport(const KDevPluginInfo *info, QObject *parent, const char *name)
    : KDevPlugin(info, parent, name ? name : "KDevLanguageSupport" )
{
}

KDevLanguageSupport::~KDevLanguageSupport()
{
}

KDevLanguageSupport::Features KDevLanguageSupport::features()
{
    return Features(0);
}

KMimeType::List KDevLanguageSupport::mimeTypes()
{
    return KMimeType::List();
}

QString KDevLanguageSupport::formatTag( const Tag& /*tag*/ )
{
    return QString::null;
}

QString KDevLanguageSupport::formatClassName(const QString &name)
{
    return name;
}

QString KDevLanguageSupport::unformatClassName(const QString &name)
{
    return name;
}

void KDevLanguageSupport::addClass()
{
}

void KDevLanguageSupport::addMethod( ClassDom /*klass*/ )
{
}

void KDevLanguageSupport::implementVirtualMethods( ClassDom /*klass*/ )
{
}

void KDevLanguageSupport::addAttribute( ClassDom /*klass*/ )
{
}

QStringList KDevLanguageSupport::subclassWidget(const QString& /*formName*/)
{
    return QStringList();
}

QStringList KDevLanguageSupport::updateWidget(const QString& /*formName*/, const QString& /*fileName*/)
{
    return QStringList();
}

QString KDevLanguageSupport::formatModelItem( const CodeModelItem *item, bool /*shortDescription*/ )
{
    return item->name();
}

void KDevLanguageSupport::addFunction( DesignerType type, const QString & formName, Function function )
{
//    kdDebug() << "KDevLanguageSupport::addFunction: 1" << endl;
    KDevDesignerIntegration *designerIntegration = designer(type);
//    kdDebug() << "KDevLanguageSupport::addFunction: 2" << endl;
    if (!designerIntegration)
    {
//        kdDebug() << "KDevLanguageSupport::addFunction: x" << endl;
        return;
    }
//    kdDebug() << "KDevLanguageSupport::addFunction: 3" << endl;
    designerIntegration->addFunction(formName, function);
//    kdDebug() << "KDevLanguageSupport::addFunction: 4" << endl;
}

void KDevLanguageSupport::editFunction( DesignerType type, const QString & formName, Function oldFunction, Function function )
{
    KDevDesignerIntegration *designerIntegration = designer(type);
    if (!designerIntegration)
        return;
    designerIntegration->editFunction(formName, oldFunction, function);
}

void KDevLanguageSupport::removeFunction( DesignerType type, const QString & formName, Function function )
{
    KDevDesignerIntegration *designerIntegration = designer(type);
    if (!designerIntegration)
        return;
    designerIntegration->removeFunction(formName, function);
}

KDevDesignerIntegration * KDevLanguageSupport::designer( DesignerType type )
{
    return 0;
}

void KDevLanguageSupport::openFunction( DesignerType type, const QString & formName, const QString & functionName )
{
    KDevDesignerIntegration *designerIntegration = designer(type);
    if (!designerIntegration)
        return;
    designerIntegration->openFunction(formName, functionName);
}

#include "kdevlanguagesupport.moc"
