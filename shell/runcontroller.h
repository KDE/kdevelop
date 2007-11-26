/* This file is part of KDevelop
Copyright 2007 Hamish Rodda <rodda@kde.org>

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

#ifndef RUNCONTROLLER_H
#define RUNCONTROLLER_H

#include "irun.h"
#include "irunprovider.h"

namespace KDevelop
{

class IPlugin;

class RunController : public IRunController
{
    Q_OBJECT
    
public:
    RunController(QObject *parent);
    ~RunController();

    virtual int execute(const IRun& run);
    virtual void abort(int serial);
    virtual void abortAll();
    virtual IRun defaultRun() const;

private Q_SLOTS:
    void slotFinished(int serial);
    void slotExecute();
    void slotOutput(int serial, const QString& line, KDevelop::IRunProvider::OutputTypes type);
    void outputViewRemoved(int id);

private:
    void setupActions();
    void setState(State state);
    void createModel(int serial, const IRun& run);
    IRunProvider* findProvider(const QString& instrumentor);
    
    class RunControllerPrivate;
    RunControllerPrivate* const d;
};

}

#endif
