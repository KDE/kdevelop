/*
   Copyright (C) 1998, 1999 Jochen Wilhelmy
                            digisnap@cs.tu-berlin.de

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

#include <string.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qfile.h>
#include <qlabel.h>

#include <qlayout.h>
#include <qgrid.h>
#include <qhbox.h>
#include <qvgroupbox.h>

#include <kconfig.h>
#include <kinstance.h>
#include <kglobal.h>
#include <kfontdialog.h>
#include <kcharsets.h>
#include <kmimemagic.h>
#include <klocale.h>
#include <kregexp.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <kstddirs.h>

#include "kwtextline.h"
#include "kwattribute.h"
#include "kwrite_factory.h"
#include "highlight.h"
#include "syntaxdocument.h"

HlManager *HlManager::s_pSelf = 0;

char fontSizes[] = {4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,22,24,26,28,32,48,64,0};

enum Item_styles { dsNormal,dsKeyword,dsDataType,dsDecVal,dsBaseN,dsFloat,
                   dsChar,dsString,dsComment,dsOthers};

int getDefStyleNum(QString name)
  {
	if (name=="dsNormal") return dsNormal;
        if (name=="dsKeyword") return dsKeyword;
        if (name=="dsDataType") return dsDataType;
        if (name=="dsDecVal") return dsDecVal;
        if (name=="dsBaseN") return dsBaseN;
        if (name=="dsFloat") return dsFloat;
        if (name=="dsChar") return dsChar;
        if (name=="dsString") return dsString;
        if (name=="dsComment") return dsComment;
        if (name=="dsOthers")  return dsOthers;
	return dsNormal;
  }

bool isInWord(QChar ch) {
  return ch.isLetter() || ch.isDigit() || ch == '_';
/*  static unsigned char data[] = {0,0,0,0,0,0,255,3,254,255,255,135,254,255,255,7};
  if (c & 128) return false;
  return !(data[c >> 3] & (1 << (c & 7)));*/
}

bool ucmp(const QChar *u, const char *s, int len) {
  while (len > 0) {
    if (*u != *s) return false;
    u++;
    s++;
    len--;
  }
  return true;
}

bool ustrchr(const char *s, QChar c) {
  while (*s != '\0') {
    if (*s == c) return true;
    s++;
  }
  return false;
}





HlItem::HlItem(int attribute, int context)
  : attr(attribute), ctx(context)  {subItems=0;
}

HlItem::~HlItem()
{
  kdDebug()<<"In hlItem::~HlItem()"<<endl;
  if (subItems!=0) {subItems->setAutoDelete(true); subItems->clear(); delete subItems;}
}

HlItemWw::HlItemWw(int attribute, int context)
  : HlItem(attribute,context) {
}


HlCharDetect::HlCharDetect(int attribute, int context, QChar c)
  : HlItem(attribute,context), sChar(c) {
}

const QChar *HlCharDetect::checkHgl(const QChar *str) {
  if (*str == sChar) return str + 1;
  return 0L;
}

Hl2CharDetect::Hl2CharDetect(int attribute, int context, QChar ch1, QChar ch2)
  : HlItem(attribute,context) {
  sChar1 = ch1;
  sChar2 = ch2;
}

const QChar *Hl2CharDetect::checkHgl(const QChar *str) {
  if (str[0] == sChar1 && str[1] == sChar2) return str + 2;
  return 0L;
}

HlStringDetect::HlStringDetect(int attribute, int context, const QString &s, bool inSensitive)
  : HlItem(attribute, context), str(inSensitive ? s.upper():s), _inSensitive(inSensitive) {
}

HlStringDetect::~HlStringDetect() {
}

const QChar *HlStringDetect::checkHgl(const QChar *s) {
  if (!_inSensitive) {if (memcmp(s, str.unicode(), str.length()*sizeof(QChar)) == 0) return s + str.length();}
     else
       {
	 QString tmp=QString(s,str.length()).upper();
	 if (tmp==str) return s+str.length();		
       }
  return 0L;
}


HlRangeDetect::HlRangeDetect(int attribute, int context, QChar ch1, QChar ch2)
  : HlItem(attribute,context) {
  sChar1 = ch1;
  sChar2 = ch2;
}

const QChar *HlRangeDetect::checkHgl(const QChar *s) {
  if (*s == sChar1) {
    do {
      s++;
      if (*s == '\0') return 0L;
    } while (*s != sChar2);
    return s + 1;
  }
  return 0L;
}

/*
KeywordData::KeywordData(const char *str) {
  len = strlen(str);
  s = new char[len];
  memcpy(s,str,len);
}

KeywordData::~KeywordData() {
  delete s;
}
*/
HlKeyword::HlKeyword(int attribute, int context)
  : HlItemWw(attribute,context) {
//  words.setAutoDelete(true);
// after reading over the docs for Dict
// 23 is probably too small when we can have > 100 items
	QDict<char> dict(113);
	Dict=dict;
}

HlKeyword::~HlKeyword() {
}

// If we use a dictionary for lookup we don't really need
// an item as such we are using the key to lookup
void HlKeyword::addWord(const QString &word)
{
  words.append(word);
  Dict.insert(word,"dummy");
}
void HlKeyword::addList(const QStringList& list)
{
 words+=list;
 for(uint i=0;i<list.count();i++) Dict.insert(list[i],"dummy");
}

void HlKeyword::addList(const char **list) {

  while (*list) {
    words.append(*list);
    Dict.insert(*list,"dummy");
    list++;
  }
}

const QChar *HlKeyword::checkHgl(const QChar *s) {
#if 0
  for (QStringList::Iterator it = words.begin(); it != words.end(); ++it) {
    if (memcmp(s, (*it).unicode(), (*it).length()*sizeof(QChar)) == 0) {
      return s + (*it).length();
    }
  }
	return 0L;
#else
// this seems to speed up the lookup of keywords somewhat
// anyway it has to be better than iterating through the list

  const QChar *s2=s;

  while( !ustrchr("!%&()*+,-./:;<=>?[]^{|}~ ", *s2) && *s2 != '\0') s2++;
// oops didn't increment s2 why do anything else?
	if(s2 == s) return 0L;
  QString lookup=QString(s,s2-s)+QString::null;
  return Dict[lookup] ? s2 : 0L;
#endif
}


