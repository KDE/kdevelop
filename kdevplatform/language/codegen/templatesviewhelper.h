/*
    SPDX-FileCopyrightText: 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEMPLATES_VIEW_HELPER_H
#define KDEVPLATFORM_TEMPLATES_VIEW_HELPER_H

#include "templatesmodel.h"

#include <language/languageexport.h>

#include <QList>
#include <QtClassHelperMacros>

#include <memory>

class QModelIndex;
class QString;
class QWidget;

namespace KNSCore {
class Entry;
}

namespace KDevelop {
class ITemplateProvider;

/**
 * This class contains helper functions useful to classes that manage views of TemplatesModel.
 *
 * The purpose of the class is implementation rather than interface reuse.
 */
class KDEVPLATFORMLANGUAGE_EXPORT TemplatesViewHelper
{
public:
    explicit TemplatesViewHelper(ITemplateProvider& templateProvider);
    Q_DISABLE_COPY_MOVE(TemplatesViewHelper)

    /**
     * @return the managed TemplatesModel instance
     */
    [[nodiscard]] const TemplatesModel& model() const
    {
        return m_model;
    }
    /**
     * @return the managed TemplatesModel instance
     */
    [[nodiscard]] TemplatesModel& model()
    {
        return m_model;
    }

    /**
     * Create a "Get More Templates" button with the GHNS configuration file
     * of a given template provider and with a given parent.
     *
     * @return the created button or @c nullptr if @p templateProvider does not have a GHNS configuration file
     *
     * @note When the user closes the KNewStuff dialog of the created button, this helper handles possible changes.
     *       Therefore, this helper object must be alive whenever the button's dialog emits the finished() signal.
     */
    [[nodiscard]] QWidget* createGetMoreTemplatesButton(ITemplateProvider& templateProvider, QWidget* buttonParent);

    /**
     * Create a "Load Template from File" button with a given parent.
     *
     * @return the created button (never @c nullptr)
     *
     * @note When the user clicks the created button, this helper shows a file dialog and loads templates from
     *       files selected by the user. Therefore, this helper object must be alive whenever the button is clicked.
     */
    [[nodiscard]] QWidget* createLoadTemplateFromFileButton(QWidget* buttonParent);

protected:
    /**
     * If something has changed, refresh model() and try to select a newly installed template in the UI.
     *
     * Call this function when the signal KNSWidgets::Button::dialogFinished() or its equivalent is emitted.
     */
    void handleNewStuffDialogFinished(const QList<KNSCore::Entry>& changedEntries);

    /**
     * Show a file dialog with a given parent, load templates from existing files
     * selected by the user and try to select a newly loaded template in the UI.
     */
    void loadTemplatesFromFiles();

    /**
     * Refresh model() after its template files change.
     *
     * The default implementation calls model().refresh().
     */
    virtual void refreshModel();

    ~TemplatesViewHelper() = default;

private:
    /**
     * @return a parent widget for dialogs shown by this helper
     */
    [[nodiscard]] virtual QWidget* dialogParent() = 0;

    /**
     * @return the file name of the template that is currently selected in the UI
     *         or an empty string if no template is selected
     */
    [[nodiscard]] virtual QString currentTemplateFileName() const = 0;

    /**
     * Select in the UI the template that corresponds to given template indexes.
     *
     * @return whether the specified template was found and selected successfully
     * @sa TemplatesModel::templateIndexes()
     */
    virtual bool setCurrentTemplate(const QList<QModelIndex>& indexes) = 0;

    /**
     * This function is called when a more relevant template cannot be selected after refreshing model().
     *
     * @note When model() is refreshed (reset), its view and selection model usually do not
     *       notify that the current item or index changes. So this function handles the
     *       situation when the current index of a model()'s view ends up invalid.
     */
    virtual void handleNoTemplateSelectedAfterRefreshingModel() = 0;

    /**
     * Select in the UI the template that corresponds to a given template file name.
     *
     * @return whether the specified template was found and selected successfully
     */
    bool setCurrentTemplate(const QString& fileName);

    /**
     * Refresh model() and try to select a new template,
     * or failing that the previously selected template, in the UI.
     *
     * @param selectNewTemplateInUi a callback that attempts to select a new template in the UI,
     *        returns @c true on success and @c false if selecting a new template fails
     */
    template<typename SelectNewTemplateInUi>
    void refreshModelAndSelectTemplate(SelectNewTemplateInUi selectNewTemplateInUi);

    TemplatesModel m_model;
};

} // namespace KDevelop

#endif // KDEVPLATFORM_TEMPLATES_VIEW_HELPER_H
