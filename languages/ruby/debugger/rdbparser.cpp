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
//Added by qt3to4:
#include <Q3CString>

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
	Q3CString	value;
	int			pos;

    Q_ASSERT(parent);
    if (buf == 0 || strlen(buf) == 0) {
        return;
	}

    if (buf[0] == 0) {
        buf = (char*)unknown;
	}
		
	QRegExp var_re("\\s*([^\\n\\s]+) => ([^\\n]+)");
	QRegExp ref_re("(#<([^:]|::)+:0x[\\da-f]+)\\s*([^=]*)>?");
	QRegExp struct_re("#<struct Struct::(\\w+)");
	
	// Look for 'dataitem => value' pairs. For example:
	// 	a => 1
  	// 	m => #<MyClass:0x30093540 @temp={"z"=>"zed", "p"=>"pee"}, @foobar="hello">
	//
	pos = var_re.search(buf);	
	if (pos != -1) {
		while (pos != -1) {
			varName = var_re.cap(1);
			if (ref_re.search(var_re.cap(2)) != -1) {
				if (var_re.cap(2).contains("=") > 0) {
					value = (ref_re.cap(1) + ">").latin1();
				} else {
					// There are no 'name=value' pairs, as in #<Qt::Color:0x0 #ff0000>
					value = var_re.cap(2).latin1();
				}
			} else if (struct_re.search(var_re.cap(2)) != -1) {
				value = (QString("#<Struct::") + struct_re.cap(1) + ">").latin1();
			} else {
				value = var_re.cap(2).latin1();
			}
			
	        DataType dataType = determineType((char *) var_re.cap(2).latin1());
			
			// 'self' variables don't need to be expandable, as their details are
			// already shown in the current frame. So always make them VALUE_TYPE's.
			if (varName == "self") {
				dataType = VALUE_TYPE;
			}
			
			setItem(parent, varName, dataType, value);
			
			pos  += var_re.matchedLength();
			pos = var_re.search(buf, pos);
		}

		return;
	}
}

