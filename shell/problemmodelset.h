/*
 * Copyright 2015 Laszlo Kis-Adam
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef PROBLEMMODELSET_H
#define PROBLEMMODELSET_H

#include <shell/shellexport.h>
#include <QObject>
#include <QList>

class QAbstractItemModel;

namespace KDevelop
{

class ProblemModel;

/// Struct that handles the model and it's name as one unit, stored in ProblemModelSet
struct ModelData
{
    QString name;
    ProblemModel *model;
};

struct ProblemModelSetPrivate;

/**
 * @brief Stores name/model pairs and emits signals when they are added/removed.
 *
 * Typically it's used from plugins, which maintains a reference to the model added.
 * Therefore It assumes that models get removed, so it doesn't delete!
 *
 * Usage example:
 * @code
 * ProblemModelSet *set = new ProblemModelSet();
 * ProblemModel *model = new ProblemModel(nullptr);
 * set->addModel(QStringLiteral("MODEL"), model); // added() signal is emitted
 * set->models().count(); // returns 1
 * set->findModel(QStringLiteral("MODEL")); // returns the model just added
 * set->removeModel(QStringLiteral("MODEL")); // removed() signal is emitted
 * @endcode
 *
 */
class KDEVPLATFORMSHELL_EXPORT ProblemModelSet : public QObject
{
    Q_OBJECT
public:
    explicit ProblemModelSet(QObject *parent = nullptr);
    ~ProblemModelSet();

    /// Adds a model
    void addModel(const QString &name, ProblemModel *model);

    /// Finds a model
    ProblemModel* findModel(const QString &name) const;

    /// Removes a model
    void removeModel(const QString &name);

    /// Retrieves a list of models stored
    QVector<ModelData> models() const;

signals:
    /// Emitted when a new model is added
    void added(const ModelData &model);

    /// Emitted when a model is removed
    void removed(const QString &name);

private:
    QScopedPointer<ProblemModelSetPrivate> d;

};

}

Q_DECLARE_TYPEINFO(KDevelop::ModelData, Q_MOVABLE_TYPE);

#endif

