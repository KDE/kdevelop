/***************************************************************************
    Copyright (C) 2000 Scott Manson
                       SDManson@alltel.net

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
#ifndef SYNTAXDOCUMENT_H
#define SYNTAXDOCUMENT_H

#include <qdom.h>
#include <qlist.h>

class QStringList;

struct syntaxModeListItem
{
  QString name;
  QString mimetype;
  QString extension;
  QString casesensitive;
};

struct syntaxContextData
{
  QDomElement parent;
  QDomElement currentGroup;
  QDomElement item;
};

typedef QList<syntaxModeListItem> SyntaxModeList;

class SyntaxDocument : public QDomDocument
{
  public:
    SyntaxDocument();
    ~SyntaxDocument();

    QStringList& finddata(const QString& langName,const QString& type);
    SyntaxModeList modeList();

    struct syntaxContextData* getGroupInfo(const QString& langName, const QString &group);
    void freeGroupInfo(struct syntaxContextData* data);
    bool nextItem(struct syntaxContextData* data);
    bool nextGroup(struct syntaxContextData* data);
    struct syntaxContextData* getSubItems(struct syntaxContextData* data);
    QString groupItemData(struct syntaxContextData* data,QString name);
    QString groupData(struct syntaxContextData* data,QString name);
  private:
     void setupModeList();

    SyntaxModeList myModeList;
    QStringList m_data;
};

#endif
