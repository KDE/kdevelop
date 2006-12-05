/***************************************************************************
*   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
*   bernd@kdevelop.org                                                    *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <qdir.h>
#include <qfile.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qdict.h>

#include <kdebug.h>
#include <kparts/componentfactory.h>
#include <kservice.h>

#include "misc.h"

#include "kdevcompileroptions.h"


static KDevCompilerOptions *createCompilerOptions( const QString &name, QObject *parent )
{
	KService::Ptr service = KService::serviceByDesktopName( name );
	if ( !service )
	{
		kdDebug( 9020 ) << "Can't find service " << name << endl;
		return 0;
	}


    KLibFactory *factory = KLibLoader::self()->factory(QFile::encodeName(service->library()));
    if (!factory) {
        QString errorMessage = KLibLoader::self()->lastErrorMessage();
        kdDebug(9020) << "There was an error loading the module " << service->name() << endl <<
	    "The diagnostics is:" << endl << errorMessage << endl;
        exit(1);
    }

    QStringList args;
    QVariant prop = service->property("X-KDevelop-Args");
    if (prop.isValid())
        args = QStringList::split(" ", prop.toString());

    QObject *obj = factory->create(parent, service->name().latin1(),
                                   "KDevCompilerOptions", args);

    if (!obj->inherits("KDevCompilerOptions")) {
        kdDebug(9020) << "Component does not inherit KDevCompilerOptions" << endl;
        return 0;
    }
    KDevCompilerOptions *dlg = (KDevCompilerOptions*) obj;

    return dlg;

/*
	QStringList args;
	QVariant prop = service->property( "X-KDevelop-Args" );
	if ( prop.isValid() )
		args = QStringList::split( " ", prop.toString() );

	return KParts::ComponentFactory
	       ::createInstanceFromService<KDevCompilerOptions>( service, parent,
	                                                         service->name().latin1(), args );*/
}


QString AutoProjectTool::execFlagsDialog( const QString &compiler, const QString &flags, QWidget *parent )
{
	KDevCompilerOptions * plugin = createCompilerOptions( compiler, parent );

	if ( plugin )
	{
		QString newflags = plugin->exec( parent, flags );
		delete plugin;
		return newflags;
	}
	return QString::null;
}


QString AutoProjectTool::canonicalize( const QString &str )
{
	QString res;
	for ( uint i = 0; i < str.length(); ++i )
		res += ( str[ i ].isLetterOrNumber() || str[ i ] == '@' ) ? str[ i ] : QChar( '_' );

	kdDebug(9020) << k_funcinfo << "normalized '" << str << "' to '" << res << "'" << endl;
	return res;
}


/**
 * Read the Makefile.am and return a map of all the variables.
 * Will take notice of backslash and += constructs.
 * @param fileName
 * @param variables
 */
void AutoProjectTool::parseMakefileam(const QString &fileName, QMap<QString, QString> *variables)
{
	QFile f(fileName);
	if (!f.open(IO_ReadOnly))
	{
		return ;
	}
	QTextStream stream(&f);

	QRegExp re("^(#kdevelop:[ \t]*)?([A-Za-z][@A-Za-z0-9_]*)[ \t]*([:\\+]?=)[ \t]*(.*)$");

	QString last;
	bool multiLine = false;
	while (!stream.atEnd())
	{
		QString s = stream.readLine().stripWhiteSpace();
		if (re.exactMatch(s))
		{
			QString lhs = re.cap(2);
			QString rhs = re.cap(4);
			if (rhs[ rhs.length() - 1 ] == '\\')
			{
				multiLine = true;
				last = lhs;
				rhs[rhs.length() - 1] = ' ';
			}

			// The need for stripWhiteSpace seems to be a Qt bug.
			// make our list nice and neat.
			QStringList bits = QStringList::split(" ", rhs);
			rhs = bits.join(" ");
			if (re.cap(3) == "+=")
			{
				((*variables)[lhs] += ' ') += rhs;
			}
			else
			{
				variables->insert(lhs, rhs);
			}
		}
		else if (multiLine)
		{
			if (s[s.length()-1] == '\\')
			{
				s[s.length()-1] = ' ';
			}
			else
			{
				multiLine = false;
			}
			QStringList bits = QStringList::split(" ", s);
			((*variables)[last] += ' ') += bits.join(" ");
		}
	}
	f.close();

	QMap<QString, QString> list;

	for (QMap<QString, QString>::iterator iter = variables->begin();iter != variables->end();iter++)
	{
		QStringList items = QStringList::split(" ", iter.data());
		QMap<QString, QString> unique;
		for (uint i = 0;i < items.size();i++)
		{
			unique.insert(items[i], "");
		}
		QString line;
		for (QMap<QString, QString>::iterator it = unique.begin();it != unique.end();it++)
		{
			line += it.key() + ' ';
		}
		if (line.length() > 1)
		{
			line.setLength(line.length() - 1);
		}

		list.insert(iter.key(), line);
	}
	*variables = list;
}

