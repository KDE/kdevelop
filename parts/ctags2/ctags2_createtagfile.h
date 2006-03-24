/***************************************************************************
 *   Copyright (C) 2004 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CTAGS2_CREATETAGFILE_H
#define CTAGS2_CREATETAGFILE_H

#include "ctags2_part.h"
#include "ctags2_createtagfilebase.h"


class CreateTagFile : public CTags2CreateTagFileBase {

Q_OBJECT

public:
	CreateTagFile(QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
	~CreateTagFile();
	QString name();
	QString tagsfilePath();
	QString directory();

private:

public slots:
    virtual void validate();
};

#endif