HlInt::HlInt(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const QChar *HlInt::checkHgl(const QChar *str) {
  const QChar *s,*s1;

  s = str;
  while (s->isDigit()) s++;
  if (s > str)
   {
     if (subItems)
       {
	 for (HlItem *it=subItems->first();it;it=subItems->next())
          {
            s1=it->checkHgl(s);
	    if (s1) return s1;
          }
       }
     return s;
  }
  return 0L;
}

HlFloat::HlFloat(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const QChar *HlFloat::checkHgl(const QChar *s) {
  bool b, p;
  const QChar *s1;

  b = false;
  while (s->isDigit()){
    s++;
    b = true;
  }
  if (p = (*s == '.')) {
    s++;
    while (s->isDigit()) {
      s++;
      b = true;
    }
  }
  if (!b) return 0L;
  if ((*s&0xdf) == 'E') s++;
    else
      if (!p) return 0L;
	else
	{
          if (subItems)
            {
	      for (HlItem *it=subItems->first();it;it=subItems->next())
                {
                  s1=it->checkHgl(s);
	          if (s1) return s1;
                }
            }
          return s;
        } 
  if (*s == '-') s++;
  b = false;
  while (s->isDigit()) {
    s++;
    b = true;
  }
  if (b)
    {
      if (subItems)
        {
          for (HlItem *it=subItems->first();it;it=subItems->next())
            {
              s1=it->checkHgl(s);
              if (s1) return s1;
            }
        }
      return s;
    }
   else return 0L;
}


HlCInt::HlCInt(int attribute, int context)
  : HlInt(attribute,context) {
}

const QChar *HlCInt::checkHgl(const QChar *s) {

//  if (*s == '0') s++; else s = HlInt::checkHgl(s);
  s = HlInt::checkHgl(s);
  if (s != 0L) {
    int l = 0;
    int u = 0;
    const QChar *str;

    do {
      str = s;
      if ((*s&0xdf) == 'L' ) {
        l++;
        if (l > 2) return 0L;
        s++;
      }
      if ((*s&0xdf) == 'U' ){
        u++;
        if (u > 1) return 0L;
        s++;
      }
    } while (s != str);
  }
  return s;
}

HlCOct::HlCOct(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const QChar *HlCOct::checkHgl(const QChar *str) {
  const QChar *s;

  if (*str == '0') {
    str++;
    s = str;
    while (*s >= '0' && *s <= '7') s++;
    if (s > str) {
      if ((*s&0xdf) == 'L' || (*s&0xdf) == 'U' ) s++;
      return s;
    }
  }
  return 0L;
}

HlCHex::HlCHex(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const QChar *HlCHex::checkHgl(const QChar *str) {
  const QChar *s;

  if (str[0] == '0' && ((str[1]&0xdf) == 'X' )) {
    str += 2;
    s = str;
    while (s->isDigit() || ((*s&0xdf) >= 'A' && (*s&0xdf) <= 'F') /*|| (*s >= 'a' && *s <= 'f')*/) s++;
    if (s > str) {
      if ((*s&0xdf) == 'L' || (*s&0xdf) == 'U' ) s++;
      return s;
    }
  }
  return 0L;
}

HlCFloat::HlCFloat(int attribute, int context)
  : HlFloat(attribute,context) {
}

const QChar *HlCFloat::checkHgl(const QChar *s) {

  s = HlFloat::checkHgl(s);
  if (s && ((*s&0xdf) == 'F' )) s++;
  return s;
}

HlAnyChar::HlAnyChar(int attribute, int context, char* charList)
  : HlItem(attribute, context) {
  _charList=charList;  
}

const QChar *HlAnyChar::checkHgl(const QChar *s) {
  kdDebug()<<"in AnyChar::checkHgl: _charList: "<<_charList<<endl;
  if (ustrchr(_charList, *s)) return s +1;
  return 0L;
}

HlCSymbol::HlCSymbol(int attribute, int context)
  : HlItem(attribute, context) {
}

const QChar *HlCSymbol::checkHgl(const QChar *s) {
  if (ustrchr("!%&()*+,-./:;<=>?[]^{|}~", *s)) return s +1;
  return 0L;
}


HlLineContinue::HlLineContinue(int attribute, int context)
  : HlItem(attribute,context) {
}

const QChar *HlLineContinue::checkHgl(const QChar *s) {
  if (*s == '\\') return s + 1;
  return 0L;
}


HlCStringChar::HlCStringChar(int attribute, int context)
  : HlItem(attribute,context) {
}

//checks for hex and oct (for example \x1b or \033)
const QChar *checkCharHexOct(const QChar *str) {
  const QChar *s;
        s=str;
        int n;
  if (*s == 'x') {
    n = 0;
    do {
      s++;
      n *= 16;
      if (s->isDigit()) n += *s - '0';
      else if ((*s&0xdf) >= 'A' && (*s&0xdf) <= 'F') n += (*s&0xdf) - 'A' + 10;
//      else if (*s >= 'a' && *s <= 'f') n += *s - 'a' + 10;
      else break;
      if (n >= 256) return 0L;
    } while (true);
    if (s - str == 1) return 0L;
  } else {
    if (!(*s >= '0' && *s <= '7')) return 0L;
    n = *s - '0';
    do {
      s++;
      n *= 8;
      if (*s >= '0' && *s <= '7') n += *s - '0'; else break;
      if (n >= 256) return s;
    } while (s - str < 3);
  }
  return s;
}
// checks for C escaped chars \n and escaped hex/octal chars
const QChar *checkEscapedChar(const QChar *s) {
  int i;
  if (s[0] == '\\' && s[1] != '\0' ) {
        s++;
        switch(*s){
                case  'a': // checks for control chars
                case  'b': // we want to fall through
                case  'e':
                case  'f':

                case  'n':
                case  'r':
                case  't':
                case  'v':
                case '\'':
                case '\"':
                case '?' : // added ? ANSI C classifies this as an escaped char
                case '\\': s++;
                           break;
                case 'x': // if it's like \xff
                        s++; // eat the x
                        // these for loops can probably be
                        // replaced with something else but
                        // for right now they work
                        // check for hexdigits
                        for(i=0;i<2 &&(*s >= '0' && *s <= '9' || (*s&0xdf) >= 'A' && (*s&0xdf) <= 'F');i++,s++);
                        if(i==0) return 0L; // takes care of case '\x'
                        break;

                case '0': case '1': case '2': case '3' :
                case '4': case '5': case '6': case '7' :
                        for(i=0;i < 3 &&(*s >='0'&& *s<='7');i++,s++);
                        break;
                        default: return 0L;
        }
  return s;
  }
  return 0L;
}

const QChar *HlCStringChar::checkHgl(const QChar *str) {
  return checkEscapedChar(str);
}


HlCChar::HlCChar(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const QChar *HlCChar::checkHgl(const QChar *str) {
  const QChar *s;

  if (str[0] == '\'' && str[1] != '\0' && str[1] != '\'') {
    s = checkEscapedChar(&str[1]); //try to match escaped char
    if (!s) s = &str[2];           //match single non-escaped char
    if (*s == '\'') return s + 1;
  }
  return 0L;
}

HlCPrep::HlCPrep(int attribute, int context)
  : HlItem(attribute,context) {
}

const QChar *HlCPrep::checkHgl(const QChar *s) {

  while (*s == ' ' || *s == '\t') s++;
  if (*s == '#') {
    s++;
    return s;
  }
  return 0L;
}

HlHtmlTag::HlHtmlTag(int attribute, int context)
  : HlItem(attribute,context) {
}

const QChar *HlHtmlTag::checkHgl(const QChar *s) {
  while (*s == ' ' || *s == '\t') s++;
  while (*s != ' ' && *s != '\t' && *s != '>' && *s != '\0') s++;
  return s;
}

HlHtmlValue::HlHtmlValue(int attribute, int context)
  : HlItem(attribute,context) {
}

const QChar *HlHtmlValue::checkHgl(const QChar *s) {
  while (*s == ' ' || *s == '\t') s++;
  if (*s == '\"') {
    do {
      s++;
      if (*s == '\0') return 0L;
    } while (*s != '\"');
    s++;
  } else {
    while (*s != ' ' && *s != '\t' && *s != '>' && *s != '\0') s++;
  }
  return s;
}

HlShellComment::HlShellComment(int attribute, int context)
  : HlCharDetect(attribute,context,'#') {
}

HlMHex::HlMHex(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const QChar *HlMHex::checkHgl(const QChar *s) {

  if (s->isDigit()) {
    s++;
    while ((s->isDigit()) || (*s >= 'A' && *s <= 'F')) s++;
    if (*s == 'H') return s + 1;
  }
  return 0L;
}

HlAdaDec::HlAdaDec(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const QChar *HlAdaDec::checkHgl(const QChar *s) {
  const QChar *str;

  if (s->isDigit()) {
    s++;
    while ((s->isDigit()) || *s == '_') s++;
    if ((*s&0xdf) != 'E') return s;
    s++;
    str = s;
    while ((s->isDigit()) || *s == '_') s++;
    if (s > str) return s;
  }
  return 0L;
}

HlAdaBaseN::HlAdaBaseN(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const QChar *HlAdaBaseN::checkHgl(const QChar *s) {
  int base;
  QChar c1, c2, c3;
  const QChar *str;

  base = 0;
  while (s->isDigit()) {
    base *= 10;
    base += *s - '0';
    if (base > 16) return 0L;
    s++;
  }
  if (base >= 2 && *s == '#') {
    s++;
    c1 = '0' + ((base <= 10) ? base : 10);
    c2 = 'A' + base - 10;
    c3 = 'a' + base - 10;
    while ((*s >= '0' && *s < c1) || (*s >= 'A' && *s < c2)
      || (*s >= 'a' && *s < c3) || *s == '_') {
      s++;
    }
    if (*s == '#') {
      s++;
      if ((*s&0xdf) != 'E') return s;
      s++;
      str = s;
      while ((s->isDigit()) || *s == '_') s++;
      if (s > str) return s;
    }
  }
  return 0L;
}

HlAdaFloat::HlAdaFloat(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const QChar *HlAdaFloat::checkHgl(const QChar *s) {
  const QChar *str;

  str = s;
  while (s->isDigit()) s++;
  if (s > str && *s == '.') {
    s++;
    str = s;
    while (s->isDigit()) s++;
    if (s > str) {
      if ((*s&0xdf) != 'E') return s;
      s++;
      if (*s == '-') s++;
      str = s;
      while ((s->isDigit()) || *s == '_') s++;
      if (s > str) return s;
    }
  }
  return 0L;
}

HlAdaChar::HlAdaChar(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const QChar *HlAdaChar::checkHgl(const QChar *s) {
  if (s[0] == '\'' && s[1] && s[2] == '\'') return s + 3;
  return 0L;
}

HlSatherClassname::HlSatherClassname(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const QChar *HlSatherClassname::checkHgl(const QChar *s) {
  if (*s == '$') s++;

  if (*s >= 'A' && *s <= 'Z') {
    s++;
    while ((*s >= 'A' && *s <= 'Z')
           || (s->isDigit())
           || *s == '_') s++;
    return s;
  }
  return 0L;
}

HlSatherIdent::HlSatherIdent(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const QChar *HlSatherIdent::checkHgl(const QChar *s) {
  if (s->isLetter()) {
    s++;
                while(isInWord(*s)) s++;
    if (*s == '!') s++;
    return s;
  }
  return 0L;
}

HlSatherDec::HlSatherDec(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const QChar *HlSatherDec::checkHgl(const QChar *s) {
  if (s->isDigit()) {
    s++;
    while ((s->isDigit()) || *s == '_') s++;
    if (*s == 'i') s++;
    return s;
  }
  return 0L;
}

HlSatherBaseN::HlSatherBaseN(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const QChar *HlSatherBaseN::checkHgl(const QChar *s) {
  if (*s == '0') {
    s++;
    if (*s == 'x') {
      s++;
      while ((s->isDigit())
//           || (*s >= 'a' && *s <= 'f')
             || ((*s&0xdf) >= 'A' && (*s&0xdf) <= 'F')
             || *s == '_') s++;
    } else if (*s == 'o') {
      s++;
      while ((*s >= '0' && *s <= '7') || *s == '_') s++;
    } else if (*s == 'b') {
      s++;
      while (*s == '0' || *s == '1' || *s == '_') s++;
    } else
      return 0L;
    if (*s == 'i') s++;
    return s;
  }
  return 0L;
}

HlSatherFloat::HlSatherFloat(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const QChar *HlSatherFloat::checkHgl(const QChar *s) {
  if (s->isDigit()) {
    s++;
    while ((s->isDigit()) || *s == '_') s++;
    if (*s == '.') {
      s++;
      while (s->isDigit()) s++;
      if ((*s&0xdf) == 'E') {
        s++;
        if (*s == '-') s++;
        if (s->isDigit()) {
          s++;
          while ((s->isDigit()) || *s == '_') s++;
        } else
          return 0L;
      }
      if (*s == 'i') return s+1;
      if (*s == 'd') s++;
      if (*s == 'x') s++;
                // "dx" is allowed too
      return s;
    }
  }
  return 0L;
}

HlSatherChar::HlSatherChar(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const QChar *HlSatherChar::checkHgl(const QChar *s) {
  if (*s == '\'') {
    s++;
    if (*s == '\\') {
      s++;
      if (*s == 'a' || *s == 'b' || *s == 'f' || *s == 'n'
          || *s == 'r' || *s == 't' || *s == 'v' || *s == '\\'
          || *s == '\'' || *s == '\"') s++;
      else if (*s>='0' && *s<='7')
        while (*s>='0' && *s<='7') s++;
      else
        return 0L;
    } else if (*s != '\0') s++;
  }
  if (*s == '\'')
    return s+1;
  else
    return 0L;
}

HlSatherString::HlSatherString(int attribute, int context)
  : HlItemWw(attribute, context) {
}

const QChar *HlSatherString::checkHgl(const QChar *s) {
  if (*s == '\"') {
    s++;
    while (*s != '\"') {
      if (*s == '\\')
        s++;
      if (*s == '\n' || *s == '\0')
        return s;
      s++;
    }
    return s+1;
  }
  return 0L;
}


HlLatexTag::HlLatexTag(int attribute, int context)
  : HlItem(attribute, context) {
}

const QChar *HlLatexTag::checkHgl(const QChar *s) {
  const QChar *str;

  if (*s == '\\') {
    s++;
    if (*s == ' ' || *s == '/' || *s == '\\') return s +1;
    str = s;
    while (((*s&0xdf) >= 'A' && (*s&0xdf) <= 'Z')
      || (s->isDigit()) || *s == '@') {
      s++;
    }
    if (s != str) return s;
  } /*else if (*s == '$') return s +1*/;
  return 0L;
}

HlLatexChar::HlLatexChar(int attribute, int context)
  : HlItem(attribute, context) {
}

const QChar *HlLatexChar::checkHgl(const QChar *s) {
  if (*s == '\\') {
    s++;
    if (*s && strchr("{}$&#_%", *s)) return s +1;
  }/* else if (*s == '"') {
    s++;
    if (*s && (*s < '0' || *s > '9')) return s +1;
  } */
  return 0L;
}

HlLatexParam::HlLatexParam(int attribute, int context)
  : HlItem(attribute, context) {
}

const QChar *HlLatexParam::checkHgl(const QChar *s) {
  if (*s == '#') {
    s++;
    while (s->isDigit()) {
      s++;
    }
    return s;
  }
  return 0L;
}

//--------
ItemStyle::ItemStyle() : selCol(Qt::white), bold(false), italic(false) {
}

ItemStyle::ItemStyle(const QColor &col, const QColor &selCol,
  bool bold, bool italic)
  : col(col), selCol(selCol), bold(bold), italic(italic) {
}

ItemFont::ItemFont() : family("courier"), size(12), charset("") {
}

ItemData::ItemData(const char * name, int defStyleNum)
  : name(name), defStyleNum(defStyleNum), defStyle(true), defFont(true) {
}

ItemData::ItemData(const char * name, int defStyleNum,
  const QColor &col, const QColor &selCol, bool bold, bool italic)
  : ItemStyle(col,selCol,bold,italic), name(name), defStyleNum(defStyleNum),
  defStyle(false), defFont(true) {
}

HlData::HlData(const QString &wildcards, const QString &mimetypes)
  : wildcards(wildcards), mimetypes(mimetypes) {

  itemDataList.setAutoDelete(true);
}

Highlight::Highlight(const char * name) : iName(name), refCount(0)
{


}

Highlight::~Highlight() {
	
}

KConfig *Highlight::getKConfig() {
  KConfig *config;

  config = KWriteFactory::instance()->config();
  config->setGroup(QString::fromUtf8(iName) + QString::fromUtf8(" Highlight"));
  return config;
}

QString Highlight::getWildcards() {
  KConfig *config;

  config = getKConfig();

  //if wildcards not yet in config, then use iWildCards as default
  return config->readEntry("Wildcards", iWildcards);
}


QString Highlight::getMimetypes() {
  KConfig *config;

  config = getKConfig();

  return config->readEntry("Mimetypes", iMimetypes);
}


HlData *Highlight::getData() {
  KConfig *config;
  HlData *hlData;

  config = getKConfig();

//  iWildcards = config->readEntry("Wildcards");
//  iMimetypes = config->readEntry("Mimetypes");
//  hlData = new HlData(iWildcards,iMimetypes);
  hlData = new HlData(
    config->readEntry("Wildcards", iWildcards),
    config->readEntry("Mimetypes", iMimetypes));
  getItemDataList(hlData->itemDataList, config);
  return hlData;
}

void Highlight::setData(HlData *hlData) {
  KConfig *config;

  config = getKConfig();

//  iWildcards = hlData->wildcards;
//  iMimetypes = hlData->mimetypes;

  config->writeEntry("Wildcards",hlData->wildcards);
  config->writeEntry("Mimetypes",hlData->mimetypes);

  setItemDataList(hlData->itemDataList,config);
}

void Highlight::getItemDataList(ItemDataList &list) {
  KConfig *config;

  config = getKConfig();
  getItemDataList(list, config);
}

void Highlight::getItemDataList(ItemDataList &list, KConfig *config) {
  ItemData *p;
  QString s;
  QRgb col, selCol;
  char family[96];
  char charset[48];

  list.clear();
  list.setAutoDelete(true);
  createItemData(list);

  for (p = list.first(); p != 0L; p = list.next()) {
    s = config->readEntry(p->name);
    if (!s.isEmpty()) {
      sscanf(s.latin1(),"%d,%X,%X,%d,%d,%d,%95[^,],%d,%47[^,]",
        &p->defStyle,&col,&selCol,&p->bold,&p->italic,
        &p->defFont,family,&p->size,charset);
      p->col.setRgb(col);
      p->selCol.setRgb(selCol);
      p->family = family;
      p->charset = charset;
    }
  }
}

void Highlight::setItemDataList(ItemDataList &list, KConfig *config) {
  ItemData *p;
  QString s;

  for (p = list.first(); p != 0L; p = list.next()) {
    s.sprintf("%d,%X,%X,%d,%d,%d,%1.95s,%d,%1.47s",
      p->defStyle,p->col.rgb(),p->selCol.rgb(),p->bold,p->italic,
      p->defFont,p->family.utf8().data(),p->size,p->charset.utf8().data());
    config->writeEntry(p->name,s);
  }
}

void Highlight::use() {
  if (refCount == 0) init();
  refCount++;
}

void Highlight::release() {
  refCount--;
  if (refCount == 0) done();
}

/*
bool Highlight::isInWord(char ch) {
  static char data[] = {0,0,0,0,0,0,255,3,254,255,255,135,254,255,255,7};
  if (ch & 128) return true;
  return data[ch >> 3] & (1 << (ch & 7));
}
*/
int Highlight::doHighlight(int, TextLine *textLine) {

  textLine->setAttribs(0,0,textLine->length());
  textLine->setAttr(0);
  return 0;
}

void Highlight::createItemData(ItemDataList &list) {

  list.append(new ItemData(I18N_NOOP("Normal Text"), dsNormal));
}


void Highlight::init() {
}

void Highlight::done() {
}


HlContext::HlContext(int attribute, int lineEndContext)
  : attr(attribute), ctx(lineEndContext) {
  items.setAutoDelete(true);
}


GenHighlight::GenHighlight(const char * name) : Highlight(name) {
}


int GenHighlight::doHighlight(int ctxNum, TextLine *textLine) {
  HlContext *context;
  const QChar *str, *s1, *s2;
  QChar lastChar;
  HlItem *item;

  context = contextList[ctxNum];
  str = textLine->getString();
  lastChar = '\0';
#if 0
  s1 = str;
#else
// this causes the while loop to skip any spaces at beginning of line
// while still allowing the highlighting to continue
// On limited tests I got a 5-10% reduction in number of times in while loop
// Anything helps :)
 s1=textLine->firstNonSpace();
#endif

  while (*s1 != '\0') {
    for (item = context->items.first(); item != 0L; item = context->items.next()) {
      if (item->startEnable(lastChar)) {
        s2 = item->checkHgl(s1);
        if (s2 > s1) {
          if (item->endEnable(*s2)) { //jowenn: Here I've to change a lot
            textLine->setAttribs(item->attr,s1 - str,s2 - str);
            ctxNum = item->ctx;
            context = contextList[ctxNum];
            s1 = s2 - 1;
            goto found;
          }
        }
      }
    }
    // nothing found: set attribute of one char
    textLine->setAttribs(context->attr,s1 - str,s1 - str + 1);

    found:
    lastChar = *s1;
    s1++;
  }
  //set "end of line"-properties
  textLine->setAttr(context->attr);
  //return new context
  return context->ctx;
}

void GenHighlight::init() {
  int z;

  for (z = 0; z < nContexts; z++) contextList[z] = 0L;
  makeContextList();
}

void GenHighlight::done() {
  int z;

  for (z = 0; z < nContexts; z++) delete contextList[z];
}


CHighlight::CHighlight(const char * name) : GenHighlight(name) {
  iWildcards = "*.c";
  iMimetypes = "text/x-c-src";
}

CHighlight::~CHighlight() {
}

void CHighlight::createItemData(ItemDataList &list) {

  list.append(new ItemData(I18N_NOOP("Normal Text" ),dsNormal));
  list.append(new ItemData(I18N_NOOP("Keyword"     ),dsKeyword));
  list.append(new ItemData(I18N_NOOP("Data Type"   ),dsDataType));
  list.append(new ItemData(I18N_NOOP("Decimal"     ),dsDecVal));
  list.append(new ItemData(I18N_NOOP("Octal"       ),dsBaseN));
  list.append(new ItemData(I18N_NOOP("Hex"         ),dsBaseN));
  list.append(new ItemData(I18N_NOOP("Float"       ),dsFloat));
  list.append(new ItemData(I18N_NOOP("Char"        ),dsChar));
  list.append(new ItemData(I18N_NOOP("String"      ),dsString));
  list.append(new ItemData(I18N_NOOP("String Char" ),dsChar));
  list.append(new ItemData(I18N_NOOP("Comment"     ),dsComment));
  list.append(new ItemData(I18N_NOOP("Symbol"      ),dsNormal));
  list.append(new ItemData(I18N_NOOP("Preprocessor"),dsOthers));
  list.append(new ItemData(I18N_NOOP("Prep. Lib"   ),dsOthers,Qt::darkYellow,Qt::yellow,false,false));

}

void CHighlight::makeContextList() {
  HlContext *c;
  HlKeyword *keyword, *dataType;

  //normal context
  contextList[0] = c = new HlContext(dsNormal,0);
  c->items.append(keyword = new HlKeyword(dsKeyword,0));
  c->items.append(dataType = new HlKeyword(dsDataType,0));
  c->items.append(new HlCFloat(6,0));
  c->items.append(new HlCOct(4,0));
  c->items.append(new HlCHex(5,0));
  c->items.append(new HlCInt(3,0));
  c->items.append(new HlCChar(7,0));
  c->items.append(new HlCharDetect(8,1,'"'));
  c->items.append(new Hl2CharDetect(10,2, '/', '/'));
  c->items.append(new Hl2CharDetect(10,3, '/', '*'));
  c->items.append(new HlCSymbol(11,0));
  c->items.append(new HlCPrep(12,4));
  //string context
  contextList[1] = c = new HlContext(8,0);
  c->items.append(new HlLineContinue(8,6));
  c->items.append(new HlCStringChar(9,1));
  c->items.append(new HlCharDetect(8,0,'"'));
  //one line comment context
  contextList[2] = new HlContext(10,0);
  //multi line comment context
  contextList[3] = c = new HlContext(10,3);
  c->items.append(new Hl2CharDetect(10,0, '*', '/'));
  //preprocessor context
  contextList[4] = c = new HlContext(12,0);
  c->items.append(new HlLineContinue(12,7));
  c->items.append(new HlRangeDetect(13,4, '\"', '\"'));
  c->items.append(new HlRangeDetect(13,4, '<', '>'));
  c->items.append(new Hl2CharDetect(10,2, '/', '/'));
  c->items.append(new Hl2CharDetect(10,5, '/', '*'));
  //preprocessor multiline comment context
  contextList[5] = c = new HlContext(10,5);
  c->items.append(new Hl2CharDetect(10,4, '*', '/'));
  //string line continue
  contextList[6] = new HlContext(0,1);
  //preprocessor string line continue
  contextList[7] = new HlContext(0,4);

  setKeywords(keyword, dataType);
//  keyword->addList(HlManager::self()->syntax->finddata("C","keyword"));
//  dataType->addList(HlManager::self()->syntax->finddata("C","type"));

}
void CHighlight::setKeywords(HlKeyword *keyword, HlKeyword *dataType) {

  keyword->addList(HlManager::self()->syntax->finddata("C","keyword"));
  dataType->addList(HlManager::self()->syntax->finddata("C","type"));
}

CppHighlight::CppHighlight(const char * name) : CHighlight(name) {

  iWildcards = "*.cpp;*.cc;*.C;*.h";
  iMimetypes = "text/x-c++-src;text/x-c++-hdr;text/x-c-hdr;text/x-c++-src";
}

CppHighlight::~CppHighlight() {
}

void CppHighlight::setKeywords(HlKeyword *keyword, HlKeyword *dataType) {
  keyword->addList(HlManager::self()->syntax->finddata("C","keyword"));
  dataType->addList(HlManager::self()->syntax->finddata("C","type"));
  keyword->addList(HlManager::self()->syntax->finddata("C++","keyword"));
  dataType->addList(HlManager::self()->syntax->finddata("C++","type"));
//  kdDebug() << "Types " << dataType->getList().count() <<" Keywords " << keyword->getList().count() << endl;
}

ObjcHighlight::ObjcHighlight(const char * name) : CHighlight(name) {
  iWildcards = "*.m;*.h";
  iMimetypes = "text/x-objc-src;text/x-c-hdr";
}

ObjcHighlight::~ObjcHighlight() {
}

void ObjcHighlight::makeContextList() {
  HlContext *c;

  CHighlight::makeContextList();
  c = contextList[0];
  c->items.append(new Hl2CharDetect(8,1,'@','"'));
}
// UNTESTED
void ObjcHighlight::setKeywords(HlKeyword *keyword, HlKeyword *dataType) {

 keyword->addList(HlManager::self()->syntax->finddata("C","keyword"));
 dataType->addList(HlManager::self()->syntax->finddata("C","type"));
 keyword->addList(HlManager::self()->syntax->finddata("Objective-C","keyword"));
 dataType->addList(HlManager::self()->syntax->finddata("Objective-C","type"));

}

Hl2CharDetect::Hl2CharDetect(int attribute, int context, const QChar *s)
  : HlItem(attribute,context) {
  sChar1 = s[0];
  sChar2 = s[1];
}

HlCaseInsensitiveKeyword::HlCaseInsensitiveKeyword(int attribute, int context)
  : HlKeyword(attribute,context) {
// make dictionary case insensitive
  QDict<char> dict(113,false);
  Dict=dict;
}

HlCaseInsensitiveKeyword::~HlCaseInsensitiveKeyword() {
}

HlPHex::HlPHex(int attribute,int context)
  : HlItemWw(attribute,context){
}

const QChar *HlPHex::checkHgl(const QChar *str)
{
  const QChar *s;
  if(str[0] == '$') {
  str=str+1;
  s=str;
  while (s->isDigit() || ((*s&0xdf) >= 'A' && (*s&0xdf) <= 'F')) s++;
  if(s > str) return s;
  }
	return 0L;
}
void HlCaseInsensitiveKeyword::addList(const QStringList& lst)
{
 words+=lst;
 for(uint i=0;i<lst.count();i++)
	Dict.insert(lst[i].lower(),"dummy");
}
void HlCaseInsensitiveKeyword::addList(const char **list)
{
  while (*list) {
    words.append(*list);
    Dict.insert(QString(*list).lower(),"dummy");
    list++;
  }
}
const QChar *HlCaseInsensitiveKeyword::checkHgl(const QChar *s)
{
  const QChar *s2=s;
  if(*s2=='\0') return 0L;
  while( !ustrchr("!%&()*+,-./:;<=>?[]^{|}~ ", *s2) && *s2 != '\0') s2++;
// oops didn't increment s2 why do anything else ?
  if(s2 == s) return 0L;
  QString lookup=QString(s,s2-s)+QString::null;
  return Dict[lookup.lower()] ? s2 : 0L;
}

/*
   Not really tested but I assume it will work
*/
const char *HlCaseInsensitiveKeyword::checkHgl(const char *s) {
#if 0
  int z, count;
  QString word;

  count = words.count();
  for (z = 0; z < count; z++) {
    word = *words.at(z);
    if (strncasecmp(s,word.latin1(),word.length()) == 0) {
      return s + word.length();
    }
  }
  return 0L;
#else
// if s is in dictionary then return s+strlen(s)
   return Dict[s] ? s+strlen(s) : 0L;
#endif
}

PascalHighlight::PascalHighlight(const char *name) : CHighlight(name) {
  iWildcards = "*.pp;*.pas;*.inc";
  iMimetypes = "text/x-pascal-src";
}

PascalHighlight::~PascalHighlight() {
}

void PascalHighlight::createItemData(ItemDataList &list) {
  list.append(new ItemData("Normal Text",dsNormal));   // 0
  list.append(new ItemData("Keyword",dsKeyword));      // 1
  list.append(new ItemData("Data Type",dsDataType));   // 2
  list.append(new ItemData("Number",dsDecVal));        // 3
	list.append(new ItemData("Hex",dsBaseN));				     // 4			
  list.append(new ItemData("String",dsString));        // 5
  list.append(new ItemData("Directive",dsOthers));     // 6
  list.append(new ItemData("Comment",dsComment));      // 7
}

void PascalHighlight::makeContextList() {
  HlContext *c;
  HlKeyword *keyword, *dataType;

  contextList[0] = c = new HlContext(dsNormal,0);
    c->items.append(keyword = new HlCaseInsensitiveKeyword(dsKeyword,0));
    c->items.append(dataType = new HlCaseInsensitiveKeyword(dsDataType,0));
    c->items.append(new HlFloat(dsDecVal,0));
    c->items.append(new HlInt(dsDecVal,0));
    c->items.append(new HlPHex(dsBaseN,0));
    c->items.append(new HlCharDetect(5,1,'\''));
    c->items.append(new HlStringDetect(6,2,"(*$"));
    c->items.append(new Hl2CharDetect(6,3,(QChar*)"{$"));
    c->items.append(new Hl2CharDetect(7,4,(QChar*) "(*"));
    c->items.append(new HlCharDetect(7,5,'{'));
    c->items.append(new Hl2CharDetect(7,6,'/','/'));
  // string context
  contextList[1] = c = new HlContext(5,0);
    c->items.append(new HlCharDetect(5,0,'\''));
  // TODO: detect '''' or 'Holger''s Jokes are silly'

  // (*$ directive context
  contextList[2] = c = new HlContext(6,2);
    c->items.append(new Hl2CharDetect(6,0,(QChar*)"*)"));
  // {$ directive context
  contextList[3] = c = new HlContext(6,3);
    c->items.append(new HlCharDetect(6,0,'}'));
  // (* comment context
  contextList[4] = c = new HlContext(7,4);
    c->items.append(new Hl2CharDetect(7,0,(QChar*)"*)"));
  // { comment context
  contextList[5] = c = new HlContext(7,5);
    c->items.append(new HlCharDetect(7,0,'}'));
  // one line context
  contextList[6] = c = new HlContext(7,0);

  setKeywords(keyword,dataType);

}
void PascalHighlight::setKeywords(HlKeyword *keyword, HlKeyword *dataType)
{
  keyword->addList(HlManager::self()->syntax->finddata("Pascal","keyword"));
  dataType->addList(HlManager::self()->syntax->finddata("Pascal","type"));
}


PovrayHighlight::PovrayHighlight(const char *name) : CHighlight(name) {
  iWildcards = "*.pov;*.inc";
  iMimetypes = "text/x-povray";
}
 
PovrayHighlight::~PovrayHighlight() {
}

 void PovrayHighlight::createItemData(ItemDataList &list) {
  list.append(new ItemData("Normal Text",dsNormal));  //0
  list.append(new ItemData("Keyword",dsKeyword));     //1
  list.append(new ItemData("Number",dsDecVal));       //2
  list.append(new ItemData("String",dsString));       //3
  list.append(new ItemData("Comment",dsComment));     //4
  list.append(new ItemData("Symbol",dsNormal));       //5
}

void PovrayHighlight::makeContextList() {
  HlContext *c;
  HlKeyword *keyword, *dataType;
 
  // normal context
  contextList[0] = c = new HlContext(dsNormal,0);
  c->items.append(keyword = new HlKeyword(dsKeyword,0));
  c->items.append(new HlCFloat(2,0));
  c->items.append(new HlCInt(2,0));
  c->items.append(new HlCChar(3,0));
  c->items.append(new HlCharDetect(3,1,'"'));
  c->items.append(new Hl2CharDetect(4,2, '/', '/'));
  c->items.append(new Hl2CharDetect(4,3, '/', '*'));
  c->items.append(new HlCSymbol(5,0));
  //string context
  contextList[1] = c = new HlContext(3,0);
  c->items.append(new HlLineContinue(3,4));
  c->items.append(new HlCStringChar(3,1));
  c->items.append(new HlCharDetect(3,0,'"'));
  //one line comment context
  contextList[2] = new HlContext(4,0);
  //multi line comment context
  contextList[3] = c = new HlContext(4,3);
  c->items.append(new Hl2CharDetect(4,0, '*', '/'));
  //string line continue
  contextList[4] = new HlContext(0,1);
 
  setKeywords(keyword,dataType);
}
 
void PovrayHighlight::setKeywords(HlKeyword *keyword, HlKeyword *dataType)
{
  keyword->addList(HlManager::self()->syntax->finddata("Povray","keyword"));
}

IdlHighlight::IdlHighlight(const char * name) : CHighlight(name) {
  iWildcards = "*.idl";
  iMimetypes = "text/x-idl-src";
}


IdlHighlight::~IdlHighlight() {
}

void IdlHighlight::setKeywords(HlKeyword *keyword, HlKeyword *dataType) {

 keyword->addList(HlManager::self()->syntax->finddata("IDL","keyword"));
 dataType->addList(HlManager::self()->syntax->finddata("IDL","type"));
}

JavaHighlight::JavaHighlight(const char * name) : CHighlight(name) {
  iWildcards = "*.java";
  iMimetypes = "text/x-java-src";
}

JavaHighlight::~JavaHighlight() {
}

// UNTESTED
void JavaHighlight::setKeywords(HlKeyword *keyword, HlKeyword *dataType)
{
 keyword->addList(HlManager::self()->syntax->finddata("Java","keyword"));
 dataType->addList(HlManager::self()->syntax->finddata("Java","type"));
}


HtmlHighlight::HtmlHighlight(const char * name) : GenHighlight(name) {
  iWildcards = "*.html;*.htm;*.docbook;*.xml;*.sgml";
  iMimetypes = "text/html;text/sgml;text/xml;text/book";
}

HtmlHighlight::~HtmlHighlight() {
}

void HtmlHighlight::createItemData(ItemDataList &list) {

  list.append(new ItemData(I18N_NOOP("Normal Text"),dsNormal));
  list.append(new ItemData(I18N_NOOP("Char"       ),dsChar,Qt::darkGreen,Qt::green,false,false));
  list.append(new ItemData(I18N_NOOP("Comment"    ),dsComment));
  list.append(new ItemData(I18N_NOOP("Tag Text"   ),dsOthers,Qt::black,Qt::white,true,false));
  list.append(new ItemData(I18N_NOOP("Tag"        ),dsKeyword,Qt::darkMagenta,Qt::magenta,true,false));
  list.append(new ItemData(I18N_NOOP("Tag Value"  ),dsDecVal,Qt::darkCyan,Qt::cyan,false,false));
}

void HtmlHighlight::makeContextList() {
  HlContext *c;

  contextList[0] = c = new HlContext(0,0);
    c->items.append(new HlRangeDetect(1,0, '&', ';'));
    c->items.append(new HlStringDetect(2,1,"<!--"));
    c->items.append(new HlStringDetect(2,2,"<COMMENT>"));
    c->items.append(new HlCharDetect(3,3,'<'));
  contextList[1] = c = new HlContext(2,1);
    c->items.append(new HlStringDetect(2,0,"-->"));
  contextList[2] = c = new HlContext(2,2);
    c->items.append(new HlStringDetect(2,0,"</COMMENT>"));
  contextList[3] = c = new HlContext(3,3);
    c->items.append(new HlHtmlTag(4,3));
    c->items.append(new HlHtmlValue(5,3));
    c->items.append(new HlCharDetect(3,0,'>'));
}


BashHighlight::BashHighlight(const char * name) : GenHighlight(name) {
//  iWildcards = "";
  iMimetypes = "text/x-shellscript";
}



BashHighlight::~BashHighlight() {
}

void BashHighlight::createItemData(ItemDataList &list) {

  list.append(new ItemData(I18N_NOOP("Normal Text" ),dsNormal));
  list.append(new ItemData(I18N_NOOP("Keyword"     ),dsKeyword));
  list.append(new ItemData(I18N_NOOP("Integer"     ),dsDecVal));
  list.append(new ItemData(I18N_NOOP("String"      ),dsString));
  list.append(new ItemData(I18N_NOOP("Substitution"),dsOthers));//darkCyan,cyan,false,false);
  list.append(new ItemData(I18N_NOOP("Comment"     ),dsComment));
}

void BashHighlight::makeContextList() {
  HlContext *c;
  HlKeyword *keyword;

  contextList[0] = c = new HlContext(0,0);
    c->items.append(keyword = new HlKeyword(1,0));
    c->items.append(new HlInt(2,0));
    c->items.append(new HlCharDetect(3,1,'"'));
    c->items.append(new HlCharDetect(4,2,'`'));
    c->items.append(new HlShellComment(5,3));
  contextList[1] = c = new HlContext(3,0);
    c->items.append(new HlCharDetect(3,0,'"'));
  contextList[2] = c = new HlContext(4,0);
    c->items.append(new HlCharDetect(4,0,'`'));
  contextList[3] = new HlContext(5,0);

  keyword->addList(HlManager::self()->syntax->finddata("Bash","keyword"));
}


ModulaHighlight::ModulaHighlight(const char * name) : GenHighlight(name) {
  iWildcards = "*.md;*.mi";
  iMimetypes = "text/x-modula-2-src";
}

ModulaHighlight::~ModulaHighlight() {
}

void ModulaHighlight::createItemData(ItemDataList &list) {

  list.append(new ItemData(I18N_NOOP("Normal Text"),dsNormal));
  list.append(new ItemData(I18N_NOOP("Keyword"    ),dsKeyword));
  list.append(new ItemData(I18N_NOOP("Decimal"    ),dsDecVal));
  list.append(new ItemData(I18N_NOOP("Hex"        ),dsBaseN));
  list.append(new ItemData(I18N_NOOP("Float"      ),dsFloat));
  list.append(new ItemData(I18N_NOOP("String"     ),dsString));
  list.append(new ItemData(I18N_NOOP("Comment"    ),dsComment));
}
// UNTESTED
void ModulaHighlight::makeContextList() {
  HlContext *c;
  HlKeyword *keyword;
  contextList[0] = c = new HlContext(0,0);
    c->items.append(keyword = new HlKeyword(1,0));
    c->items.append(new HlFloat(4,0));
    c->items.append(new HlMHex(3,0));
    c->items.append(new HlInt(2,0));
    c->items.append(new HlCharDetect(5,1,'"'));
    c->items.append(new Hl2CharDetect(6,2, '(', '*'));
  contextList[1] = c = new HlContext(5,0);
    c->items.append(new HlCharDetect(5,0,'"'));
  contextList[2] = c = new HlContext(6,2);
    c->items.append(new Hl2CharDetect(6,0, '*', ')'));

  keyword->addList(HlManager::self()->syntax->finddata("Modula","keyword"));
}


AdaHighlight::AdaHighlight(const char * name) : GenHighlight(name) {
  iWildcards = "*.a";
  iMimetypes = "text/x-ada-src";
}

AdaHighlight::~AdaHighlight() {
}

void AdaHighlight::createItemData(ItemDataList &list) {

  list.append(new ItemData(I18N_NOOP("Normal Text"),dsNormal));
  list.append(new ItemData(I18N_NOOP("Keyword"    ),dsKeyword));
  list.append(new ItemData(I18N_NOOP("Decimal"    ),dsDecVal));
  list.append(new ItemData(I18N_NOOP("Base-N"     ),dsBaseN));
  list.append(new ItemData(I18N_NOOP("Float"      ),dsFloat));
  list.append(new ItemData(I18N_NOOP("Char"       ),dsChar));
  list.append(new ItemData(I18N_NOOP("String"     ),dsString));
  list.append(new ItemData(I18N_NOOP("Comment"    ),dsComment));
}
// UNTESTED
void AdaHighlight::makeContextList() {
  HlContext *c;
  HlKeyword *keyword;

  contextList[0] = c = new HlContext(0,0);
    c->items.append(keyword = new HlKeyword(1,0));
    c->items.append(new HlAdaBaseN(3,0));
    c->items.append(new HlAdaFloat(4,0));
    c->items.append(new HlAdaDec(2,0));
    c->items.append(new HlAdaChar(5,0));
    c->items.append(new HlCharDetect(6,1,'"'));
    c->items.append(new Hl2CharDetect(7,2, '-', '-'));
  contextList[1] = c = new HlContext(6,0);
    c->items.append(new HlCharDetect(6,0,'"'));
  contextList[2] = c = new HlContext(7,0);

//  HlManager *manager=HlManager::self();
  keyword->addList(HlManager::self()->syntax->finddata("C","keyword"));
}


PythonHighlight::PythonHighlight(const char * name) : GenHighlight(name) {
  iWildcards = "*.py";
  iMimetypes = "text/x-python-src";
}

PythonHighlight::~PythonHighlight() {
}

void PythonHighlight::createItemData(ItemDataList &list) {

  list.append(new ItemData(I18N_NOOP("Normal Text"),dsNormal));
  list.append(new ItemData(I18N_NOOP("Keyword"    ),dsKeyword));
  list.append(new ItemData(I18N_NOOP("Decimal"    ),dsDecVal));
  list.append(new ItemData(I18N_NOOP("Octal"      ),dsBaseN));
  list.append(new ItemData(I18N_NOOP("Hex"        ),dsBaseN));
  list.append(new ItemData(I18N_NOOP("Float"      ),dsFloat));
  list.append(new ItemData(I18N_NOOP("Char"       ),dsChar));
  list.append(new ItemData(I18N_NOOP("String"     ),dsString));
  list.append(new ItemData(I18N_NOOP("String Char"),dsChar));
  list.append(new ItemData(I18N_NOOP("Comment"    ),dsComment));
}

void PythonHighlight::makeContextList() {
  HlContext *c;
  HlKeyword *keyword;

  //note that a C octal has to be detected before an int and """ before "
  contextList[0] = c = new HlContext(0,0);
    c->items.append(keyword = new HlKeyword(1,0));
    c->items.append(new HlCOct(3,0));
    c->items.append(new HlInt(2,0));
    c->items.append(new HlCHex(4,0));
    c->items.append(new HlFloat(5,0));
    c->items.append(new HlCChar(6,0));
    c->items.append(new HlStringDetect(7,3,"\"\"\""));

    c->items.append(new HlStringDetect(7,4,"\'\'\'"));
    c->items.append(new HlCharDetect(7,1,'"'));
    c->items.append(new HlCharDetect(7,2,'\''));
    c->items.append(new HlCharDetect(9,5,'#'));
  contextList[1] = c = new HlContext(7,0);
    c->items.append(new HlLineContinue(7,6));
    c->items.append(new HlCStringChar(8,1));
    c->items.append(new HlCharDetect(7,0,'"'));
  contextList[2] = c = new HlContext(7,0);
    c->items.append(new HlLineContinue(7,7));
    c->items.append(new HlCStringChar(8,2));
    c->items.append(new HlCharDetect(7,0,'\''));
  contextList[3] = c = new HlContext(7,3);
    c->items.append(new HlStringDetect(7,0,"\"\"\""));
  contextList[4] = c = new HlContext(7,4);
    c->items.append(new HlStringDetect(7,0,"\'\'\'"));
  contextList[5] = new HlContext(9,0);
  contextList[6] = new HlContext(0,1);
  contextList[7] = new HlContext(0,2);

  keyword->addList(HlManager::self()->syntax->finddata("Python","keyword"));
//  dataType->addList(HlManager::self()->syntax->finddata("Python","type"));
}

PerlHighlight::PerlHighlight(const char * name) : Highlight(name) {
  iWildcards = "";
  iMimetypes = "application/x-perl";
}

void PerlHighlight::createItemData(ItemDataList &list) {

  list.append(new ItemData(I18N_NOOP("Normal Text"),dsNormal));
  list.append(new ItemData(I18N_NOOP("Keyword"    ),dsKeyword));
  list.append(new ItemData(I18N_NOOP("Variable"   ),dsDecVal));
  list.append(new ItemData(I18N_NOOP("Operator"   ),dsOthers));
  list.append(new ItemData(I18N_NOOP("String"     ),dsString));
  list.append(new ItemData(I18N_NOOP("String Char"),dsChar));
  list.append(new ItemData(I18N_NOOP("Comment"    ),dsComment));
  list.append(new ItemData(I18N_NOOP("Pod"        ),dsOthers, Qt::darkYellow, Qt::yellow, false, true));
}



/*
hardcoded perl highlight

Op Customary  Generic     Meaning    Interpolates         Modifiers
1     ''       q{}       Literal         no
2     ""      qq{}       Literal         yes
3     ``      qx{}       Command         yes (no for ')
4             qw{}      Word list        no
5     //       m{}    Pattern match      yes (no for ')   cgimosx
6              s{}{}   Substitution      yes (no for ')   egimosx
7             tr{}{}   Translation       no               cds
7              y{}{}   Translation       no               cds
*/
int PerlHighlight::doHighlight(int ctxNum, TextLine *textLine) {
  static const char *opList[] = {"q", "qq", "qx", "qw", "m", "s", "tr", "y"};
  static int opLenList[] = {1, 2, 2, 2, 1, 1, 2, 1}; // length of strings in opList
  QChar delimiter;
  bool division;
  int op, argCount;
  bool interpolating, brackets, pod;

  const QChar *str, *s, *s2;
  QChar lastChar;
  bool lastWw;
  int pos, z, l;

  //extract some states out of the context number
  delimiter = QChar(ctxNum >> 9);
  division = ctxNum & 256;
  op = (ctxNum >> 5) & 7;
  argCount = (ctxNum >> 3) & 3;
  interpolating = !(ctxNum & 4);
  brackets = ctxNum & 2;
  pod = ctxNum & 1;

  //current line to process
  str = textLine->getString();
  //last caracter and its whole word check status
  lastChar = ' ';
  lastWw = true;

  s = str;

  //match pod documentation tags
  if (*s == '=') {
    s++;
    pod = true;
    if (ucmp(s, "cut", 3)) {
      pod = false;
      s += 3;
      textLine->setAttribs(7, 0, 4);
    }
  }
  if (pod) {
    textLine->setAttribs(7, 0, textLine->length());
    textLine->setAttr(7);
    goto finished;
  }
  while (*s) {
    pos = s - str;
    if (op == 0 && lastWw) {
      //match keyword
      s2 = keyword->checkHgl(s);
      if (s2 && !isInWord(*s2)) {
        s = s2;
        textLine->setAttribs(1, pos, s - str);
        goto newContext;
      }
      //match perl operator
      if (lastChar != '-') {
        for (z = 0; z < 8; z++) {
          l = opLenList[z];
          if (ucmp(s, opList[z], l) && !isInWord(s[l])) {
            //operator found
            op = z;            // generate op number (1 to 7)
            if (op < 7) op++;
            argCount = (op >= 6) ? 2 : 1; // number of arguments
            s += l;
            textLine->setAttribs(3, pos, pos + l);
            goto newContext;
          }
        }
      }
      //match customary
      if (*s == '\'') {
        op = 1;
        interpolating = false;
      }
      if (*s == '"') {
        op = 2;
      }
      if (*s == '`') {
        op = 3;
      }
      if (!division && *s == '/') { // don't take it if / is division
        op = 5;
      }
      if (op != 0) {
        delimiter = *s;
        s++;
        argCount = 1;
        textLine->setAttribs(3, pos, pos + 1);
        goto newContext;
      }
    }
    if (delimiter == '\0') { //not in string
      // match comment
      if (lastWw && *s == '#') {
        textLine->setAttribs(6, pos, textLine->length());
        textLine->setAttr(6);
        goto finished;
      }
      // match delimiter
      if (op != 0 && !s->isSpace()) {
        delimiter = *s;
        if (delimiter == '(') {
          delimiter = ')';
          brackets = true;
        }
        if (delimiter == '<') {
          delimiter = '>';
          brackets = true;
        }
        if (delimiter == '[') {
          delimiter = ']';
          brackets = true;
        }
        if (delimiter == '{') {
          delimiter = '}';
          brackets = true;
        }
        s++;
        if (op == 1 || op == 4 || op == 7 || (delimiter == '\'' && op != 2))
          interpolating = false;
        textLine->setAttribs(3, pos, pos + 1);
        goto newContext;
      }
      // match bind operator or command end
      if (*s == '~' || *s == ';') {
        division = false; // pattern matches with / now allowed
      }
    }
    if (interpolating) {
      // match variable
      if (*s == '$' || *s == '@' || *s == '%') {
        s2 = s;
        do {
          s2++;
        } while ((isInWord(*s2) || *s2 == '#') && *s2 != delimiter);
        if (s2 - s > 1) {
          // variable found
          s = s2;
          textLine->setAttribs(2, pos, s2 - str);
          division = true; // division / or /= may follow
          goto newContext;
        }
      }
      // match special variables
      if (s[0] == '$' && s[1] != '\0' && s[1] != delimiter) {
        if (ustrchr("&`'+*./|,\\;#%=-~^:?!@$<>()[]", s[1])) {
          // special variable found
          s += 2;
          textLine->setAttribs(2, pos, pos + 2);
          division = true; // division / or /= may follow
          goto newContext;
        }
      }
    }
    if (delimiter != '\0') { //in string
      //match escaped char
      if (interpolating) {
        if (*s == '\\' && s[1] != '\0') {
          s++;
          s2 = checkCharHexOct(s);
          if (s2) s = s2; else s++;
          textLine->setAttribs(5, pos, s - str);
          goto newContext;
        }
      }
      //match string end
      if (delimiter == *s) {
        s++;
        argCount--;
        if (argCount < 1) {
          //match operator modifiers
          if (op == 5) while (*s && ustrchr("cgimosx", *s)) s++;
          if (op == 6) while (*s && ustrchr("egimosx", *s)) s++;
          if (op == 7) while (*s && ustrchr("cds", *s)) s++;
          op = 0;
        }
        textLine->setAttribs(3, pos, s - str);
        if (brackets || op == 0) {
          interpolating = true;
          delimiter = '\0'; //string end delimiter = '\0' means "not in string"
          brackets = false;
        }
      } else {
        //highlight a ordinary character in string
        s++;
        textLine->setAttribs(4, pos, pos + 1);
      }
      goto newContext;
    }
    s++;
    textLine->setAttribs(0, pos, pos + 1);
    newContext:
    lastChar = s[-1];
    lastWw = !isInWord(lastChar);
  }
  textLine->setAttr(0);
  finished:

  //compose new context number
  ctxNum = delimiter.unicode() << 9;
  if (division) ctxNum |= 256;
  ctxNum |= op << 5;
  ctxNum |= argCount << 3;
  if (!interpolating) ctxNum |= 4;
  if (brackets) ctxNum |= 2;
  if (pod) ctxNum |= 1;
  return ctxNum;
  //method will be called again if there are more lines to highlight
}

void PerlHighlight::init() {
  keyword = new HlKeyword(0,0);

  keyword->addList(HlManager::self()->syntax->finddata("Perl","keyword"));
}

void PerlHighlight::done() {
  delete keyword;
}

SatherHighlight::SatherHighlight(const char * name) : GenHighlight(name) {
  iWildcards = "*.sa";
  iMimetypes = "text/x-sather-src";
}

SatherHighlight::~SatherHighlight() {
}

void SatherHighlight::createItemData(ItemDataList &list) {

  list.append(new ItemData(I18N_NOOP("Normal Text"        ),dsNormal)); // 0
  list.append(new ItemData(I18N_NOOP("Keyword"            ),dsKeyword));// 1
  list.append(new ItemData(I18N_NOOP("Special Classname"  ), dsNormal));// 2
  list.append(new ItemData(I18N_NOOP("Classname"          ),dsNormal)); // 3
  list.append(new ItemData(I18N_NOOP("Special Featurename"),dsOthers)); // 4
  list.append(new ItemData(I18N_NOOP("Identifier"         ),dsOthers)); // 5
  list.append(new ItemData(I18N_NOOP("Decimal"            ),dsDecVal)); // 6
  list.append(new ItemData(I18N_NOOP("Base-N"             ),dsBaseN));  // 7
  list.append(new ItemData(I18N_NOOP("Float"              ),dsFloat));  // 8
  list.append(new ItemData(I18N_NOOP("Char"               ),dsChar));   // 9
  list.append(new ItemData(I18N_NOOP("String"             ),dsString)); // 10
  list.append(new ItemData(I18N_NOOP("Comment"            ),dsComment));// 11
}

// UNTESTED
void SatherHighlight::makeContextList() {
  HlContext *c;
  HlKeyword *keyword,*spec_class,*spec_feat;

  //Normal Context
  contextList[0] = c = new HlContext(0,0);
    c->items.append(keyword = new HlKeyword(1,0));
    c->items.append(spec_class = new HlKeyword(2,0));
    c->items.append(new HlSatherClassname(3,0));
    c->items.append(spec_feat = new HlKeyword(4,0));
    c->items.append(new HlSatherIdent(5,0));
    c->items.append(new HlSatherFloat(8,0)); // check float before int
    c->items.append(new HlSatherBaseN(7,0));
    c->items.append(new HlSatherDec(6,0));
    c->items.append(new HlSatherChar(9,0));
    c->items.append(new HlSatherString(10,0));
    c->items.append(new Hl2CharDetect(11,1, '-', '-'));
  //Comment Context
  contextList[1] = c = new HlContext(11,0);

// I combined spec_class and spec_feat in XML file
  keyword->addList(HlManager::self()->syntax->finddata("Sather","keyword"));
  spec_class->addList(HlManager::self()->syntax->finddata("Sather","type"));
}


KBasicHighlight::KBasicHighlight(const char *name) : GenHighlight(name)
{
  iWildcards = "*.kbasic";
  iMimetypes = "text/x-kbasic-src";
}

KBasicHighlight::~KBasicHighlight()
{
}


void KBasicHighlight::createItemData(ItemDataList &list)
{
  list.append(new ItemData(I18N_NOOP("Normal Text"),dsNormal));  // 0
  list.append(new ItemData(I18N_NOOP("Keyword"),dsKeyword)); // 1
  list.append(new ItemData(I18N_NOOP("Identifier"),dsOthers)); // 2
  list.append(new ItemData(I18N_NOOP("Types"),dsDataType));  // 3
  list.append(new ItemData(I18N_NOOP("String"),dsString)); // 4
  list.append(new ItemData(I18N_NOOP("Comment"),dsComment)); // 5
}

void KBasicHighlight::makeContextList()
{
  HlContext *c;
  HlKeyword *keyword, *dataType;

 //Normal Context
  contextList[0] = c = new HlContext(0,0);
  c->items.append(keyword = new HlKeyword(1,0));
  c->items.append(dataType = new HlKeyword(2,0));
  c->items.append(new HlFloat(4,0)); // check float before int
  c->items.append(new HlInt(3,0));
  c->items.append(new HlCharDetect(4,2,'\"'));
  c->items.append(new HlCharDetect(5,1, '\''));
//Comment Context
  contextList[1] = c = new HlContext(5,0);

  contextList[2] = c = new HlContext(4,0);
  c->items.append(new HlCharDetect(4,0,'"'));

  setKeywords(keyword, dataType);
}

void KBasicHighlight::setKeywords(HlKeyword *keyword, HlKeyword *dataType)
{
  keyword->addList(HlManager::self()->syntax->finddata("KBasic","keyword"));
  dataType->addList(HlManager::self()->syntax->finddata("KBasic","type"));
}

LatexHighlight::LatexHighlight(const char * name) : GenHighlight(name) {
  iWildcards = "*.tex;*.sty";
  iMimetypes = "text/x-tex";
}

LatexHighlight::~LatexHighlight() {
}

void LatexHighlight::createItemData(ItemDataList &list) {

  list.append(new ItemData(I18N_NOOP("Normal Text"), dsNormal));
  list.append(new ItemData(I18N_NOOP("Tag/Keyword"), dsKeyword));
 list.append(new ItemData(I18N_NOOP("Optional Argument"), dsKeyword));
 list.append(new ItemData(I18N_NOOP("Mandatory Argument"), dsKeyword));
 list.append(new ItemData(I18N_NOOP("Inline Maths"), dsDecVal));
  list.append(new ItemData(I18N_NOOP("Char"       ), dsChar));
  list.append(new ItemData(I18N_NOOP("Parameter"  ), dsDecVal));
  list.append(new ItemData(I18N_NOOP("Comment"    ), dsComment));
}

void LatexHighlight::makeContextList() {
  HlContext *c;
/*
  //normal context
  contextList[0] = c = new HlContext(0,0);
    c->items.append(new HlLatexTag(1,0));
    c->items.append(new HlLatexChar(2,0));
    c->items.append(new HlLatexParam(3,0));
    c->items.append(new HlCharDetect(4,1,'%'));
  //one line comment context
  contextList[1] = new HlContext(4,0);
*/

  //normal context
  contextList[0] = c = new HlContext(0,0);
    c->items.append(new HlLatexTag(1,4));
    c->items.append(new HlLatexChar(5,0));
    c->items.append(new HlCharDetect(4,2,'$'));
    c->items.append(new HlLatexParam(6,0));
    c->items.append(new HlCharDetect(7,1,'%'));
  //one line comment context
  contextList[1] = new HlContext(7,0);
  //multiline inline maths context
  contextList[2] = c = new HlContext(4,2);
    c->items.append(new HlLatexChar(4,2));
    c->items.append(new HlCharDetect(4,3,'%'));
    c->items.append(new HlCharDetect(4,0,'$'));
  //comments in math mode context
  contextList[3] = new HlContext(7,2);
  //arguments to functions context
  contextList[4] = c = new HlContext(0,0);
    c->items.append(new HlCharDetect(2,5, '['));
    c->items.append(new HlCharDetect(3,6, '{'));
    c->items.append(new HlLatexTag(1,4));
    c->items.append(new HlLatexChar(5,0));
    c->items.append(new HlCharDetect(4,2,'$'));
    c->items.append(new HlLatexParam(6,0));
    c->items.append(new HlCharDetect(7,1,'%'));
  //optional arguments to functions context
  //this is buggy because nested arguments can span lines
  //should be OK for 99% of the time
  contextList[5] = c = new HlContext(2,5);
    c->items.append(new HlRangeDetect(2,5, '[', ']'));
    c->items.append(new HlCharDetect(2,4, ']'));
  //mandatory arguments to functions context
  //this is buggy because nested arguments can span lines
  //should be OK for 99% of the time
  contextList[6] = c = new HlContext(3,6);
    c->items.append(new HlRangeDetect(3,6, '{', '}'));
    c->items.append(new HlCharDetect(3,4, '}'));
}

AutoHighlight::AutoHighlight(syntaxModeListItem *def):GenHighlight(def->name.latin1())
{
  iName = def->name;
  iWildcards = def->extension;
  iMimetypes = def->mimetype;
  casesensitive = def->casesensitive;
}

AutoHighlight::~AutoHighlight()
{
}

void AutoHighlight::setKeywords(HlKeyword *keyword, HlKeyword *dataType)
{
  if (casesensitive=="1")
  {
    keyword->addList(HlManager::self()->syntax->finddata(iName,"keyword"));
    dataType->addList(HlManager::self()->syntax->finddata(iName,"type"));
  }
}

void AutoHighlight::createItemData(ItemDataList &list)
{
  struct syntaxContextData *data;

  kdDebug()<<"In AutoHighlight::createItemData"<<endl;
  data=HlManager::self()->syntax->getGroupInfo(iName,"itemData");
  while (HlManager::self()->syntax->nextGroup(data))
    {
	list.append(new ItemData(
          HlManager::self()->syntax->groupData(data,QString("name")).latin1(),
          getDefStyleNum(HlManager::self()->syntax->groupData(data,QString("defStyleNum")))));

    }
  if (data) HlManager::self()->syntax->freeGroupInfo(data);
}

HlItem *AutoHighlight::createHlItem(struct syntaxContextData *data, int *res)
{

                QString dataname=HlManager::self()->syntax->groupItemData(data,QString("name"));
                int attr=((HlManager::self()->syntax->groupItemData(data,QString("attribute"))).toInt());
                int context=((HlManager::self()->syntax->groupItemData(data,QString("context"))).toInt());
		char chr;
                if (! HlManager::self()->syntax->groupItemData(data,QString("char")).isEmpty())
		  chr= (HlManager::self()->syntax->groupItemData(data,QString("char")).latin1())[0];
		else
                  chr=0;
		QString stringdata=HlManager::self()->syntax->groupItemData(data,QString("String"));
                char chr1;
                if (! HlManager::self()->syntax->groupItemData(data,QString("char1")).isEmpty())
		  chr1= (HlManager::self()->syntax->groupItemData(data,QString("char1")).latin1())[0];
		else
                  chr1=0;
		bool insensitive=(HlManager::self()->syntax->groupItemData(data,QString("insensitive"))==QString("TRUE"));
		*res=0;

                if (dataname=="keyword") {*res=1; return(new HlKeyword(attr,context));} else
                if (dataname=="dataType") {*res=2; return new HlKeyword(attr,context);} else
                if (dataname=="Float") return (new HlFloat(attr,context)); else
                if (dataname=="Int") return(new HlInt(attr,context)); else
                if (dataname=="CharDetect") return(new HlCharDetect(attr,context,chr)); else
                if (dataname=="2CharDetect") return(new Hl2CharDetect(attr,context,chr,chr1)); else
                if (dataname=="RangeDetect") return(new HlRangeDetect(attr,context, chr, chr1)); else
		if (dataname=="LineContinue") return(new HlLineContinue(attr,context)); else
                if (dataname=="StringDetect") return(new HlStringDetect(attr,context,stringdata,insensitive)); else
                if (dataname=="AnyChar") return(new HlAnyChar(attr,context,(char*)stringdata.latin1())); else
		  {
                    kdDebug()<<"***********************************"<<endl<<"Unknown entry for Context:"<<dataname<<endl;
                    return 0;
                  }


}

void AutoHighlight::makeContextList()
{
  HlKeyword *keyword, *dataType;
  struct syntaxContextData *data, *datasub;
  HlItem *c;

  kdDebug()<< "AutoHighlight makeContextList()"<<endl;
  data=HlManager::self()->syntax->getGroupInfo(iName,"context");
  int i=0;
  if (data)
    {
      while (HlManager::self()->syntax->nextGroup(data))
        {
	kdDebug()<< "In make Contextlist: Group"<<endl;
          contextList[i]=new HlContext(
            (HlManager::self()->syntax->groupData(data,QString("attribute"))).toInt(),
            (HlManager::self()->syntax->groupData(data,QString("lineEndContext"))).toInt());

/*            if ((i==0) && (casesensitive=="0"))
            {
               contextList[0]->items.append(keyword = new HlCaseInsensitiveKeyword(dsKeyword,0));
               contextList[0]->items.append(dataType = new HlCaseInsensitiveKeyword(dsDataType,0));
            }*/

            while (HlManager::self()->syntax->nextItem(data))
              {
		kdDebug()<< "In make Contextlist: Item:"<<endl;

		int res;
		c=createHlItem(data,&res);
		if (c)
			{
				contextList[i]->items.append(c);
				if (res==1) keyword=(HlKeyword*)c; else if (res==2) dataType=(HlKeyword*)c;

				datasub=HlManager::self()->syntax->getSubItems(data);
				bool tmpbool;
				if (tmpbool=HlManager::self()->syntax->nextItem(datasub))
					{
                                          c->subItems=new QList<HlItem>;
					  for (;tmpbool;tmpbool=HlManager::self()->syntax->nextItem(datasub))
                                            c->subItems->append(createHlItem(datasub,&res));
                                        }
				HlManager::self()->syntax->freeGroupInfo(datasub);
			}
		kdDebug()<<"Last line in loop"<<endl;
              }
          i++;
        }
      }
  kdDebug()<<"After creation loop in AutoHighlight::makeContextList"<<endl;
  HlManager::self()->syntax->freeGroupInfo(data);
  setKeywords(keyword, dataType);
  kdDebug()<<"After setKeyWords AutoHighlight::makeContextList"<<endl;

}

//--------



HlManager::HlManager() : QObject(0L)
{
  syntax = new SyntaxDocument();
  SyntaxModeList modeList = syntax->modeList();

  hlList.setAutoDelete(true);
  hlList.append(new Highlight(I18N_NOOP("Normal")));

   /* new stuff
 uint i=0;
  while (i < modeList.count())
  {
    hlList.append(new AutoHighlight(modeList.at(i)));
    i++;
  }              
 */

  hlList.append(new CHighlight(     "C"        ));
  hlList.append(new CppHighlight(   "C++"      ));
  hlList.append(new ObjcHighlight(  "Objective-C"));
  hlList.append(new JavaHighlight(  "Java"     ));
  hlList.append(new HtmlHighlight(  "HTML"     ));
  hlList.append(new BashHighlight(  "Bash"     ));
  hlList.append(new ModulaHighlight("Modula 2" ));
  hlList.append(new AdaHighlight(   "Ada"      ));
  hlList.append(new PascalHighlight("Pascal"   ));
  hlList.append(new PovrayHighlight("Povray"   ));
  hlList.append(new PythonHighlight("Python"   ));
  hlList.append(new PerlHighlight(  "Perl"     ));
  hlList.append(new SatherHighlight("Sather"   ));
  hlList.append(new KBasicHighlight("KBasic"));
  hlList.append(new LatexHighlight( "Latex"    ));
  hlList.append(new IdlHighlight("IDL"));

}

HlManager::~HlManager() {
  if(syntax) delete syntax;
}

HlManager *HlManager::self()
{
  if ( !s_pSelf )
    s_pSelf = new HlManager;
  return s_pSelf;
}

Highlight *HlManager::getHl(int n) {
  if (n < 0 || n >= (int) hlList.count()) n = 0;
  return hlList.at(n);
}

int HlManager::defaultHl() {
  KConfig *config;

  config = KWriteFactory::instance()->config();
  config->setGroup("General Options");
  return nameFind(config->readEntry("Highlight"));
}


int HlManager::nameFind(const QString &name) {
  int z;

  for (z = hlList.count() - 1; z > 0; z--) {
    if (hlList.at(z)->iName == name) break;
  }
  return z;
}

int HlManager::wildcardFind(const QString &fileName) {
  Highlight *highlight;
  int p1, p2;
  QString w;
  for (highlight = hlList.first(); highlight != 0L; highlight = hlList.next()) {
    p1 = 0;
    w = highlight->getWildcards();
    while (p1 < (int) w.length()) {
      p2 = w.find(';',p1);
      if (p2 == -1) p2 = w.length();
      if (p1 < p2) {
        QRegExp regExp(w.mid(p1,p2 - p1),true,true);
        if (regExp.match(fileName) == 0) return hlList.at();
      }
      p1 = p2 + 1;
    }
  }
  return -1;
}

int HlManager::mimeFind(const QByteArray &contents, const QString &fname)
{
/*
  // fill the detection buffer with the contents of the text
  const int HOWMANY = 1024;
  char buffer[HOWMANY];
  int number=0, len;

  for (int index=0; index<doc->lastLine(); index++)
  {
    len = doc->textLength(index);

    if (number+len > HOWMANY)
      break;

    memcpy(&buffer[number], doc->textLine(index)->getText(), len);
    number += len;
  }
*/
  // detect the mime type
  KMimeMagicResult *result;
  result = KMimeMagic::self()->findBufferFileType(contents, fname);

  Highlight *highlight;
  int p1, p2;
  QString w;

  for (highlight = hlList.first(); highlight != 0L; highlight = hlList.next())
  {
    w = highlight->getMimetypes();

    p1 = 0;
    while (p1 < (int) w.length()) {
      p2 = w.find(';',p1);
      if (p2 == -1) p2 = w.length();
      if (p1 < p2) {
        QRegExp regExp(w.mid(p1,p2 - p1),true,true);
        if (regExp.match(result->mimeType()) == 0) return hlList.at();
      }
      p1 = p2 + 1;
    }
  }

  return -1;
}

int HlManager::makeAttribs(Highlight *highlight, Attribute *a, int maxAttribs) {
  ItemStyleList defaultStyleList;
  ItemStyle *defaultStyle;
  ItemFont defaultFont;
  ItemDataList itemDataList;
  ItemData *itemData;
  int nAttribs, z;
  QFont font;

  defaultStyleList.setAutoDelete(true);
  getDefaults(defaultStyleList, defaultFont);

  itemDataList.setAutoDelete(true);
  highlight->getItemDataList(itemDataList);
  nAttribs = itemDataList.count();
  for (z = 0; z < nAttribs; z++) {
    itemData = itemDataList.at(z);
    if (itemData->defStyle) {
      // default style
      defaultStyle = defaultStyleList.at(itemData->defStyleNum);
      a[z].col = defaultStyle->col;
      a[z].selCol = defaultStyle->selCol;
      font.setBold(defaultStyle->bold);
      font.setItalic(defaultStyle->italic);
    } else {
      // custom style
      a[z].col = itemData->col;
      a[z].selCol = itemData->selCol;
      font.setBold(itemData->bold);
      font.setItalic(itemData->italic);
    }
    if (itemData->defFont) {
      font.setFamily(defaultFont.family);
      font.setPointSize(defaultFont.size);
//      KCharset(defaultFont.charset).setQFont(font);
    } else {
      font.setFamily(itemData->family);
      font.setPointSize(itemData->size);
//      KCharset(itemData->charset).setQFont(font);
    }
    a[z].setFont(font);
  }
  for (; z < maxAttribs; z++) {
    a[z].col = black;
    a[z].selCol = black;
    a[z].setFont(font);
  }
  return nAttribs;
}

int HlManager::defaultStyles() {
  return 10;
}

const char * HlManager::defaultStyleName(int n) {
  static const char *names[] = {
    I18N_NOOP("Normal"),
    I18N_NOOP("Keyword"),
    I18N_NOOP("Data Type"),
    I18N_NOOP("Decimal/Value"),
    I18N_NOOP("Base-N Integer"),
    I18N_NOOP("Floating Point"),
    I18N_NOOP("Character"),
    I18N_NOOP("String"),
    I18N_NOOP("Comment"),
    I18N_NOOP("Others")};

  return names[n];
}

void HlManager::getDefaults(ItemStyleList &list, ItemFont &font) {
  KConfig *config;
  int z;
  ItemStyle *i;
  QString s;
  QRgb col, selCol;

  list.setAutoDelete(true);
  //ItemStyle(color, selected color, bold, italic)
  list.append(new ItemStyle(black,white,false,false));     //normal
  list.append(new ItemStyle(black,white,true,false));      //keyword
  list.append(new ItemStyle(darkRed,white,false,false));   //datatype
  list.append(new ItemStyle(blue,cyan,false,false));       //decimal/value
  list.append(new ItemStyle(darkCyan,cyan,false,false));   //base n
  list.append(new ItemStyle(darkMagenta,cyan,false,false));//float
  list.append(new ItemStyle(magenta,magenta,false,false)); //char
  list.append(new ItemStyle(red,red,false,false));         //string
  list.append(new ItemStyle(darkGray,gray,false,true));    //comment
  list.append(new ItemStyle(darkGreen,green,false,false)); //others

  config = KWriteFactory::instance()->config();
  config->setGroup("Default Item Styles");
  for (z = 0; z < defaultStyles(); z++) {
    i = list.at(z);
    s = config->readEntry(defaultStyleName(z));
    if (!s.isEmpty()) {
      sscanf(s.latin1(),"%X,%X,%d,%d",&col,&selCol,&i->bold,&i->italic);
      i->col.setRgb(col);
      i->selCol.setRgb(selCol);
    }
  }

  config->setGroup("Default Font");
  QFont defaultFont = KGlobalSettings::fixedFont();
  font.family = config->readEntry("Family", defaultFont.family());
//  qDebug("family == %s", font.family.ascii());
  font.size = config->readNumEntry("Size", defaultFont.pointSize());
//  qDebug("size == %d", font.size);
  font.charset = config->readEntry("Charset","ISO-8859-1");
}

void HlManager::setDefaults(ItemStyleList &list, ItemFont &font) {
  KConfig *config;
  int z;
  ItemStyle *i;
  char s[64];

  config = KWriteFactory::instance()->config();
  config->setGroup("Default Item Styles");
  for (z = 0; z < defaultStyles(); z++) {
    i = list.at(z);
    sprintf(s,"%X,%X,%d,%d",i->col.rgb(),i->selCol.rgb(),i->bold, i->italic);
    config->writeEntry(defaultStyleName(z),s);
  }

  config->setGroup("Default Font");
  config->writeEntry("Family",font.family);
  config->writeEntry("Size",font.size);
  config->writeEntry("Charset",font.charset);

  emit changed();
}


int HlManager::highlights() {
  return (int) hlList.count();
}

const char * HlManager::hlName(int n) {
  return hlList.at(n)->iName;
}

void HlManager::getHlDataList(HlDataList &list) {
  int z;

  for (z = 0; z < (int) hlList.count(); z++) {
    list.append(hlList.at(z)->getData());
  }
}

void HlManager::setHlDataList(HlDataList &list) {
  int z;

  for (z = 0; z < (int) hlList.count(); z++) {
    hlList.at(z)->setData(list.at(z));
  }
  //notify documents about changes in highlight configuration
  emit changed();
}

StyleChanger::StyleChanger( QWidget *parent )
  : QWidget(parent)
{
  QLabel *label;

  QGridLayout *glay = new QGridLayout( this, 4, 3, 0, KDialog::spacingHint() );
  CHECK_PTR(glay);
  glay->addColSpacing( 1, KDialog::spacingHint() ); // Looks better
  glay->setColStretch( 2, 10 );

  col = new KColorButton(this);
  CHECK_PTR(col);
  connect(col,SIGNAL(changed(const QColor &)),this,SLOT(changed()));
  label = new QLabel(col,i18n("Normal:"),this);
  CHECK_PTR(label);
  glay->addWidget(label,0,0);
  glay->addWidget(col,1,0);

  selCol = new KColorButton(this);
  CHECK_PTR(selCol);
  connect(selCol,SIGNAL(changed(const QColor &)),this,SLOT(changed()));
  label = new QLabel(selCol,i18n("Selected:"),this);
  CHECK_PTR(label);
  glay->addWidget(label,2,0);
  glay->addWidget(selCol,3,0);

  bold = new QCheckBox(i18n("Bold"),this);
  CHECK_PTR(bold);
  connect(bold,SIGNAL(clicked()),SLOT(changed()));
  glay->addWidget(bold,1,2);

  italic = new QCheckBox(i18n("Italic"),this);
  CHECK_PTR(italic);
  connect(italic,SIGNAL(clicked()),SLOT(changed()));
  glay->addWidget(italic,2,2);
}

void StyleChanger::setRef(ItemStyle *s) {

  style = s;
  col->setColor(style->col);
  selCol->setColor(style->selCol);
  bold->setChecked(style->bold);
  italic->setChecked(style->italic);

}

void StyleChanger::setEnabled(bool enable) {

  col->setEnabled(enable);
  selCol->setEnabled(enable);
  bold->setEnabled(enable);
  italic->setEnabled(enable);
}

void StyleChanger::changed() {

  if (style) {
    style->col = col->color();
    style->selCol = selCol->color();
    style->bold = bold->isChecked();
    style->italic = italic->isChecked();
  }
}





FontChanger::FontChanger( QWidget *parent )
  : QWidget(parent)
{
  QLabel *label;
  QStringList fontList;

  QVBoxLayout *vlay = new QVBoxLayout( this, 0, KDialog::spacingHint() );
  CHECK_PTR(vlay);

  familyCombo = new QComboBox(true,this);
  CHECK_PTR(familyCombo);
  label = new QLabel( familyCombo,i18n("Family:"), this );
  CHECK_PTR(label);
  vlay->addWidget(label);
  vlay->addWidget(familyCombo);
  connect( familyCombo, SIGNAL(activated(const QString&)),
           this, SLOT(familyChanged(const QString&)));
  KFontChooser::getFontList(fontList, false);
  familyCombo->insertStringList(fontList);


  sizeCombo = new QComboBox(true,this);
  CHECK_PTR(sizeCombo);
  label = new QLabel(sizeCombo,i18n("Size:"),this);
  CHECK_PTR(label);
  vlay->addWidget(label);
  vlay->addWidget(sizeCombo);
  connect( sizeCombo, SIGNAL(activated(int)),
           this, SLOT(sizeChanged(int)) );
  for( int i=0; fontSizes[i] != 0; i++ ){
    sizeCombo->insertItem(QString().setNum(fontSizes[i]));
  }


  charsetCombo = new QComboBox(true,this);
  CHECK_PTR(charsetCombo);
  label = new QLabel(charsetCombo,i18n("Charset:"),this);
  CHECK_PTR(label);
  vlay->addWidget(label);
  vlay->addWidget(charsetCombo);
  connect( charsetCombo, SIGNAL(activated(const QString&)),
           this, SLOT(charsetChanged(const QString&)) );
}


void FontChanger::setRef(ItemFont *f) {
  int z;

  font = f;
  for (z = 0; z < (int) familyCombo->count(); z++) {
    if (font->family == familyCombo->text(z)) {
      familyCombo->setCurrentItem(z);
      goto found;
    }
  }
  font->family = familyCombo->text(0);
found:

  for (z = 0; fontSizes[z] > 0; z++) {
    if (font->size == fontSizes[z]) {
      sizeCombo->setCurrentItem(z);
      break;
    }
  }
  displayCharsets();
}

void FontChanger::familyChanged(const QString& family) {

  font->family = family;
  displayCharsets();
}

void FontChanger::sizeChanged(int n) {

  font->size = fontSizes[n];;
}

void FontChanger::charsetChanged(const QString& charset) {

  font->charset = charset;
  //KCharset(chset).setQFont(font);
}

void FontChanger::displayCharsets() {
  int z;
  QString charset;
  KCharsets *charsets;

  charsets = KGlobal::charsets();
  QStringList lst = charsets->availableCharsetNames(font->family);
//  QStrList lst = charsets->displayable(font->family);
  charsetCombo->clear();
  for(z = 0; z < (int) lst.count(); z++) {
    charset = *lst.at(z);
    charsetCombo->insertItem(charset);
    if (/*(QString)*/ font->charset == charset) charsetCombo->setCurrentItem(z);
  }
  charset = "any";
  charsetCombo->insertItem(charset);
  if (/*(QString)*/ font->charset == charset) charsetCombo->setCurrentItem(z);
}

//---------


HighlightDialog::HighlightDialog( HlManager *hlManager, ItemStyleList *styleList,
                                  ItemFont *font,
                                  HlDataList *highlightDataList,
                                  int hlNumber, QWidget *parent,
                                  const char *name, bool modal )
  :KDialogBase(KDialogBase::Tabbed, i18n("Highlight Settings"), Ok|Cancel, Ok, parent, name, modal),
   defaultItemStyleList(styleList), hlData(0L)
{

  // defaults =========================================================

  QFrame *page1 = addPage(i18n("&Defaults"));
  QGridLayout *grid = new QGridLayout(page1,2,2,0,spacingHint());

  QVGroupBox *dvbox1 = new QVGroupBox( i18n("Default Item Styles"), page1 );
  /*QLabel *label = */new QLabel( i18n("Item:"), dvbox1 );
  QComboBox *styleCombo = new QComboBox( false, dvbox1 );
  defaultStyleChanger = new StyleChanger( dvbox1 );
  for( int i = 0; i < hlManager->defaultStyles(); i++ ) {
    styleCombo->insertItem(i18n(hlManager->defaultStyleName(i)));
  }
  connect(styleCombo, SIGNAL(activated(int)), this, SLOT(defaultChanged(int)));
  grid->addWidget(dvbox1,0,0);

  QVGroupBox *dvbox2 = new QVGroupBox( i18n("Default Font"), page1 );
  defaultFontChanger = new FontChanger( dvbox2 );
  defaultFontChanger->setRef(font);
  grid->addWidget(dvbox2,0,1);

  grid->setRowStretch(1,1);
  grid->setColStretch(1,1);

  defaultChanged(0);

  // highlight modes =====================================================

  QFrame *page2 = addPage(i18n("&Highlight Modes"));
  grid = new QGridLayout(page2,3,2,0,spacingHint());

  QVGroupBox *vbox1 = new QVGroupBox( i18n("Config Select"), page2 );
  grid->addWidget(vbox1,0,0);
  QVGroupBox *vbox2 = new QVGroupBox( i18n("Item Style"), page2 );
  grid->addWidget(vbox2,1,0);
  QVGroupBox *vbox3 = new QVGroupBox( i18n("Highlight Auto Select"), page2 );
  grid->addWidget(vbox3,0,1);
  QVGroupBox *vbox4 = new QVGroupBox( i18n("Item Font"), page2 );
  grid->addWidget(vbox4,1,1);

  grid->setRowStretch(2,1);
  grid->setColStretch(1,1);

  QLabel *label = new QLabel( i18n("Highlight:"), vbox1 );
  hlCombo = new QComboBox( false, vbox1 );
  connect( hlCombo, SIGNAL(activated(int)),
           this, SLOT(hlChanged(int)) );
  for( int i = 0; i < hlManager->highlights(); i++) {
    hlCombo->insertItem(hlManager->hlName(i));
  }
  hlCombo->setCurrentItem(hlNumber);


  label = new QLabel( i18n("Item:"), vbox1 );
  itemCombo = new QComboBox( false, vbox1 );
  connect( itemCombo, SIGNAL(activated(int)), this, SLOT(itemChanged(int)) );

  label = new QLabel( i18n("File Extensions:"), vbox3 );
  wildcards  = new QLineEdit( vbox3 );
  label = new QLabel( i18n("Mime Types:"), vbox3 );
  mimetypes = new QLineEdit( vbox3 );


  styleDefault = new QCheckBox(i18n("Default"), vbox2 );
  connect(styleDefault,SIGNAL(clicked()),SLOT(changed()));
  styleChanger = new StyleChanger( vbox2 );


  fontDefault = new QCheckBox(i18n("Default"), vbox4 );
  connect(fontDefault,SIGNAL(clicked()),SLOT(changed()));
  fontChanger = new FontChanger( vbox4 );

  hlDataList = highlightDataList;
  hlChanged(hlNumber);
}


void HighlightDialog::defaultChanged(int z)
{
  defaultStyleChanger->setRef(defaultItemStyleList->at(z));
}


void HighlightDialog::hlChanged(int z)
{
  writeback();

  hlData = hlDataList->at(z);

  wildcards->setText(hlData->wildcards);
  mimetypes->setText(hlData->mimetypes);

  itemCombo->clear();
  for (ItemData *itemData = hlData->itemDataList.first(); itemData != 0L;
    itemData = hlData->itemDataList.next()) {
    itemCombo->insertItem(i18n(itemData->name));
  }

  itemChanged(0);
}

void HighlightDialog::itemChanged(int z)
{
  itemData = hlData->itemDataList.at(z);

  styleDefault->setChecked(itemData->defStyle);
  styleChanger->setRef(itemData);

  fontDefault->setChecked(itemData->defFont);
  fontChanger->setRef(itemData);
}

void HighlightDialog::changed()
{
  itemData->defStyle = styleDefault->isChecked();
  itemData->defFont = fontDefault->isChecked();
}

void HighlightDialog::writeback() {
  if (hlData) {
    hlData->wildcards = wildcards->text();
    hlData->mimetypes = mimetypes->text();
  }
}

void HighlightDialog::done(int r) {
  writeback();
  QDialog::done(r);
}

#include "highlight.moc"
