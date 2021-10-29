/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMAKEBUILDER_H
#define IMAKEBUILDER_H

#include <project/interfaces/iprojectbuilder.h>

#include <QVector>
#include <QStringList>
#include <QPair>

class KJob;

/**
@author Andreas Pakulat
*/

/**
 * Used to create make variables of the form KEY=VALUE.
 */
using MakeVariables = QVector<QPair<QString, QString>>;

class IMakeBuilder : public KDevelop::IProjectBuilder
{
public:

    ~IMakeBuilder() override = default;

    /**
     * Return a build job for the given target.
     *
     * E.g. if @a targetnames is \c 'myModule', the returned job
     * will execute the command:
     *
     * \code make myModule \endcode
     *
     * The command is executed inside the directory of @a item and uses the
     * configuration of its project.
     *
     * @param item Item of the project to build.
     * @param targetname Command-line target name to pass to make.
     */
    virtual KJob* executeMakeTarget(KDevelop::ProjectBaseItem* item,
                                   const QString& targetname ) = 0;

    /**
     * Return a build job for the given targets with optional make variables defined.
     *
     * E.g. if @a targetnames is \code {'all', 'modules'} \endcode and @a variables is
     * \code { 'CFLAGS' : '-Wall', 'CC' : 'gcc-3.4' } \endcode, the returned job should
     * execute this command:
     *
     * \code make CFLAGS=-Wall CC=gcc-3.4 all modules \endcode
     *
     * The command is executed inside the directory of @a item and uses the
     * configuration of its project.
     *
     * @param item Item of the project to build.
     * @param targetnames Optional command-line targets names to pass to make.
     * @param variables Optional list of command-line variables to pass to make.
     */
    virtual KJob* executeMakeTargets(KDevelop::ProjectBaseItem* item,
                                     const QStringList& targetnames = QStringList(),
                                     const MakeVariables& variables = MakeVariables() ) = 0;

Q_SIGNALS:
    /**
     * Emitted every time a target is finished being built for a project item.
     */
    void makeTargetBuilt( KDevelop::ProjectBaseItem* item, const QString& targetname );
};

Q_DECLARE_INTERFACE( IMakeBuilder, "org.kdevelop.IMakeBuilder" )

#endif