void RDBParser::parseExpandedVariable(VarItem *parent, char *buf)
{
	DataType	dataType;
	int			pos;
	QString		varName;
	Q3CString	value;
	QRegExp 	ppref_re("(#<([^:]|::)+:0x[\\da-f]+)([^\\n>]*)(>?)");
    
	switch (parent->dataType()) {
    case REFERENCE_TYPE:
	{
		// Look for a reference type which has been printed via a 'pp' command, to
		// expand its sub items on multiple lines. For example:
		//     #<MyClass:0x30093540
		//		@foobar="hello",
		//		@sleeper=#<Thread:0x3008fd18 sleep>,
		//		@temp={"z"=>"zed", "p"=>"pee"}>
		//
		QRegExp ppvalue_re("\\s*([^\\n\\s=]+)=([^\\n]+)[,>]");

		pos = ppref_re.search(buf);
		if (pos != -1) {
			if (ppref_re.cap(4) != "") {
				// The value is all on one line, so match against name=value 
				// pairs which can't have commas in their values
				ppvalue_re = QRegExp("\\s*([^\\s=]+)=([^,>]+)([,>])");
			}

			pos = ppvalue_re.search(buf, pos);
					
			while (pos != -1) {
				varName = ppvalue_re.cap(1);
				
				if (ppref_re.search(ppvalue_re.cap(2)) != -1) {
					if (ppvalue_re.cap(2).contains("=") > 0) {
						value = (ppref_re.cap(1) + ">").latin1();
					} else {
						// There are no 'name=value' pairs, as in #<Qt::Color:0x0 #ff0000>
						value = ppvalue_re.cap(2).latin1();
					}
				} else {
					value = ppvalue_re.cap(2).latin1();
				}
				
				dataType = determineType((char *) ppvalue_re.cap(2).latin1());
				setItem(parent, varName, dataType, value);
				
				pos += ppvalue_re.matchedLength();
				pos = ppvalue_re.search(buf, pos);
			}
			
		}
		return;
	}
	
    case ARRAY_TYPE:
	{
		// Look for a array type which has been printed via a 'pp' command, to
		// expand its sub items. For example:
		//		[0]="hello"
		//		[1]=#"goodbye"
		//
		QRegExp pparray_re("\\s*([^=]+)=([^\\n]+)\\n");
		
		pos = pparray_re.search(buf);
			
		while (pos != -1) {
			varName = pparray_re.cap(1);
			
			if (ppref_re.search(pparray_re.cap(2)) != -1) {
				value = (ppref_re.cap(1) + ">").latin1();
			} else {
				value = pparray_re.cap(2).latin1();
			}
				
			DataType dataType = determineType((char *) pparray_re.cap(2).latin1());
			setItem(parent, varName, dataType, value);
				
			pos += pparray_re.matchedLength();
			pos = pparray_re.search(buf, pos);
		}
					
		return;
	}
	
    case HASH_TYPE:
	{
		// Look for a hash type which has been printed via a 'pp' command, to
		// expand its sub items. For example:
		//		["greeting"]="hello"
		//		["farewell"]="goodbye"
		//
		QRegExp pphash_re("\\s*(\\[[^\\]]+\\])=([^\\n]+)\\n");
		pos = pphash_re.search(buf);
		
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
	
    case STRUCT_TYPE:
	{
		// Look for a reference type which has been printed via a 'pp' command, to
		// expand its sub items. For example:
		//     #<Struct::Customer
		//		@foobar="hello",
		//		@sleeper=#<Thread:0x3008fd18 sleep>,
		//		@temp={"z"=>"zed", "p"=>"pee"}>
		//
		QRegExp ppstruct_re("(#<Struct::\\w+)\\s([^\\n>]*)(>?)");
		QRegExp ppvalue_re("\\s*([^\\n\\s=]+)=([^\\n]+)[,>]");
	
		pos = ppstruct_re.search(buf);
		if (pos != -1) {
			if (ppstruct_re.cap(3) != "" && ppvalue_re.search(ppstruct_re.cap(0)) != -1) {
				// The line ends with a '>', but we have this case now..
				// If there is only one instance variable, pp puts everything
				// on a single line:
				//     #<Struct::Customer @foobar="hello">
				// So search for '@foobar="hello"', to use as the
				// first name=value pair
				pos = 0;
			} else {
				// Mltiple lines with name=value pairs:
				//     #<Struct::Customer
				//		@foobar="hello",
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
			
		}
		return;
	}
			
    case STRING_TYPE:
	{
		// Look for a long String which has been printed via a 'pp' command, to
		// show it as a sequence of 12 bytes slices in hex. For example:
		//		[0..11]=0x89504e470d0a1a0a0000000d
		//		[12..23]=0x494844520000001600000016
		//
		QRegExp ppstring_re("\\s*(\\[[^\\]]+\\])=([^\\n]+)\\n");
		pos = ppstring_re.search(buf);
		
		while (pos != -1) {
			varName = ppstring_re.cap(1);
			value = ppstring_re.cap(2).latin1();
	        DataType dataType = determineType(value.data());
			setItem(parent, varName, dataType, value);
			
			pos += ppstring_re.matchedLength();
			pos = ppstring_re.search(buf, pos);
		}
		
		return;
	}
		
	default:
		Q_ASSERT(false);
	}
	
	return;
}


// **************************************************************************

void RDBParser::setItem(LazyFetchItem *parent, const QString &varName,
                        DataType dataType, const Q3CString &value)
{
	VarItem *item = parent->findItem(varName);
    if (item == 0) {
        item = new VarItem(parent, varName, dataType);
    } else {
		// The dataType of an item can change, so update it
		item->setDataType(dataType);
	}

    switch (dataType) {
    case HASH_TYPE:
    case ARRAY_TYPE:
    case REFERENCE_TYPE:
    case STRUCT_TYPE:
    case STRING_TYPE:
        item->setText(VALUE_COLUMN, value);
		item->setExpandable(true);
        item->update();
        break;

    case COLOR_TYPE:
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
	QRegExp array_re("(Array \\(\\d+ element\\(s\\)\\))");
	QRegExp hash_re("(Hash \\(\\d+ element\\(s\\)\\))");
	QRegExp string_re("(String \\(length \\d+\\))");
	
	if (qstrncmp(buf, "#<struct", strlen("#<struct")) == 0) {
		return STRUCT_TYPE;
	} else if (qstrncmp(buf, "#<Qt::Color:0x", strlen("#<Qt::Color:0x")) == 0) {
		return COLOR_TYPE;
	} else if (qstrncmp(buf, "#<", strlen("#<")) == 0 && strstr(buf, "=") != 0) {
		// An object instance reference is only expandable and a 'REFERENCE_TYPE'
		// if it contains an '=' (ie it has at least one '@instance_variable=value').
		// Otherwise, treat it as a 'VALUE_TYPE'.
		return REFERENCE_TYPE;
	} else if (array_re.search(buf) != -1) {
		return ARRAY_TYPE;
	} else if (hash_re.search(buf) != -1) {
		return HASH_TYPE;
	} else if (string_re.search(buf) != -1) {
		return STRING_TYPE;
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
