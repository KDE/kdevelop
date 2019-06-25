/*
 * Copyright 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>
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

namespace KDevelop
{

class ProblemModel;
class ProblemModelSetPrivate;

/// Struct that handles the model and it's name as one unit, stored in ProblemModelSet
struct ModelData
{
    QString id;
    QString name;
    ProblemModel *model;
};

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
 * set->addModel(QStringLiteral("MODEL_ID"), QStringLiteral("MODEL"), model); // added() signal is emitted
 * set->models().count(); // returns 1
 * set->findModel(QStringLiteral("MODEL_ID")); // returns the model just added
 * set->removeModel(QStringLiteral("MODEL_ID")); // removed() signal is emitted
 * @endcode
 *
 */
class KDEVPLATFORMSHELL_EXPORT ProblemModelSet : public QObject
{
    Q_OBJECT
public:
    explicit ProblemModelSet(QObject *parent = nullptr);
    ~ProblemModelSet() override;

    /// Adds a model
    void addModel(const QString &id, const QString &name, ProblemModel *model);

    /// Finds a model
    ProblemModel* findModel(const QString &id) const;

    /// Removes a model
    void removeModel(const QString &id);

    /// Show model in ProblemsView
    void showModel(const QString &id);

    /// Retrieves a list of models stored
    QVector<ModelData> models() const;

Q_SIGNALS:
    /// Emitted when a new model is added
    void added(const ModelData &model);

    /// Emitted when a model is removed
    void removed(const QString &id);

    /// Emitted when showModel() is called
    void showRequested(const QString &id);

    /// Emitted when any model emits problemsChanged()
    void problemsChanged();

private:
    const QScopedPointer<class ProblemModelSetPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ProblemModelSet)
};

}

Q_DECLARE_TYPEINFO(KDevelop::ModelData, Q_MOVABLE_TYPE);

#endif

