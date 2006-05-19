/* This file is part of the KDE project
   Copyright (C) 2006 David Nolden <david.nolden.kdevelop@art-master.de>

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
#ifndef KDEVAPPFRONTEND_H
#define KDEVAPPFRONTEND_H


#include <kdevplugin.h>
#include <codemodel.h>

namespace Extensions {

class KDevCodeBrowserFrontend  : public KDevPlugin {
    Q_OBJECT

    public:
        KDevCodeBrowserFrontend(const KDevPluginInfo *info, QObject *parent=0, const char *name=0 )
            :KDevPlugin(info, parent, name ? name : "CodeBrowserFrontend") {}
        
        ///Used by the quickopen-plugin to notify extensions that it jumped to a searched item
        virtual bool jumpedToItem( ItemDom item ) = 0;
};
}

#endif
