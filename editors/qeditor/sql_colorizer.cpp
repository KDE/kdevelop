/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "sql_colorizer.h"

static const char *sql_keywords[] = {
  "alter", "analyze", "audit", "comment", "commit", "create",
  "delete", "drop", "execute", "explain", "grant", "insert", "lock", "noaudit",
  "rename", "revoke", "rollback", "savepoint", "select", "set",
  "truncate", "update",

  "boolean", "char", "character", "date", "float", "integer", "long",
  "number", "raw", "rowid", "varchar", "varchar2", "varray",

  "false", "null", "true",

  "access", "add", "as", "asc", "begin", "by", "check", "cluster", "column",
  "compress", "connect", "current", "cursor", "decimal", "default", "desc",
  "else", "elsif", "end", "exception", "exclusive", "file", "for", "from",
  "function", "group", "having", "identified", "if", "immediate", "increment",
  "index", "initial", "into", "is", "level", "loop", "maxextents", "mode", "modify",
  "nocompress", "nowait", "of", "offline", "on", "online", "start",
  "successful", "synonym", "table", "then", "to", "trigger", "uid",
  "unique", "user", "validate", "values", "view", "whenever",
  "where", "with", "option", "order", "pctfree", "privileges", "procedure",
  "public", "resource", "return", "row", "rowlabel", "rownum", "rows",
  "session", "share", "size", "smallint", "type", "using",

  "not", "and", "or", "in", "any", "some", "all", "between", "exists",
  "like", "escape", "union", "intersect", "minus", "prior", "distinct",
  "sysdate", "out",

  "left", "right", "outer", "inner", "join",

  0
};

SqlColorizer::SqlColorizer( QEditor* editor )
    : QSourceColorizer( editor )
{
    // default context
    HLItemCollection* context0 = new HLItemCollection( 0 );
    context0->appendChild( new KeywordsHLItem( sql_keywords, Keyword, Normal, 0, true, true ) );
    context0->appendChild( new StartsWithHLItem( "--", Comment, 0 ) );
    context0->appendChild( new StringHLItem( "'", String, 1 ) );
    context0->appendChild( new StringHLItem( "\"", String, 2 ) );

    HLItemCollection* context1 = new HLItemCollection( String );
    context1->appendChild( new StringHLItem( "\\\\", String, 1 ) );
    context1->appendChild( new StringHLItem( "\\'", String, 1 ) );
    context1->appendChild( new StringHLItem( "'", String, 0 ) );

    HLItemCollection* context2 = new HLItemCollection( String );
    context2->appendChild( new StringHLItem( "\\\\", String, 2 ) );
    context2->appendChild( new StringHLItem( "\\\"", String, 2 ) );
    context2->appendChild( new StringHLItem( "\"", String, 0 ) );

    m_items.append( context0 );
    m_items.append( context1 );
    m_items.append( context2 );
}

SqlColorizer::~SqlColorizer()
{
}

