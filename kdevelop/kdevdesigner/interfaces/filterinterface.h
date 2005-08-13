 /**********************************************************************
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef FILTERINTERFACE_H
#define FILTERINTERFACE_H

#include <private/qcom_p.h>

// {ea8cb381-59b5-44a8-bae5-9bea8295762a}
#ifndef IID_ImportFilter
#define IID_ImportFilter QUuid( 0xea8cb381, 0x59b5, 0x44a8, 0xba, 0xe5, 0x9b, 0xea, 0x82, 0x95, 0x76, 0x2a )
#endif

/*! If you write a filter plugin to import dialogs or other user
  interfaces from a different format than .ui into the Qt Designer,
  implement this interface in that plugin.

  You also have to implement the function featureList() (\sa
  QFeatureListInterface) and return there all filters (names of it)
  which this interface provides.
*/

struct ImportFilterInterface : public QFeatureListInterface
{
    /*! This function is called by Qt Designer to open the file \a
      filename using the filter \a filter. Qt Designer expects to get
      back one or more .ui files, which it can open then. In the
      implementation of the interface you have to return these
      filenames, which the filter created, in this function.*/
    virtual QStringList import( const QString &filter, const QString &filename ) = 0;
};

// *************** INTERNAL *************************

// {c32a07e0-b006-471e-afca-d227457a1280}
#ifndef IID_ExportFilterInterface
#define IID_ExportFilterInterface QUuid( 0xc32a07e0, 0xb006, 0x471e, 0xaf, 0xca, 0xd2, 0x27, 0x45, 0x7a, 0x12, 0x80 )
#endif

struct ExportFilterInterface : public QFeatureListInterface
{
//    virtual QStringList export( const QString& filter, const QString& filename ) = 0;
};

#endif
