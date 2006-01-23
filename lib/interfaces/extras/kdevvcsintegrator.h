/* This file is part of the KDE project
   Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KDEVVCSINTEGRATOR_H
#define KDEVVCSINTEGRATOR_H

#include <qobject.h>

/**
@file kdevvcsintegrator.h
The interface to VCS integrators.
*/

class QDomDocument;
class QWidget;

/**
VCS Integration Dialog.

Usually it is created as:
@code
class MyVCSDialog: public QWidget, public VCSDialog {
    MyVCSDialog(QWidget *parent = 0);
    virtual void accept() { ... }
    virtual void init(const QString &projectName, const QString &projectLocation) { ... }
    virtual QWidget *self() {
        return const_cast<MyVCSDialog*>(this);
    }
}
@endcode
*/
class VCSDialog {
public:
    VCSDialog() { }
    /**Implement all integration actions here. Do not use QDialog::accept method
    to perform integration actions.*/
    virtual void accept() = 0;
    /**Init integration dialog with the project name and location.*/
    virtual void init(const QString &projectName, const QString &projectLocation) = 0;
    /**Reimplement to return an actual integration widget. Use QWidgets for that, not
    QDialogs because integrator dialogs are usually have parent containers.*/
    virtual QWidget *self() = 0;
};

/**
The interface to VCS integrators.
VCS integrator takes care about setting up VCS for new and existing projects.
It can, for example, perform checkout or import operations.
*/
class KDevVCSIntegrator: public QObject {
    Q_OBJECT
public:
    KDevVCSIntegrator(QObject *parent = 0);

    /**Reimplement to return a dialog to fetch the project from VCS.*/
    virtual VCSDialog *fetcher(QWidget *parent) = 0;
    /**Reimplement to return a dialog to integrate the project into VCS.*/
    virtual VCSDialog *integrator(QWidget *parent) = 0;
};

#endif
