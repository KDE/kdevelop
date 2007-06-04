/***************************************************************************
                                kompare.h  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001-2003 by Otto Bruggeman
                                  and John Firebaugh
        email                   : otto.bruggeman@home.nl
                                  jfirebaugh@kde.org
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#ifndef KOMPARE_H
#define KOMPARE_H

#include <kurl.h>

namespace Kompare
{
	enum Format {
		Context       = 0,
		Ed            = 1,
		Normal        = 2,
		RCS           = 3,
		Unified       = 4,
		SideBySide    = 5,
		UnknownFormat = -1
	};

	enum Generator {
		CVSDiff          = 0,
		Diff             = 1,
		Perforce         = 2,
		SubVersion       = 3,
		Reserved2        = 4,
		Reserved3        = 5,
		Reserved4        = 6,
		Reserved5        = 7,
		Reserved6        = 8,
		Reserved7        = 9,
		UnknownGenerator = -1
	};

	enum Mode {
		ComparingFiles,  // compareFiles
		ComparingDirs,   // compareDirs
		ShowingDiff,     // openDiff
		BlendingDir,     // openDirAnfDiff
		BlendingFile,    // openFileAndDiff
		UnknownMode      // Used to initialize the Infoi struct
	};

	enum DiffMode {
		Default,
		Custom,
		UnknownDiffMode // Use to initialize the Info struct
	};

	enum Status {
		RunningDiff,
		Parsing,
		FinishedParsing,
		FinishedWritingDiff,
		ReRunningDiff	// When a change has been detected after diff has run
	};

	enum Target {
		Source,
		Destination
	};

	struct Info {
		Info (
			enum Mode _mode = UnknownMode,
			enum DiffMode _diffMode = UnknownDiffMode,
			enum Format _format = UnknownFormat,
			enum Generator _generator = UnknownGenerator,
			KUrl _source = KUrl(),
			KUrl _destination = KUrl(),
			QString _localSource = "",
			QString _localDestination = ""
		)
		{
			mode = _mode;
			diffMode = _diffMode;
			format = _format;
			generator = _generator;
			source = _source;
			destination = _destination;
			localSource = _localSource;
			localDestination = _localDestination;
		}
		enum Mode      mode;
		enum DiffMode  diffMode;
		enum Format    format;
		enum Generator generator;
		KUrl           source;
		KUrl           destination;
		QString        localSource;
		QString        localDestination;
	};
} // End of namespace Kompare

/*
** This should be removed and put somewhere else
*/
class KompareFunctions
{
public:
	static QString constructRelativePath( const QString& from, const QString& to )
	{
		KUrl fromURL( from );
		KUrl toURL( to );
		KUrl root;
		int upLevels = 0;

		// Find a common root.
		root = from;
		while( root.isValid() && !root.isParentOf( toURL ) ) {
			root = root.upUrl();
			upLevels++;
		}

		if( !root.isValid() ) return to;

		QString relative;
		for( ; upLevels > 0; upLevels-- ) {
			relative += "../";
		}

		relative += QString( to ).replace( 0, root.path( KUrl::LeaveTrailingSlash ).length(), "" );
		return relative;
	}
};

#endif