/**
 * Add entries to a variable. Will just add the variables to the existing line, removing duplicates
 * Will preserve += constructs and make sure that the variable only has one copy of the value across
 * all += constructs
 * @param fileName
 * @param variables key=value string of entries to add
 */
void AutoProjectTool::addToMakefileam(const QString &fileName, QMap<QString, QString> variables)
{
	AutoProjectTool::addRemoveMakefileam(fileName,  variables,  true);
}

/**
 * Set entries to a variable. Will set the variables to the existing line, removing duplicates
 * Will preserve += constructs and make sure that the variable only has one copy of the value across
 * all += constructs
 * Adds line if it does not exist.
 * @param fileName
 * @param variables key=value string of entries to add
 */
void AutoProjectTool::setMakefileam ( const QString &fileName, QMap<QString, QString> variables )
{
	for ( QMap<QString, QString>::Iterator it0 = variables.begin(); it0 != variables.end(); ++it0 )
	{
		kdDebug ( 9020 ) << "key (set): " << it0.key() << "=" << it0.data() << endl;
	}

	// input file reading
	QFile fin ( fileName );
	if ( !fin.open ( IO_ReadOnly ) )
	{
		return ;
	}
	QTextStream ins ( &fin );

	// output file writing.
	QFile fout ( fileName + "#" );
	if ( !fout.open ( IO_WriteOnly ) )
	{
		fin.close();
		return ;
	}
	QTextStream outs ( &fout );

	// variables
	QRegExp re ( "^(#kdevelop:[ \t]*)?([A-Za-z][@A-Za-z0-9_]*)[ \t]*([:\\+]?=)[ \t]*(.*)$" );

	bool multiLine = false;
	QString lastLhs;
	QMap<QString, QString> seenLhs;
	while ( !fin.atEnd() )
	{
		QString s = ins.readLine();
		if ( re.exactMatch ( s ) )
		{
			QString lhs = re.cap ( 2 );
			bool notFound = ( variables.find ( lhs ) == variables.end() );

			if ( notFound )
			{
				if ( seenLhs.find ( lhs ) == seenLhs.end() )
				{
					// not interested in this line at all
					// write it out as is..
					outs << s << endl;
				}
				// we have seen this variable, but since we are setting the
				// whole line - we skip this as it will be a += line.
			}
			else
			{
				// we are interested in this line..
				QString rhs = re.cap ( 4 ).stripWhiteSpace();
				if ( rhs[ rhs.length() - 1 ] == '\\' )
				{
					// save it for when we have the whole line..
					multiLine = true;
					lastLhs = lhs;
				}
				else
				{
					// deal with it now - a single line
					// we are adding our interested values to this line and writing it
					// now write the line out if it is not going to be empty.
					QString newLine ( lhs );
					newLine += " = ";
					bool added = false;
					int len = newLine.length();
					QStringList variableList = QStringList::split ( ' ', variables[lhs] );
					for ( uint count = 0; count < variableList.size(); count++ )
					{
						len += variableList[count].length() + 1;
						if ( len > 80 )
						{
							newLine += "\\\n\t";
							len = 8;
						}
						newLine += variableList[count];
						newLine += ' ';
						added = true;

					}
					// only print it out if there was a value to add..
					if ( added )
					{
						newLine.setLength ( newLine.length() - 1 );
						outs << newLine << endl;
					}
					seenLhs[lhs] = "done";
					variables.erase ( lhs );
				}
			}
		}
		else if ( multiLine )
		{
			s = s.stripWhiteSpace();
			// we are only here if were interested in this line..
			if ( s[s.length()-1] == '\\' )
			{
				s.setLength ( s.length() - 1 );
				// still more multi line we wait for..
			}
			else
			{
				// end of the multi line..
				multiLine = false;
			}

			if ( !multiLine )
			{
				// we are adding our interested values to this line and writing it
				// now write the line out if it is not going to be empty.
				QString newLine ( lastLhs );
				newLine += " = ";
				bool added = false;
				int len = newLine.length();
				QStringList variableList = QStringList::split ( ' ', variables[lastLhs] );
				for ( uint count = 0; count < variableList.size(); count++ )
				{
					len += variableList[count].length() + 1;
					if ( len > 80 )
					{
						newLine += "\\\n\t";
						len = 8;
					}
					newLine += variableList[count];
					newLine += ' ';
					added = true;
				}
				// only print it out if there was a value to add..
				if ( added )
				{
					newLine.setLength ( newLine.length() - 1 );
					outs << newLine << endl;
				}
				seenLhs[lastLhs] = "done";
				variables.erase ( lastLhs );
				lastLhs.setLength ( 0 );
			}
		}
		else
		{
			// can write this line out..
			// not a match, not a multi line,
			outs << s << endl;
		}
	}

	for ( QMap<QString, QString>::Iterator it0 = variables.begin(); it0 != variables.end(); ++it0 )
	{
		QString newLine ( it0.key() );
		newLine += " = ";
		bool added = false;
		int len = newLine.length();
		QStringList variableList = QStringList::split ( ' ', it0.data() );
		for ( uint count = 0; count < variableList.size(); count++ )
		{
			len += variableList[count].length() + 1;
			if ( len > 80 )
			{
				newLine += "\\\n\t";
				len = 8;
			}
			newLine += variableList[count];
			newLine += ' ';
			added = true;

		}
		// only print it out if there was a value to add..
		if ( added )
		{
			newLine.setLength ( newLine.length() - 1 );
			outs << newLine << endl;
		}
	}

	fin.close();
	fout.close();

	QDir().rename ( fileName + "#", fileName );
}


