/* This file is part of the KDE project
 Copyright (C) 2005 Ian Reinhart Geiser <geiser@kde.org>

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 Boston, MA 02110-1301, USA.
*/
#ifndef KATEDOCUMENTMANAGERINTERFACE_H
#define KATEDOCUMENTMANAGERINTERFACE_H

#include <qobject.h>
#include <dcopobject.h>
#include <dcopref.h>
#include <kurl.h>
class KDevPartController;

/**
This is an emulated interface to provide compatibility with Kate scripts.
 
@author KDevelop Authors
*/
class KateDocumentManagerInterface : public QObject, public DCOPObject {

    Q_OBJECT
    K_DCOP
public:
    KateDocumentManagerInterface( KDevPartController *pc );

    ~KateDocumentManagerInterface();

k_dcop:
    DCOPRef activeDocument();
    DCOPRef document( uint n );
    DCOPRef documentWithID( uint id );
    DCOPRef openURL( const KURL &url, const QString &encoding );
    bool closeAllDocuments();
    bool closeDocument( uint n );
    bool isOpen( const KURL &url );
    int findDocument( const KURL &url );
    uint activeDocumentNumber();
    uint documents();
private:

    KDevPartController *m_controller;
};

#endif
