/***************************************************************************
                          component.h  -  base class for all components
                             -------------------                                         
    copyright            : (C) 1999 by Bernd Gehrmann
    email                : bernd@physik.hu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef _COMPONENT_H_
#define _COMPONENT_H_

#include <qstring.h>

/**
 * This is the base class for all components like doc tree view, LFV, RFV
 * and so on. It contains a collection of virtual classes which components
 * can override if they want to be informed about several kinds of 'events'
 * happening. In this way, it is possible to move much code from the class
 * CKDevelop which handles special cases into separate modules. And of
 * course, this functionality is essential for non-trivial plugins.
 */

class Component
{
public:
    /**
     * The user has changed the path to kdelibs/qt.
     */
    virtual void docPathChanged();
    /**
     * The project has beeen closed.
     */
    virtual void projectClosed();
    /**
     * A file has been added to the project. The filename is
     * given relative to the project dir.
     */
    void addedFileToProject(const QString&);
    /**
     * A file has been removed from the project. The filename is
     * given relative to the project dir.
     */
    void removedFileFromProject(const QString&);
};


#endif





