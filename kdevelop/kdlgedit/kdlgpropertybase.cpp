/***************************************************************************
                         kdlgpropertybase.cpp  -  
                             -------------------                                         
    begin                : Thu Mar 18 1999                                           
    copyright            : (C) 1999 by Pascal Krahmer
    email                : pascal@beast.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include "kdlgpropertybase.h"
#include "itemsglobal.h"


KDlgPropertyBase::KDlgPropertyBase(bool fillWithStdEntrys)
{
  numEntrys = 0;

  int i;
  for (i = 0; i<MAX_ENTRYS_PER_WIDGET; i++)
    setProp(i,"","","",0);

  if (fillWithStdEntrys)
    fillWithStandardEntrys();
}


KDlgPropertyEntry *KDlgPropertyBase::getProp(int no) 
{
    if ((no<0) || (no>numEntrys))
	return 0;
    else
	return &props[no];
}


KDlgPropertyEntry* KDlgPropertyBase::getProp(QString name) 
{
  for (int i=0; i<=getEntryCount(); i++)
    {
      if (getProp(i)->name.upper() == name.upper())
        return getProp(i);
    }

  return 0;
}


QString KDlgPropertyBase::getPropValue(QString name) 
{
    return getProp(name)->value;
}


bool KDlgPropertyBase::propValueAsBool(QString name) 
{
    return getPropValue(name) == "true";
}


void KDlgPropertyBase::dumpConstruct(QTextStream *stream, QString classname,
				     QString parentname)
{
  if(classname == "KKeyButton" || classname == "KDatePicker"){
    // last minute workaround -Sandy
    if (classname == "KKeyButton"){
      QString s = "  ";
      s += getPropValue("VarName");
      s += "= new ";
      s += classname;
      s += "(\"";
      s += getPropValue("Name");
      s += "\",";
      s += parentname;
      s += ");\n";
      *stream << s;
    }
    if (classname == "KDatePicker"){
      QString s = "  ";
      s += getPropValue("VarName");
      s += "= new ";
      s += classname;
      s += "(";
      s += parentname;
      s += ");\n";
      *stream << s;
    }
  }
  
  else{
    QString s = "  ";
    s += getPropValue("VarName");
    s += "= new ";
    s += classname;
    s += "(";
    s += parentname;
    s += ",\"";
    s += getPropValue("Name");
    s += "\");\n";
    *stream << s;
  }
}


void KDlgPropertyBase::dumpPropCall(QTextStream *stream, QString methname,
				    QString contents)
{
    QString s = "  ";
    s += getPropValue("VarName");
    s += "->";
    s += methname;
    s += "(";
    s += contents;
    s += ");\n";
    *stream << s;
}


void KDlgPropertyBase::dumpStringPropCall(QTextStream *stream, QString methname,
					  QString name, bool withi18n)
{
  QString contents = getPropValue(name);
  if (withi18n)
    {
      if(contents != ""){
	contents.prepend("i18n(\"");
	contents.append("\")");
      }
      else{
	return; // no dump
      }
    }
  else{
    contents.prepend("\"");
    contents.append("\"");
  }
  dumpPropCall(stream, methname, contents);
}


void KDlgPropertyBase::dumpIntPropCall(QTextStream *stream, QString methname,
				       QString name)
{
    if (getPropValue(name) != "")
	{
	    dumpPropCall(stream, methname, getPropValue(name));
	}
}


void KDlgPropertyBase::dumpBoolPropCall(QTextStream *stream, QString methname,
					QString name, bool iffalse)
{
    // iffalse == true  -> dump only if property is false
    // iffalse == false -> dump only if property is true
    if (iffalse && propValueAsBool(name)){
	return;
    }

    if (getPropValue(name) == ""){
	return;
    }
    
    dumpPropCall(stream, methname, getPropValue(name));
}


void KDlgPropertyBase::dumpColorPropCall(QTextStream *stream, QString methname,
					 QString name)
{
    if (getPropValue(name) != "")
	{
	    QColor col = Str2Color(getPropValue(name));
	    QString contents = "QColor(";
	    contents += QString().setNum(col.red());
	    contents += ", ";
	    contents += QString().setNum(col.green());
	    contents += ", ";
	    contents += QString().setNum(col.blue());
	    contents += ")";
	    dumpPropCall(stream, methname, contents);
	}
}

void KDlgPropertyBase::dumpFontPropCall(QTextStream *stream, QString methname,
					 QString name)
{
    if (getPropValue(name) != "")
	{
	    QFont font = KDlgItemsGetFont(getPropValue(name));
	    QString contents = "QFont(";
	    contents += "\"" + QString(font.family()) +"\"";
	    contents += ", ";
	    contents += QString().setNum(font.pointSize());
	    contents += ", ";
	    contents += QString().setNum(font.weight());
	    if(font.italic())
	    contents += ",true";
	    contents += ")";
	    dumpPropCall(stream, methname, contents);
	}
}


void KDlgPropertyBase::dumpPixmapPropCall(QTextStream *stream, QString methname,
					  QString name)
{
    QString contents = "QPixmap(\"";
    contents += getPropValue(name);
    contents += "\")";
    dumpPropCall(stream, methname, contents);
}


void KDlgPropertyBase::setProp_Name   (QString n, QString name)
{
  int i;
  for (i=0; i<=getEntryCount(); i++)
    if ((getProp(i)) && (getProp(i)->name.upper() == n.upper()))
      setProp_Name(i,name);
}


void KDlgPropertyBase::setProp_Value  (QString n, QString value)
{
  int i;
  for (i=0; i<=getEntryCount(); i++)
    if ((getProp(i)->name.upper() == n.upper()))
      setProp_Value(i,value);
}


void KDlgPropertyBase::setProp_Data  (QString n, QString data)
{
  int i;
  for (i=0; i<=getEntryCount(); i++)
    if ((getProp(i)->name.upper() == n.upper()))
      setProp_Data(i,data);
}


void KDlgPropertyBase::setProp_Group  (QString n, QString group)
{
  int i;
  for (i=0; i<=getEntryCount(); i++)
    if ((getProp(i)) && (getProp(i)->name.upper() == n.upper()))
      setProp_Group(i,group);
}


void KDlgPropertyBase::setProp_Allowed(QString n, int allowed)
{
  int i;
  for (i=0; i<=getEntryCount(); i++)
    if ((getProp(i)) && (getProp(i)->name.upper() == n.upper()))
      setProp_Allowed(i,allowed);
}


int KDlgPropertyBase::getIntFromProp(int nr, int defaultval)
{
  if ((nr > getEntryCount()) || (nr < 0))
    return defaultval;

  QString val = getProp(nr)->value.stripWhiteSpace();

  if (val.length() == 0)
    return defaultval;


  bool ok = true;
  int dest = getProp(nr)->value.toInt(&ok);

  return ok ? dest : defaultval;
}


int KDlgPropertyBase::getIntFromProp(QString name, int defaultval)
{
  int i;
  for (i=0; i<=getEntryCount(); i++)
    {
      if ((getProp(i)) && (getProp(i)->name.upper() == name.upper()))
        return getIntFromProp(i,defaultval);
    }

  return defaultval;
}


void KDlgPropertyBase::fillWithStandardEntrys()
{
  addProp("QuickHelp",               "",       "General",        ALLOWED_STRING);
  addProp("ToolTip",               "",       "General",        ALLOWED_STRING);
  addProp("Name",               "NoName",       "General",        ALLOWED_STRING);
  addProp("IsHidden",           "",             "General",        ALLOWED_BOOL);
  addProp("IsEnabled",          "",             "General",        ALLOWED_BOOL);

  addProp("VarName",            "",             "C++ Code",       ALLOWED_VARNAME);
  //  addProp("Connections",        "",             "C++ Code",       ALLOWED_CONNECTIONS);
  //addProp("ResizeToParent",     "",             "C++ Code",       ALLOWED_BOOL);
  addProp("AcceptsDrops",       "",             "C++ Code",       ALLOWED_BOOL);
  addProp("HasFocus",           "",             "C++ Code",       ALLOWED_BOOL);
  //  addProp("FocusProxy",         "",             "C++ Code",       ALLOWED_STRING);

  addProp("X",                  "10",           "Geometry",       ALLOWED_INT);
  addProp("Y",                  "10",           "Geometry",       ALLOWED_INT);
  addProp("Width",              "100",          "Geometry",       ALLOWED_INT);
  addProp("Height",             "30",           "Geometry",       ALLOWED_INT);
  addProp("MinWidth",           "0",            "Geometry",       ALLOWED_INT);
  addProp("MinHeight",          "0",            "Geometry",       ALLOWED_INT);
  addProp("MaxWidth",           "",             "Geometry",       ALLOWED_INT);
  addProp("MaxHeight",          "",             "Geometry",       ALLOWED_INT);
  addProp("IsFixedSize",        "",             "Geometry",       ALLOWED_BOOL);
  addProp("SizeIncX",           "",             "Geometry",       ALLOWED_INT);
  addProp("SizeIncY",           "",             "Geometry",       ALLOWED_INT);

 //  addProp("BgMode",             "",             "Appearance",     ALLOWED_COMBOLIST,
// 	  "(not set)\n"
// 	  "FixedColor\n"
// 	  "FixedPixmap\n"
// 	  "NoBackground\n"	
// 	  "PaletteForeground\n"
// 	  "PaletteBackground\n"
// 	  "PaletteLight\n"
// 	  "PaletteMidlight\n"
// 	  "PaletteDark\n"
// 	  "PaletteMid\n"
// 	  "PaletteText\n"
// 	  "PaletteBase\n");
  //  addProp("BgColor",            "",             "Appearance",     ALLOWED_COLOR);
  //  addProp("BgPalColor",         "",             "Appearance",     ALLOWED_COLOR);
  addProp("BgPixmap",           "",             "Appearance",     ALLOWED_FILE);
  addProp("MaskBitmap",         "",             "Appearance",     ALLOWED_FILE);
  addProp("Font",               "",             "Appearance",     ALLOWED_FONT);
  //  addProp("Cursor",             "",             "Appearance",     ALLOWED_CURSOR);
}



