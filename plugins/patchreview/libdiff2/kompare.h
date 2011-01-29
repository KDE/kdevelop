/***************************************************************************
                                kompare.h  -  description
                                -------------------
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

#ifndef KOMPARE_H
#define KOMPARE_H

#include <kurl.h>

#include "diff2export.h"

// Forward declaration needed
class KTempDir;

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
		ComparingFiles,      // compareFiles
		ComparingFileString, // Compare a source file with a destination string
		ComparingStringFile, // Compare a source string with a destination file
		ComparingDirs,       // compareDirs
		ShowingDiff,         // openDiff
		BlendingDir,         // openDirAnfDiff
		BlendingFile,        // openFileAndDiff
		UnknownMode          // Used to initialize the Infoi struct
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
			QString _localDestination = "",
			KTempDir* _sourceKTempDir = 0,
			KTempDir* _destinationKTempDir = 0,
			uint _depth = 0,
			bool _applied = true
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
			sourceKTempDir = _sourceKTempDir;
			destinationKTempDir = _destinationKTempDir;
			depth = _depth;
			applied = _applied;
		}
		void swapSourceWithDestination()
		{
			KUrl url = source;
			source = destination;
			destination = url;

			QString string = localSource;
			localSource = localDestination;
			localDestination = string;

			KTempDir* tmpDir = sourceKTempDir;
			sourceKTempDir = destinationKTempDir;
			destinationKTempDir = tmpDir;
		}
		enum Mode      mode;
		enum DiffMode  diffMode;
		enum Format    format;
		enum Generator generator;
		KUrl           source;
		KUrl           destination;
		QString        localSource;
		QString        localDestination;
		KTempDir*      sourceKTempDir;
		KTempDir*      destinationKTempDir;
		uint           depth;
		bool           applied;
	};
} // End of namespace Kompare

/*
** This should be removed and put somewhere else
*/
class DIFF2_EXPORT KompareFunctions
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
