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
#ifndef __KDE30X_MARKINTERFACEEXTENSION_H__
#define __KDE30X_MARKINTERFACEEXTENSION_H__

#include <ktexteditor/markinterface.h>
#include <kdeversion.h>

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

#include <qptrlist.h>
#include <qpixmap.h>
#include <qstring.h>

class QCString;

namespace KTextEditor
{


/*
*  This is an interface for the KTextEditor::Document class !!!
*  It allows to set pixmaps for marks and allows the developer to set, which
*  marks be settable by the user. In kate eg in the sidebar's popup menu
*/
class MarkInterfaceExtension
{
  friend class PrivateMarkInterfaceExtension;

  public:
    MarkInterfaceExtension ();
    virtual ~MarkInterfaceExtension ();

    unsigned int markInterfaceExtensionNumber () const;

  protected:
    void setMarkInterfaceExtensionDCOPSuffix (const QCString &suffix);

  public:
    virtual void setPixmap(MarkInterface::MarkTypes, const QPixmap &)=0;
    virtual void setDescription(MarkInterface::MarkTypes, const QString &)=0;
    virtual void setMarksUserChangable(uint markMask)=0;

    enum MarkChangeAction {
		MarkAdded=0,
		MarkRemoved=1
	};
  //
  // slots !!!
  //
  public:

  //
  // signals !!!
  //
  public:
    virtual void markChanged (KTextEditor::Mark mark,
                              KTextEditor::MarkInterfaceExtension::MarkChangeAction action) = 0;

  private:
    class PrivateMarkInterfaceExtension *d;
    static unsigned int globalMarkInterfaceExtensionNumber;
    unsigned int myMarkInterfaceExtensionNumber;
};

MarkInterfaceExtension *markInterfaceExtension (Document *doc);

}; // KTextEditor namespace

//----end-of-copy-of-KDE-3.1-----------------------------------

#endif // !(KDE_VERSION > 305)

#endif // __KDE30X_MARKINTERFACEEXTENSION_H__
