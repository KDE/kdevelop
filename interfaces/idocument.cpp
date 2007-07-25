/***************************************************************************
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                  *
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
#include "idocument.h"

#include "icore.h"
#include "idocumentcontroller.h"

namespace KDevelop {

class IDocumentPrivate
{
public:
    KDevelop::ICore* m_core;
};

IDocument::IDocument( KDevelop::ICore* core )
  : d(new IDocumentPrivate)
{
    d->m_core = core;
}

IDocument::~IDocument()
{
    delete d;
}

KDevelop::ICore* IDocument::core()
{
    return d->m_core;
}

void IDocument::notifySaved()
{
    emit core()->documentController()->documentSaved(this);
}

void IDocument::notifyStateChanged()
{
    emit core()->documentController()->documentStateChanged(this);
}

void IDocument::notifyActivated()
{
    emit core()->documentController()->documentActivated(this);
}

void IDocument::notifyContentChanged() {
    emit core()->documentController()->documentContentChanged(this);
}

}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
