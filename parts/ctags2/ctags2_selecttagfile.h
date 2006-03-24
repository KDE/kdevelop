/***************************************************************************
 *   Copyright (C) 2006 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CTAGS2_SELECTTAGFILE_H
#define CTAGS2_SELECTTAGFILE_H

#include "ctags2_selecttagfilebase.h"

class SelectTagFile : public SelectTagFileBase {
	Q_OBJECT

	public:
	SelectTagFile( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
	~SelectTagFile();
	QString name();
	QString tagsfilePath();

public slots:
	virtual void validate();
};


#endif

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
