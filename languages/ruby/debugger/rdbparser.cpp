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

void RDBParser::parseVariables(LazyFetchItem *parent, char *buf)
{
    static const char *unknown = "?";
	
	QString		varName;
	QCString	value;
	int			pos;

    Q_ASSERT(parent);
    if (buf == 0 || strlen(buf) == 0) {
        return;
	}

    if (buf[0] == 0) {
        buf = (char*)unknown;
	}
		
	QRegExp var_re("\\s*([^\\n\\s]+) => ([^\\n]+)");
	QRegExp ref_re("(#<[^:]+:0x[\\da-f]+)\\s*([^=]*)>?");
	
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
			
	        DataType dataType = determineType((char *) var_re.cap(2).latin1());
			setItem(parent, varName, dataType, value);
			
			pos  += var_re.matchedLength();
			pos = var_re.search(buf, pos);
		}

		return;
	}
}

void RDBParser::parseExpandedVariable(LazyFetchItem *parent, char *buf)
{
	DataType	dataType;
	int			pos;
	QString		varName;
	QCString	value;
	
	// Look for a reference type which has been printed via a 'pp' command, to
	// expand its sub items. For example:
	//     #<MyClass:0x30093540
 	//		@foobar="hello",
 	//		@sleeper=#<Thread:0x3008fd18 sleep>,
 	//		@temp={"z"=>"zed", "p"=>"pee"}>
	//
	QRegExp ppref_re("(#<[^:]+:0x[\\da-f]+)([^\\n>]*)(>?)");
	QRegExp ppvalue_re("\\s*([^\\n\\s=]+)=([^\\n]+)[,>]");
	
	pos = ppref_re.search(buf);
	if (pos != -1) {
		if (ppref_re.cap(3) != "" && ppvalue_re.search(ppref_re.cap(0)) != -1) {
			// The line ends with a '>', but we have this case now..
			// If there is only one instance variable, pp puts everything
			// on a single line:
			//     #<MyClass:0x30094b90 @foobar="hello">
			// So search for '@foobar="hello"', to use as the
			// first name=value pair
			parent->setText(VALUE_COLUMN, ppref_re.cap(1) + ">");
			pos = 0;
		} else {
			// Either a single line like:
			//     #<Thread:0x3008fd18 sleep>
			// Or on multiple lines with name=value pairs:
			//     #<MyClass:0x30093540
 			//		@foobar="hello",
			parent->setText(VALUE_COLUMN, QString("%1%2>").arg(ppref_re.cap(1)).arg(ppref_re.cap(2)));
			pos = ppvalue_re.search(buf, pos);
		}
				
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
	QRegExp array_re("(Array \\(\\d+ element\\(s\\)\\))\\n");
	QRegExp pparray_re("\\s*([^=]+)=([^\\n]+)\\n");
	
	pos = array_re.search(buf);
	if (pos != -1) {
		parent->setText(VALUE_COLUMN, array_re.cap(1));
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
		
		return;
	}
	
	// Look for a hash type which has been printed via a 'pp' command, to
	// expand its sub items. For example:
	//     Hash (2 element(s))
 	//		"greeting"=>"hello"
 	//		"farewell"=>"goodbye"
	//
	QRegExp hash_re("(Hash \\(\\d+ element\\(s\\)\\))\\n");
	QRegExp pphash_re("\\s*([^=\\s]+)=([^\n]+)\\n");
	
	pos = hash_re.search(buf);
	if (pos != -1) {
		parent->setText(VALUE_COLUMN, hash_re.cap(1));
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
		
		return;
	}
	
	return;
}


// **************************************************************************

void RDBParser::setItem(LazyFetchItem *parent, const QString &varName,
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
    case HASH_TYPE:
    case ARRAY_TYPE:
		// Don't set the name in the value column yet, as it gets
		// set when the pp command returns with the array or hash
		// expansion
        item->setCache("");
        break;

    case REFERENCE_TYPE:
        item->setCache("");
        break;

    case VALUE_TYPE:
        item->setText(VALUE_COLUMN, value);
		item->setExpandable(false);
        break;

    default:
        break;
    }
}

// **************************************************************************

DataType RDBParser::determineType(char *buf)
{
	if (qstrncmp(buf, "#<", strlen("#<")) == 0 && strstr(buf, "=") != 0) {
		// An object instance reference is only expandable and a 'REFERENCE_TYPE'
		// if it contains an '=' (ie it has at least one '@instance_variable=value').
		// Otherwise, treat it as a 'VALUE_TYPE'.
		return REFERENCE_TYPE;
	} else if (qstrncmp(buf, "[", strlen("[")) == 0) {
		return ARRAY_TYPE;
	} else if (qstrncmp(buf, "{", strlen("{")) == 0) {
		return HASH_TYPE;
	} else if (qstrncmp(buf, "nil", strlen("nil")) == 0) {
//		return UNKNOWN_TYPE;
		return VALUE_TYPE;
	} else {
		return VALUE_TYPE;
	}
}


// **************************************************************************
// **************************************************************************
// **************************************************************************

}
