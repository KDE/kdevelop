/***************************************************************************
                          registeredfile.h  -  description
                             -------------------
    begin                : Sat May 13 2000
    copyright            : (C) 2000 by Sandy Meier
    email                : smeier@kdevelop.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef REGISTEREDFILE_H
#define REGISTEREDFILE_H

#include <qstring.h>

/**
  *@author Sandy Meier
  */

class RegisteredFile {
public: 
	RegisteredFile();
	~RegisteredFile();
	QString getFilename();
	void setFilename(QString filename);
	
 protected:
	QString m_filename;
	bool m_dist;
	bool m_install;
	QString m_install_path;
};

#endif
