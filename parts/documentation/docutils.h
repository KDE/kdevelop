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
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef DOC_UTILS_H
#define DOC_UTILS_H

#include <qstring.h>

class KURLRequester;
class KPopupMenu;
class DocumentationItem;
class DocumentationPart;
class QPoint;
class IndexItem;
class KURL;

namespace DocUtils{

/**Returns an url selected in KURLRequester but does not
resolve environment variables.*/
QString envURL(KURLRequester *req);
/**Resolves environment variables in url.*/
QString noEnvURL(const QString &url);
/**Resolves environment variables in url.*/
KURL noEnvURL(const KURL &url);

void docItemPopup(DocumentationPart *part, DocumentationItem *docItem, const QPoint &pos,
    bool showBookmark = false, bool showSearch = false, int titleCol = 0);
void docItemPopup(DocumentationPart *part, IndexItem *docItem, const QPoint &pos,
    bool showBookmark = false, bool showSearch = false);
void docItemPopup(DocumentationPart *part, const QString &title, const KURL &url,
    const QPoint &pos, bool showBookmark = false, bool showSearch = false);
}

#endif
