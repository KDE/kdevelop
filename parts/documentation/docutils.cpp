/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "docutils.h"

#include <kurlrequester.h>
#include <kurlcompletion.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <kpopupmenu.h>
#include <klocale.h>

#include <kdevpartcontroller.h>

#include "kdevdocumentationplugin.h"
#include "documentation_part.h"

QString DocUtils::noEnvURL(const QString &url)
{
    return KURLCompletion::replacedPath(url, true, true);
}

QString DocUtils::envURL(KURLRequester *req)
{
    if (req->lineEdit())
        return req->lineEdit()->text();
    else if (req->comboBox())
        return req->comboBox()->currentText();
    else
        return req->url();
}

void DocUtils::docItemPopup(DocumentationPart *part, DocumentationItem *docItem,
    const QPoint &pos, bool showBookmark, bool showSearch, int titleCol)
{
    docItemPopup(part, docItem->text(titleCol), docItem->url(), pos, showBookmark, showSearch);
}

void DocUtils::docItemPopup(DocumentationPart *part, IndexItem *docItem, const QPoint &pos,
    bool showBookmark, bool showSearch)
{
    docItemPopup(part, docItem->text(), docItem->urls().first(), pos, showBookmark, showSearch);
}

void DocUtils::docItemPopup(DocumentationPart *part, const QString &title, const KURL &url,
    const QPoint &pos, bool showBookmark, bool showSearch)
{
    KPopupMenu menu;
    menu.setTitle(i18n("Documentation"));
    menu.insertItem(i18n("Open in Current Tab"), 1);
    menu.insertItem(i18n("Open in New Tab"), 2);
    if (showBookmark)
    {
        menu.insertSeparator();   
        menu.insertItem(i18n("Bookmark This Location"), 3);
    }
    if (showSearch)
    {
        menu.insertSeparator();
        menu.insertItem(QString("%1: %2").arg(i18n("Search")).arg(title), 4);
    }
    
    switch (menu.exec(pos))
    {
        case 1: part->partController()->showDocument(url); break;
        case 2: part->partController()->showDocument(url, true); break;
        case 3: part->emitBookmarkLocation(title, url); break;
        case 4: break;
    }
}
