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

#include "svn_mergewidget.h"
#include "subversion_global.h"
#include <kurlrequester.h>
#include <qradiobutton.h>
#include <knuminput.h>
#include <qcheckbox.h>
#include <kcombobox.h>

using namespace SvnGlobal;

SvnMergeDialog::SvnMergeDialog( const KURL &wcTarget, QWidget *parent )
	: SvnMergeOptionDialogBase( parent )
{
	dest->setURL( wcTarget.prettyURL() );
	
	connect( revnumbtn1, SIGNAL(toggled(bool)), revnum1,  SLOT(setEnabled(bool)) );
	connect( revnumbtn1, SIGNAL(toggled(bool)), revkind1, SLOT(setDisabled(bool)) );
	connect( revnumbtn2, SIGNAL(toggled(bool)), revnum2,  SLOT(setEnabled(bool)) );
	connect( revnumbtn2, SIGNAL(toggled(bool)), revkind2, SLOT(setDisabled(bool)) );
	revkind1->setDisabled(true);
	revnum2->setDisabled(true);
}

SvnMergeDialog::~SvnMergeDialog()
{
}

KURL SvnMergeDialog::source1()
{
	return KURL( src1->url() );
}
SvnRevision SvnMergeDialog::rev1()
{
	SvnRevision rev;

	if( revkindbtn1->isChecked() ){
		rev.revNum = -1;
		rev.revKind = revkind1->currentText();
	} else {
		rev.revNum = revnum1->value();
		rev.revKind = "UNSPECIFIED";
	}
	return rev;
}
KURL SvnMergeDialog::source2()
{
	return KURL( src2->url() );
}
SvnRevision SvnMergeDialog::rev2()
{
	SvnRevision rev;

	if( revkindbtn2->isChecked() ){
		rev.revNum = -1;
		rev.revKind = revkind2->currentText();
	} else {
		rev.revNum = revnum2->value();
		rev.revKind = "UNSPECIFIED";
	}
	return rev;
}
bool SvnMergeDialog::recurse()
{
	return !(nonRecurse->isChecked());
}
bool SvnMergeDialog::force()
{
	return forceCheck->isChecked();
}
bool SvnMergeDialog::ignoreAncestry()
{
	return ignoreAncestryCheck->isChecked();
}
bool SvnMergeDialog::dryRun()
{
	return dryRunCheck->isChecked();
}

#include "svn_mergewidget.moc"
