/*
 *  Copyright (C) 2007 Dukju Ahn (dukjuahn@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#ifndef SVN_MERGEWIDGET_H
#define SVN_MERGEWIDGET_H

#include "svn_mergeoptiondlgbase.h"
namespace SvnGlobal
{
	class SvnRevision;
}

class KURL;

class SvnMergeDialog : public SvnMergeOptionDialogBase
{
	Q_OBJECT
public:
	SvnMergeDialog( const KURL &wcTarget, QWidget *parent = NULL );
	virtual ~SvnMergeDialog();

	KURL source1();
	SvnGlobal::SvnRevision rev1();
	KURL source2();
	SvnGlobal::SvnRevision rev2();
	bool recurse();
	bool force();
	bool ignoreAncestry();
	bool dryRun();
	
};

#endif
