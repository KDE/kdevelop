/***************************************************************************
                             KDevelopIface.cpp - DCOP interface for KDevelop
                             -------------------
    copyright            : (C) 1999 by Bernd Gehrmann
    email                : bernd@physik.hu-berlin.de
 ***************************************************************************/


#include <dcopclient.h>
#include <ckdevelop.h>
#include "kdevelopifaceimpl.h"


KDevelopIfaceImpl::KDevelopIfaceImpl(CKDevelop *ckdevel)
    : DCOPObject("Application")
{
    core = ckdevel;
}


void KDevelopIfaceImpl::openDocument(const QString &filename)
{
    core->switchToFile(filename);
}
