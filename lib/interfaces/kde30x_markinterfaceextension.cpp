/* This file is part of the KDE project
   Copyright (C) 2000-2001 F@lk Brettschneider <falkbr@kdevelop.org>

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
#include <kdeversion.h>
#include "kde30x_markinterfaceextension.h"

#if !(KDE_VERSION > 305)

#include <ktexteditor/document.h>
//----begin-of-copy-of-KDE-3.1-----------------------------------
/* This file is part of the KDE project
   Copyright (C) 2002 Joseph Wenninger <jowenn@kde.org>

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

namespace KTextEditor
{

class PrivateMarkInterfaceExtension
{
  public:
    PrivateMarkInterfaceExtension() {}
    ~PrivateMarkInterfaceExtension() {}
};

};


using namespace KTextEditor;

unsigned int MarkInterfaceExtension::globalMarkInterfaceExtensionNumber = 0;

MarkInterfaceExtension::MarkInterfaceExtension()
{
  globalMarkInterfaceExtensionNumber++;
  myMarkInterfaceExtensionNumber = globalMarkInterfaceExtensionNumber++;

  d = new PrivateMarkInterfaceExtension();
}

MarkInterfaceExtension::~MarkInterfaceExtension()
{
  delete d;
}

unsigned int MarkInterfaceExtension::markInterfaceExtensionNumber () const
{
  return myMarkInterfaceExtensionNumber;
}

void MarkInterfaceExtension::setMarkInterfaceExtensionDCOPSuffix (const QCString &/*suffix*/)
{
  //d->interface->setObjId ("MarkInterfaceExtension#"+suffix);
}

MarkInterfaceExtension *KTextEditor::markInterfaceExtension (Document *doc)
{
  if (!doc)
    return 0;

  return static_cast<MarkInterfaceExtension*>(doc->qt_cast("KTextEditor::MarkInterfaceExtension"));
}

//----end-of-copy-of-KDE-3.1-----------------------------------

#endif // !(KDE_VERSION > 305)
