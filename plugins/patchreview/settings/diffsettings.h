/***************************************************************************
                                diffsettings.h
                                --------------
        begin                   : Sun Mar 4 2001
        Copyright 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
        Copyright 2001-2003 John Firebaugh <jfirebaugh@kde.org>
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_DIFFSETTINGS_H
#define KDEVPLATFORM_PLUGIN_DIFFSETTINGS_H

#include <QtCore/QStringList>
#include <QWidget>

#include "../libdiff2/kompare.h"
#include "settingsbase.h"

class DiffSettings : public SettingsBase
{
Q_OBJECT
public:
	DiffSettings( QWidget* parent );
	virtual ~DiffSettings();
public:
	// some virtual functions that will be overloaded from the base class
	virtual void loadSettings( KConfig* config );
	virtual void saveSettings( KConfig* config );

public:
	QString m_diffProgram;
	int m_linesOfContext;
	Kompare::Format m_format;
	bool m_largeFiles;                           // -H
	bool m_ignoreWhiteSpace;                     // -b
	bool m_ignoreAllWhiteSpace;                  // -w
	bool m_ignoreEmptyLines;                     // -B
	bool m_ignoreChangesDueToTabExpansion;       // -E
	bool m_createSmallerDiff;                    // -d
	bool m_ignoreChangesInCase;                  // -i
	bool m_showCFunctionChange;                  // -p
	bool m_convertTabsToSpaces;                  // -t
	bool m_ignoreRegExp;                         // -I
	QString m_ignoreRegExpText;                  // the RE for -I
	QStringList m_ignoreRegExpTextHistory;
	bool m_recursive;                            // -r
	bool m_newFiles;                             // -N
//	bool m_allText;                              // -a
	bool m_excludeFilePattern;                   // -x
	QStringList m_excludeFilePatternList;        // The list of patterns for -x
	bool m_excludeFilesFile;                     // -X
	QString m_excludeFilesFileURL;               // The filename to -X
	QStringList m_excludeFilesFileHistoryList;   // The history list of filenames
};

#endif