/**
 * Add entries to a variable. Will just add the variables to the existing line, removing duplicates
 * Will preserve += constructs and make sure that the variable only has one copy of the value across
 * all += constructs
 * @param fileName
 * @param variables key=value string of entries to add
 * @param add true= add these key,value pairs, false = remove. You can have empty values for an add - the whole line is
 * removed. For adding, we will not add an empty line.
 */
void AutoProjectTool::addRemoveMakefileam(const QString &fileName, QMap<QString, QString> variables,  bool add)
{
	// input file reading
	QFile fin(fileName);
	if (!fin.open(IO_ReadOnly))
	{
		return ;
	}
	QTextStream ins(&fin);

	// output file writing.
	QFile fout(fileName + "#");
	if (!fout.open(IO_WriteOnly))
	{
		fin.close();
		return ;
	}
	QTextStream outs(&fout);

	// variables
	QRegExp re("^(#kdevelop:[ \t]*)?([A-Za-z][@A-Za-z0-9_]*)[ \t]*([:\\+]?=)[ \t]*(.*)$");

	// build key=map of values to add
	// map can be empty.we never add an empty key, but do remove empty keys from the file..
	QDict< QMap<QString, bool> > interest;
	for (QMap<QString, QString>::Iterator it0 = variables.begin(); it0 != variables.end(); ++it0)
	{
		kdDebug(9020) << "key (" << add<<"): " << it0.key() << "="<< it0.data() << endl;

		QMap<QString, bool>* set = new QMap<QString, bool>();
		if (!it0.data().stripWhiteSpace().isEmpty())
		{
			QStringList variableList = QStringList::split(' ', it0.data());

			for (uint i = 0; i < variableList.count(); i++)
			{
				set->insert(variableList[i], true);
			}
		}
		interest.insert(it0.key(), set);
	}

	bool multiLine = false;
	QString lastLhs;
	QStringList lastRhs;
	QMap<QString, QString> seenLhs;
	while (!fin.atEnd())
	{
		QString s = ins.readLine();
		if (re.exactMatch(s))
		{
			QString lhs = re.cap(2);
			QMap<QString, bool>* ourRhs = interest.find(lhs);

			if (!ourRhs)
			{
				// not interested in this line at all
				// write it out as is..
				outs << s << endl;
			}
			else
			{
				// we are interested in this line..
				QString rhs = re.cap(4).stripWhiteSpace();
				if (rhs[ rhs.length() - 1 ] == '\\')
				{
					// save it for when we have the whole line..
					multiLine = true;
					lastLhs = lhs;
					rhs.setLength(rhs.length() - 1);
					lastRhs += QStringList::split(" ", rhs);
				}
				else
				{
					// deal with it now.

					QStringList bits = QStringList::split(" ", rhs);
					if (add)
					{
						// we are adding our interested values to this line and writing it

						// add this line to we we want to add to remove duplicates.
						for (uint index = 0; index < bits.size(); index++)
						{
							QMap<QString, bool>::iterator findEntry = ourRhs->find(bits[index]);
							if (findEntry == ourRhs->end())
							{
								// we haven't seen it, so add it, so we don't add it again later..
								ourRhs->insert(bits[index], true);
							}
							// else we have this value in our 'to add list' , it is either already been
							// added, so we don't want to add it again, or it hasn't been added, in which
							// case we will do so soon. so we can ignore this now..
						}
						// now write the line out if it is not going to be empty.
						QString newLine(lhs);
						if (seenLhs.find(lhs) == seenLhs.end())
						{
							newLine += " = ";
							seenLhs[lhs] = "";
						}
						else
						{
							newLine += " += ";
						}

						int len = newLine.length();
						bool added = false;
						QValueList<QString> keys = ourRhs->keys();
						for (uint count = 0; count < keys.size(); count++)
						{
							// if out entry is true, add it..
							if ((*ourRhs)[keys[count]])
							{
								added = true;
								len += keys[count].length() + 1;
								if (len > 80)
								{
									newLine += "\\\n\t";
									len = 8;
								}
								newLine += keys[count];
								newLine += ' ';
								// set our value so we don't add it again.
								(*ourRhs)[keys[count]] = false;
							}
						}
						// only print it out if there was a value to add..
						if (added)
						{
							newLine.setLength(newLine.length() - 1);
							outs << newLine << endl;
						}
					}
					else
					{
						// we are removing our interested values from this line

						// special case - no values, remove the line..
						if (!ourRhs->empty())
						{
							// check if any of these values are down to remove.
							QString newLine(lhs);
							if (seenLhs.find(lhs) == seenLhs.end())
							{
								newLine += " = ";
								seenLhs[lhs] = "";
							}
							else
							{
								newLine += " += ";
							}

							int len = newLine.length();
							bool added = false;
							for (QStringList::Iterator posIter = bits.begin(); posIter != bits.end();posIter++)
							{
								QMap<QString, bool>::iterator findEntry = ourRhs->find(*posIter);
								if (findEntry == ourRhs->end())
								{
									// we do not want to remove it..
									added = true;
									len += (*posIter).length() + 1;
									if (len > 80)
									{
										newLine += "\\\n\t";
										len = 8;
									}
									newLine += (*posIter);
									newLine += ' ';
								}
								// else we have this value in our 'to remove list', so don't add it.
							}
							// only print it out if there was a value on it..
							if (added)
							{
								newLine.setLength(newLine.length() - 1);
								outs << newLine << endl;
							}
						}
					}//if (add)
				}//if ( rhs[ rhs.length() - 1 ] == '\\'  )
			}//if ( found == interest.end())
		}
		else if (multiLine)
		{
			s = s.stripWhiteSpace();
			// we are only here if were interested in this line..
			if (s[s.length()-1] == '\\')
			{
				s.setLength(s.length() - 1);
				// still more multi line we wait for..
			}
			else
			{
				// end of the multi line..
				multiLine = false;
			}
			lastRhs += QStringList::split(" ", s);

			if (!multiLine)
			{
				// now we have to deal with this multiLine value..
				// ourRhs will always be a value, as we only get multiLine if we're interested in it..
				QMap<QString, bool>* ourRhs = interest.find(lastLhs);

				if (add)
				{
					// we are adding our interested values to this line and writing it

					// add this line to we we want to add to remove duplicates.
					for (uint index = 0; index < lastRhs.size(); index++)
					{
						QMap<QString, bool>::iterator findEntry = ourRhs->find(lastRhs[index]);
						if (findEntry == ourRhs->end())
						{
							// we haven't seen it, so add it, so we don't add it again later..
							ourRhs->insert(lastRhs[index], true);
						}
						// else we have this value in our 'to add list' , it is either already been
						// added, so we don't want to add it again, or it hasn't been added, in which
						// case we will do so soon. so we can ignore this now..
					}
					// now write the line out if it is not going to be empty.
					QString newLine(lastLhs);
					if (seenLhs.find(lastLhs) == seenLhs.end())
					{
						newLine += " = ";
						seenLhs[lastLhs] = "";
					}
					else
					{
						newLine += " += ";
					}

					int len = newLine.length();
					bool added = false;
					QValueList<QString> keys = ourRhs->keys();
					for (uint count = 0; count < keys.size(); count++)
					{
						// if out entry is true, add it..
						if ((*ourRhs)[keys[count]])
						{
							added = true;
							len += keys[count].length() + 1;
							if (len > 80)
							{
								newLine += "\\\n\t";
								len = 8;
							}
							newLine += keys[count];
							newLine += ' ';
							// set our value so we don't add it again.
							(*ourRhs)[keys[count]] = false;
						}
					}
					// only print it out if there was a value to add..
					if (added)
					{
						newLine.setLength(newLine.length() - 1);
						outs << newLine << endl;
					}
				}
				else
				{
					// we are removing our interested values from this line
					// special case - no values, remove the line..
					if (!ourRhs->empty())
					{
						// check if any of these values are down to remove.
						QString newLine(lastLhs);
						if (seenLhs.find(lastLhs) == seenLhs.end())
						{
							newLine += " = ";
							seenLhs[lastLhs] = "";
						}
						else
						{
							newLine += " += ";
						}
						int len = newLine.length();
						bool added = false;
						for (QStringList::Iterator posIter = lastRhs.begin(); posIter != lastRhs.end();posIter++)
						{
							QMap<QString, bool>::iterator findEntry = ourRhs->find(*posIter);
							if (findEntry == ourRhs->end())
							{
								// we do not want to remove it..
								added = true;
								len += (*posIter).length() + 1;
								if (len > 80)
								{
									newLine += "\\\n\t";
									len = 8;
								}
								newLine += (*posIter);
								newLine += ' ';
							}
							// else we have this value in our 'to remove list', so don't add it.
						}
						// only print it out if there was a value on it..
						if (added)
						{
							newLine.setLength(newLine.length() - 1);
							outs << newLine << endl;
						}
					}
				}

				lastLhs.setLength(0);
				lastRhs.clear();
			}
		}
		else
		{
			// can write this line out..
			// not a match, not a multi line,
			outs << s << endl;
		}
	}

	if (add)
	{
		QDictIterator<QMap<QString, bool> > it(interest);
		for (; it.current(); ++it)
		{
			QString lhs = it.currentKey();
			QMap<QString, bool>* ourRhs = it.current();

			QString newLine(lhs);
			if (seenLhs.find(lhs) == seenLhs.end())
			{
				newLine += " = ";
				seenLhs[lastLhs] = "";
			}
			else
			{
				newLine += " += ";
			}
			int len = newLine.length();
			bool added = false;
			QValueList<QString> keys = ourRhs->keys();
			for (uint count = 0; count < keys.size(); count++)
			{
				if ((*ourRhs)[keys[count]])
				{
					added = true;
					len += keys[count].length() + 1;
					if (len > 80)
					{
						newLine += "\\\n\t";
						len = 8;
					}
					newLine += keys[count];
					newLine += ' ';
					// set our value so we don't add it again.
					(*ourRhs)[keys[count]] = false;
				}
			}
			// only print it out if there was a value to add..
			if (added)
			{
				newLine.setLength(newLine.length() - 1);
				outs << newLine << endl;
			}
		}
	}
	interest.setAutoDelete(true);
	interest.clear();

	fin.close();
	fout.close();

	QDir().rename(fileName + "#", fileName);
}

