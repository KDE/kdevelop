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


#include "syntaxdocument.h"
#include <qfile.h>
#include <kdebug.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qstringlist.h>

SyntaxDocument::SyntaxDocument() : QDomDocument()
{
  QString syntaxPath = locate("data", "kwrite/syntax.xml");

  if( !syntaxPath.isEmpty() )
  {
    QFile f( syntaxPath );
    if ( f.open(IO_ReadOnly) )
    {
      setContent(&f);
      setupModeList();
    }
    else
      KMessageBox::error( 0L, i18n("Can't open %1").arg(syntaxPath) );

    f.close();
  }
  else
    KMessageBox::error( 0L, i18n("File share/apps/kwrite/syntax.xml not found ! Check your installation!") );
}

SyntaxDocument::~SyntaxDocument()
{
}

void SyntaxDocument::setupModeList()
{
  if (myModeList.count() > 0) return;

  QDomElement docElem = documentElement();
  QDomNodeList nL = docElem.elementsByTagName("language");

 for (int i=0;i<nL.count();i++)
  {
    QDomNode n=nL.item(i);
    if ( n.isElement())
    {
      QDomElement e = n.toElement();
      syntaxModeListItem *mli=new syntaxModeListItem;
      mli->name = e.attribute("name");
      mli->mimetype = e.attribute("mimetype");
      mli->extension = e.attribute("extensions");
      mli->casesensitive = e.attribute("casesensitive");
      myModeList.append(mli);
    }
  }
}

SyntaxModeList SyntaxDocument::modeList()
{
  return myModeList;
}


//QStringList& SyntaxDocument::

bool SyntaxDocument::nextGroup(struct syntaxContextData* data)
{
  if (data->currentGroup.isNull())
    {
      data->currentGroup=data->parent.firstChild().toElement();

    }
  else
    data->currentGroup=data->currentGroup.nextSibling().toElement();
  data->item=QDomElement();
  if (data->currentGroup.isNull()) return false; else return true;
}

bool SyntaxDocument::nextItem(struct syntaxContextData* data)
{
  if (data->item.isNull())
    {
      data->item=data->currentGroup.firstChild().toElement();
    }
  else
    data->item=data->item.nextSibling().toElement();
  if (data->item.isNull()) return false; else return true;
}

QString SyntaxDocument::groupItemData(struct syntaxContextData* data,QString name)
{
  if (!data->item.isNull()) return data->item.attribute(name); else return QString();
}

QString SyntaxDocument::groupData(struct syntaxContextData* data,QString name)
{
    if (!data->currentGroup.isNull()) return data->currentGroup.attribute(name); else return QString();
}

void SyntaxDocument::freeGroupInfo(struct syntaxContextData* data)
{
  delete data;
}


struct syntaxContextData* SyntaxDocument::getSubItems(struct syntaxContextData* data)
{
  syntaxContextData *retval=new syntaxContextData;
  if (data!=0)
    {  
      retval->parent=data->currentGroup;
      retval->currentGroup=data->item;
      retval->item=QDomElement();
   }
  return retval;
}

struct syntaxContextData* SyntaxDocument::getGroupInfo(const QString& langName, const QString &group)
{
  QDomElement docElem = documentElement();
  QDomNode n = docElem.firstChild();

  while (!n.isNull())
    {
      kdDebug()<<"in SyntaxDocument::getGroupInfo (outter loop)"<<endl;
      QDomElement e=n.toElement();
      if (e.attribute("name").compare(langName)==0 )
        {
          QDomNode n1=e.firstChild();
          while (!n1.isNull())
            {
      	      kdDebug()<<"in SyntaxDocument::getGroupInfo (inner loop)"<<endl;
              QDomElement e1=n1.toElement();
              if (e1.tagName()==group+"s")
                {
                 struct syntaxContextData *data=new (struct syntaxContextData);
                 data->parent=e1;
                 return data;
                }
              n1=e1.nextSibling();
            }
          return 0;
        }
      n=e.nextSibling();
    }
  return 0;
}


QStringList& SyntaxDocument::finddata(const QString& langName,const QString& type)
{
  QDomElement docElem = documentElement();
  QDomNode n = docElem.firstChild();

  while ( !n.isNull() )
  {
    if ( n.isElement())
    {
      QDomElement e = n.toElement(); //e.tagName is language
      QDomNode child=e.firstChild(); // child.toElement().tagname() is keywords/types
      QDomNode grandchild=child.firstChild(); // grandchild.tagname is keyword/type

      // at this point e.attribute("name") should equal langName
      // and we can use type==keyword as an index to child list

      if(e.attribute("name").compare(langName)==0 )
      {
        //&& grandchild.toElement().tagName().compare(type) == 0 ){
        QDomNodeList childlist=n.childNodes();
        QDomNodeList grandchildlist=childlist.item(type=="keyword" ? 0 : 1).childNodes();

        for(uint i=0; i< grandchildlist.count();i++)
          m_data+=grandchildlist.item(i).toElement().text().stripWhiteSpace();

        return m_data;
      }
    }

    n = n.nextSibling();
  }

  return m_data;
}
