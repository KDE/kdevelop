/***************************************************************************
 *   Copyright (C) 2004 by ian reinhart geiser                             *
 *   geiseri@kde.org                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef __SPECSUPPORT_H__
#define __SPECSUPPORT_H__

#include <qstring.h>
#include <qmap.h>
#include "packagebase.h"
#include "distpart_part.h"
#include "distpart_widget.h"

class SpecSupport : public packageBase {

public:
    SpecSupport(DistpartPart *part);
    ~SpecSupport();

public:
    QString generatePackage();
    void slotbuildAllPushButtonPressed();
    void slotexportSPECPushButtonPressed();
    void slotimportSPECPushButtonPressed();
    void slotsrcPackagePushButtonPressed();

private:
    void parseDotRpmmacros();
    bool createRpmDirectoryFromMacro(const QString & name);

    QString getInfo(QString s, QString motif);

    QString dir;
    DistpartPart *m_part;
    QMap<QString,QString> map;
};

#endif
