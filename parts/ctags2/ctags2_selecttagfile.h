// Description:
//
//
// Author: The KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//


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
