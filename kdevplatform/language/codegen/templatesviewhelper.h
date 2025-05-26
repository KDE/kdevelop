/*
    SPDX-FileCopyrightText: 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEMPLATES_VIEW_HELPER_H
#define KDEVPLATFORM_TEMPLATES_VIEW_HELPER_H

#include <language/languageexport.h>

#include <QList>
#include <QtClassHelperMacros>

class QModelIndex;
class QString;

namespace KNSCore {
class Entry;
}

namespace KDevelop {
class TemplatesModel;

/**
 * This class contains helper functions useful to classes that manage views of TemplatesModel.
 *
 * The purpose of the class is implementation rather than interface reuse.
 */
class KDEVPLATFORMLANGUAGE_EXPORT TemplatesViewHelper
{
public:
    explicit TemplatesViewHelper(TemplatesModel& model)
        : m_model{model}
    {
    }

    Q_DISABLE_COPY_MOVE(TemplatesViewHelper)

    /**
     * If something has changed, refresh the managed TemplatesModel
     * instance and try to select a newly installed template in the UI.
     *
     * Call this function when the signal KNSWidgets::Button::dialogFinished() or its equivalent is emitted.
     *
     * @return @c true if nothing has changed or if a template was successfully selected in the UI;
     *         @c false otherwise - if the model was refreshed but no template was selected in the UI
     */
    bool handleNewStuffDialogFinished(const QList<KNSCore::Entry>& changedEntries);

protected:
    ~TemplatesViewHelper() = default;

    TemplatesModel& m_model;

private:
    /**
     * Select in the UI the template that corresponds to given template indexes.
     *
     * @return whether the specified template was found and selected successfully
     * @sa TemplatesModel::templateIndexes()
     */
    virtual bool setCurrentTemplate(const QList<QModelIndex>& indexes) = 0;

    /**
     * Select in the UI the template that corresponds to a given template file name.
     *
     * @return whether the specified template was found and selected successfully
     */
    bool setCurrentTemplate(const QString& fileName);
};

} // namespace KDevelop

#endif // KDEVPLATFORM_TEMPLATES_VIEW_HELPER_H
