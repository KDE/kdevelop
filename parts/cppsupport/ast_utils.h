/***************************************************************************
 *   Copyright (C) 2002 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __ast_utils_h
#define __ast_utils_h

class AST;
class QListViewItem;

namespace KTextEditor{ class EditInterface; }

AST* findNodeAt( AST* unit, int line, int column );
void buildView( AST* ast, KTextEditor::EditInterface*, QListViewItem* parent );

#endif // __ast_utils_h
