// **************************************************************************
//    begin                : Tue Aug 17 1999
//    copyright            : (C) 1999 by John Birch
//    email                : jbb@kdevelop.org
//	
//                          Adapted for ruby debugging
//                          --------------------------
//    begin                : Mon Nov 1 2004
//    copyright            : (C) 2004 by Richard Dale
//    email                : Richard_Dale@tipitina.demon.co.uk
// **************************************************************************

// **************************************************************************
//                                                                          *
//    This program is free software; you can redistribute it and/or modify  *
//    it under the terms of the GNU General Public License as published by  *
//    the Free Software Foundation; either version 2 of the License, or     *
//    (at your option) any later version.                                   *
//                                                                          *
// **************************************************************************

#include "rdbparser.h"
#include "variablewidget.h"

#include <qregexp.h>

#include <ctype.h>
#include <stdlib.h>
#include <kdebug.h>

namespace RDBDebugger
{

// **************************************************************************
// **************************************************************************
// **************************************************************************

RDBParser *RDBParser::RDBParser_ = 0;

RDBParser *RDBParser::getRDBParser()
{
  if (!RDBParser_)
    RDBParser_ = new RDBParser();

  return RDBParser_;
}

// **************************************************************************

void RDBParser::destroy()
{
    delete RDBParser_;
    RDBParser_ = 0;
}

// **************************************************************************

RDBParser::RDBParser()
{
}

// **************************************************************************

RDBParser::~RDBParser()
{
}

// **************************************************************************

void RDBParser::parseData(TrimmableItem *parent, char *buf)
{
    static const char *unknown = "?";
	
	QString		varName;
	QCString	value;
	DataType	dataType;
	int			pos;

    Q_ASSERT(parent);
    if (buf == 0 || strlen(buf) == 0) {
        return;
	}

    if (buf[0] == 0) {
        buf = (char*)unknown;
	}
		
	QRegExp var_re("\\s*([\\w@$-]+) => ([^\n]+)\n");
	QRegExp ref_re("(#<\\w+:0x[\\da-f]+)\\s*(.*)>");
	
	// Look for 'dataitem => value' pairs. For example:
	// 	a => 1
  	// 	m => #<MyClass:0x30093540 @temp={"z"=>"zed", "p"=>"pee"}, @foobar="hello">
	//
	pos = var_re.search(buf);	
	if (pos != -1) {
		while (pos != -1) {
			varName = var_re.cap(1);
			if (ref_re.search(var_re.cap(2)) != -1) {
				value = (ref_re.cap(1) + ">").latin1();
			} else {
				value = var_re.cap(2).latin1();
			}
			
	        DataType dataType = determineType(value.data());
			setItem(parent, varName, dataType, value);
			
			pos  += var_re.matchedLength();
			pos = var_re.search(buf, pos);
		}

		return;
	}
	
	// Look for a reference type which has been printed via a 'pp' command, to
	// expand its sub items. For example:
	//     #<MyClass:0x30093540
 	//		@foobar="hello",
 	//		@sleeper=#<Thread:0x3008fd18 sleep>,
 	//		@temp={"z"=>"zed", "p"=>"pee"}>
	//
	QRegExp ppref_re("(#<\\w+:0x[\\da-f]+)\n");
	QRegExp ppvalue_re("\\s*([\\w@$-]+)=([^\n]+)[,>]\n");
	
	pos = ppref_re.search(buf);
	if (pos != -1) {
		pos  += ppref_re.matchedLength();
		pos = ppvalue_re.search(buf, pos);
		
		while (pos != -1) {
			varName = ppvalue_re.cap(1);
			value = ppvalue_re.cap(2).latin1();
			dataType = determineType(value.data());
			setItem(parent, varName, dataType, value);
			
			pos += ppvalue_re.matchedLength();
			pos = ppvalue_re.search(buf, pos);
		}
		
		return;
	}
	
	// Look for a array type which has been printed via a 'pp' command, to
	// expand its sub items. For example:
	//     Array (2 element(s))
 	//		[0]="hello"
 	//		[1]=#"goodbye"
	//
	QRegExp array_re("(Array \\(\\d+ element\\(s\\)\\))\n");
	QRegExp pparray_re("\\s*([^=]+)=([^\n]+)\n");
	
	pos = array_re.search(buf);
	if (pos != -1) {
		parent->setText(ValueCol, array_re.cap(1));
		pos  += array_re.matchedLength();
		pos = pparray_re.search(buf, pos);
		
		while (pos != -1) {
			varName = pparray_re.cap(1);
			value = pparray_re.cap(2).latin1();
	        DataType dataType = determineType(value.data());
			setItem(parent, varName, dataType, value);
			
			pos += pparray_re.matchedLength();
			pos = pparray_re.search(buf, pos);
		}
	}
	
	// Look for a hash type which has been printed via a 'pp' command, to
	// expand its sub items. For example:
	//     Hash (2 element(s))
 	//		"greeting"=>"hello"
 	//		"farewell"=>"goodbye"
	//
	QRegExp hash_re("(Hash \\(\\d+ element\\(s\\)\\))\n");
	QRegExp pphash_re("\\s*([^=]+)=([^\n]+)\n");
	
	pos = hash_re.search(buf);
	if (pos != -1) {
		parent->setText(ValueCol, hash_re.cap(1));
		pos  += hash_re.matchedLength();
		pos = pphash_re.search(buf, pos);
		
		while (pos != -1) {
			varName = pphash_re.cap(1);
			value = pphash_re.cap(2).latin1();
	        DataType dataType = determineType(value.data());
			setItem(parent, varName, dataType, value);
			
			pos += pphash_re.matchedLength();
			pos = pphash_re.search(buf, pos);
		}
	}
}


// **************************************************************************

void RDBParser::setItem(TrimmableItem *parent, const QString &varName,
                        DataType dataType, const QCString &value)
{
	VarItem *item = parent->findItemWithName(varName);
    if (item == 0) {
        item = new VarItem(parent, varName, dataType);
    } else {
		// The dataType of an item can change, so update it
		item->setDataType(dataType);
	}

    switch (dataType) {
    case typeHash:
    case typeArray:
		// Don't set the name in the value column yet, as it gets
		// set when the pp command returns with the array or hash
		// expansion
        item->setCache(value);
        item->setExpandable(true);
        break;

    case typeReference:
        item->setText(ValueCol, value);
        item->setCache(value);
		// If there's a comma, there must be a list of things
		// to expand
        item->setExpandable(value.contains(','));
        break;

    case typeValue:
        item->setText(ValueCol, value);
        item->setExpandable(false);
        break;

    default:
        break;
    }
}

// **************************************************************************

DataType RDBParser::determineType(char *buf) const
{
	if (qstrncmp(buf, "#<", strlen("#<")) == 0) {
		return typeReference;
	} else if (qstrncmp(buf, "[", strlen("[")) == 0) {
		return typeArray;
	} else if (qstrncmp(buf, "{", strlen("{")) == 0) {
		return typeHash;
	} else if (qstrncmp(buf, "nil", strlen("nil")) == 0) {
//		return typeUnknown;
		return typeValue;
	} else {
		return typeValue;
	}
}


// **************************************************************************
// **************************************************************************
// **************************************************************************

}
