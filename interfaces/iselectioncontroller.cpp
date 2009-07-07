/* This file is part of KDevelop
Copyright 2009 Andreas Pakulat <apaku@gmx.de>

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
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#include "iselectioncontroller.h"

#include "icore.h"
#include "idocumentcontroller.h"
#include "iprojectcontroller.h"
#include "iproject.h"
#include "context.h"

#include "project/projectmodel.h"

#include <KUrl>

namespace KDevelop
{

ISelectionController::ISelectionController( QObject* parent )
    : QObject( parent )
{
}

ISelectionController::~ISelectionController()
{
}

KUrl ISelectionController::folderFromSelection()
{
    KUrl u;
    
    KDevelop::Context * sel = ICore::self()->selectionController()->currentSelection();
    KDevelop::FileContext * fc = dynamic_cast<FileContext*>(sel);
    KDevelop::ProjectItemContext * pc = dynamic_cast<ProjectItemContext*>(sel);
    if(fc && !fc->urls().isEmpty())
      u = fc->urls()[0].upUrl();
    else if(pc && !pc->items().isEmpty() && pc->items()[0]->folder())
      ;//TODO check how to solve cyclic dependancy
      //u = pc->items()[0]->folder()->url();
    else if(ICore::self()->documentController()->activeDocument())
      u = ICore::self()->documentController()->activeDocument()->url().upUrl();
    else if(!ICore::self()->projectController()->projects().isEmpty())
      u = ICore::self()->projectController()->projects()[0]->folder();
      
    return u;
}

}



#include "iselectioncontroller.moc"