/**
 * Any items in the map will be removed from the Makefile.am
 * Empty lines are removed. eg. foo_LDDADD if empty is removed.
 * @param fileName full path to Makefile.am
 * @param variables lines to remove items from.
 */
void AutoProjectTool::removeFromMakefileam(const QString &fileName, QMap <QString, QString> variables)
{
	AutoProjectTool::addRemoveMakefileam(fileName, variables,  false);
}


/**
 * Open the file and parse out the AC_OUTPUT line. following backslash continue lines..
 * @param configureinpath
 * @return list of all the values
 */
QStringList AutoProjectTool::configureinLoadMakefiles(QString configureinpath)
{
	QFile configurein(configureinpath);

	if (!configurein.open(IO_ReadOnly))
	{
		kdDebug(9020) << k_funcinfo << " - couldn't open file: " << configureinpath << endl;
		return QStringList();
	}

	QTextStream stream(&configurein);
	QStringList list;

	QString ac_match("^AC_OUTPUT");

	QRegExp ac_regex(ac_match);
	bool multiLine = false;
	QChar cont('\\');
	QRegExp close("\\)");
	QRegExp open("\\(");
	while (!stream.eof())
	{
		QString line = stream.readLine().stripWhiteSpace();
		if (multiLine)
		{
			if (close.search(line) >= 0)
			{
				line = line.replace(close.search(line), 1, "");
				list += QStringList::split(" ", line);
				break;
			}
			else
			{
				if (line.endsWith(cont))
				{
					line.setLength(line.length() - 1);
				}
				list += QStringList::split(" ", line);
			}
		}
		else if (ac_regex.search(line) >= 0)
		{
			line = line.replace(ac_regex.search(line), ac_match.length() - 1, "");

			if (open.search(line) >= 0)
			{
				line = line.replace(open.search(line), 1, "");
			}

			if (line.endsWith(cont))
			{
				line.setLength(line.length() - 1);
				multiLine = true;
			}
			else
			{
				if (close.search(line) >= 0)
				{
					line = line.replace(close.search(line), 1, "");
				}
			}

			list = QStringList::split(" ", line);

			if (!multiLine)
			{
				break;
			}
		}
	}

	configurein.close();

	// make a new object on the heap
	return list;

}

/**
 * Write the addition items to the AC_OUTPUT line.
 * @param configureinpath
 * @param makefiles
 */
void AutoProjectTool::configureinSaveMakefiles(QString fileName, QStringList makefiles)
{
// input file reading
	QFile fin(fileName);
	if (!fin.open(IO_ReadOnly))
	{
		return ;
	}
	QTextStream ins(&fin);

	// output file writing.
	QFile fout(fileName + "#");
	if (!fout.open(IO_WriteOnly))
	{
		fin.close();
		return ;
	}
	QTextStream outs(&fout);

	QMap<QString, QString> toAdd;
	for (uint i = 0; i < makefiles.size();i++)
	{
		toAdd.insert(makefiles[i].stripWhiteSpace(), "");
	}

	QString ac_match("^AC_OUTPUT");
	QRegExp ac_regex(ac_match);
	bool multiLine = false;
	QChar cont('\\');
	QRegExp close("\\)");
	QRegExp open("\\(");
	bool done = false;
	while (!fin.atEnd())
	{
		QString line = ins.readLine();
		if (done)
		{
			outs << line << endl;
		}
		else
		{
			if (multiLine)
			{
				line = line.stripWhiteSpace();
				if (close.search(line) >= 0)
				{
					line = line.replace(close.search(line), 1, "");
					QStringList list = QStringList::split(" ", line);
					// remove dups if any..
					for (uint i = 0; i < list.size();i++)
					{
						toAdd.insert(list[i], "");
					}
					int len = 10;
					QString acline("AC_OUTPUT(");
					for (QMap<QString, QString>::iterator iter = toAdd.begin();iter != toAdd.end();iter++)
					{
						len += iter.key().length();
						if (len > 80)
						{
							acline += "\\\n\t";
							len = 8;
						}
						acline += iter.key();
						acline += ' ';
					}
					acline.setLength(acline.length() - 1);
					acline = acline.append(")");
					outs  << acline << endl;
					done = true;
				}
				else
				{
					if (line.endsWith(cont))
					{
						line.setLength(line.length() - 1);
					}

					QStringList list = QStringList::split(" ", line);
					// remove dups if any..
					for (uint i = 0; i < list.size();i++)
					{
						toAdd.insert(list[i], "");
					}
				}
			}
			else if (ac_regex.search(line) >= 0)
			{
				line = line.stripWhiteSpace();
				line = line.replace(ac_regex.search(line), ac_match.length() - 1, "");
				if (line.endsWith(cont))
				{
					line.setLength(line.length() - 1);
					multiLine = true;
				}
				if (open.search(line) >= 0)
				{
					line = line.replace(open.search(line), 1, "");
				}
				if (close.search(line) >= 0)
				{
					line = line.replace(close.search(line), 1, "");
				}

				QStringList list = QStringList::split(" ", line);
				// remove dups if any..
				for (uint i = 0; i < list.size();i++)
				{
					toAdd.insert(list[i], "");
				}

				if (!multiLine)
				{
					int len = 10;
					QString acline("AC_OUTPUT(");
					for (QMap<QString, QString>::iterator iter = toAdd.begin();iter != toAdd.end();iter++)
					{
						len += iter.key().length();
						if (len > 80)
						{
							acline += "\\\n\t";
							len = 8;
						}
						acline += iter.key();
						acline += ' ';
					}
					acline.setLength(acline.length() - 1);
					acline = acline.append(")");
					outs << acline << endl;
					done = true;
				}
			}
			else
			{
				outs << line << endl;
			}
		}
	}

	fin.close();
	fout.close();

	QDir().rename(fileName + "#", fileName);

}

//kate: indent-mode csands; space-indent off; tab-width 4;
