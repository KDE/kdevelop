/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_QUICKOPENDATAPROVIDER_H
#define KDEVPLATFORM_QUICKOPENDATAPROVIDER_H

#include <QList>
#include <QObject>
#include <QSet>
#include <QExplicitlySharedDataPointer>

#include <language/languageexport.h>

class QString;
#include <QStringList>
class QIcon;

namespace KDevelop {
class IndexedString;

/**
 * Hint: When implementing a data-provider, do not forget to export it! Else it won't work.
 * */

/**
 * If your plugin manages a list of files, you can use this to return that list.
 * The file-list can be queried by other data-providers(for example functions/methods) so they
 * can manipulate their content based on those file-lists. The file-list should not be filtered at all,
 * it should only depend on the enabled models/items
 *
 * Example: A list of files in the include-path, a list of files in the project, etc.
 * */
class KDEVPLATFORMLANGUAGE_EXPORT QuickOpenFileSetInterface
{
public:
    virtual QSet<IndexedString> files() const = 0;
    virtual ~QuickOpenFileSetInterface();
};
/**
 * You can use this as additional base-class for your embedded widgets to get additional interaction
 * */
class KDEVPLATFORMLANGUAGE_EXPORT QuickOpenEmbeddedWidgetInterface
{
public:
    virtual ~QuickOpenEmbeddedWidgetInterface();
    ///Is called when the keyboard-shortcut "next" is triggered on the widget, which currently is ALT+Right
    virtual bool next() = 0;
    ///Is called when the keyboard-shortcut "previous" is triggered on the widget, which currently is ALT+Left
    virtual bool previous() = 0;
    ///Is called when the keyboard-shortcut "scroll up" is triggered on the widget, which currently is ALT+Up
    virtual bool up() = 0;
    ///Is called when the keyboard-shortcut "scroll down" is triggered on the widget, which currently is ALT+Down
    virtual bool down() = 0;
    ///Is called when the keyboard-shortcut "back" is triggered on the widget, which currently is ALT+Backspace
    virtual void back() = 0;
    ///Is called when the keyboard-shortcut "accept" is triggered on the widget, which currently is ALT+Return
    virtual void accept() = 0;
    ///Reset the navigation state to the state before keyboard interaction
    virtual void resetNavigationState() = 0;
};

/**
 * Reimplement this to represent single entries within the quickopen list.
 * */
class KDEVPLATFORMLANGUAGE_EXPORT QuickOpenDataBase
    : public QSharedData
{
public:
    virtual ~QuickOpenDataBase();

    ///Return the text to be shown in the list for this item
    virtual QString text() const = 0;

    virtual QString htmlDescription() const = 0;

    /**Can return Custom highlighting triplets as explained in
     * the kde header ktexteditor/codecompletionmodel.h
     * The default-implementation returns an empty list, which means no
     * special highlighting will be applied.
     * */
    virtual QList<QVariant> highlighting() const;

    /**
     * May return an icon to mark the item in the quickopen-list.
     * The standard-implementation returns an invalid item, which means that
     * no icon will be shown.
     * */
    virtual QIcon icon() const;

    /**
     * Is called when the item should be executed.
     *
     * @param filterText Current content of the quickopen-dialogs filter line-edit.
     *                   If this is changed, and false is returned, the content of the
     *                   line-edit will be changed according to the new text.
     * @return Whether the dialog should be closed.
     * */
    virtual bool execute(QString& filterText) = 0;

    /**
     * Return true here if this data-item should be expandable with
     * an own embedded widget.
     * The default-implementation returns false.
     * */
    virtual bool isExpandable() const;

    /**
     * This will be called if isExpandable() returns true.
     *
     * A widget should be returned that will be embedded into the quickopen-list.
     * The widget will be owned by the quickopen-list and will be deleted at will.
     *
     * If the widget can be dynamic_cast'ed to QuickOpenEmbeddedWidgetInterface,
     * the additional interaction defined there will be possible.
     *
     * The default-implementation returns 0, which means no widget will be shown.
     * */
    virtual QWidget* expandingWidget() const;
};

using QuickOpenDataPointer = QExplicitlySharedDataPointer<QuickOpenDataBase>;

/**
 * Use this interface to provide custom quickopen-data to the quickopen-widget.
 *
 * If possible, you should use KDevelop::Filter (@file quickopenfilter.h )
 * to implement the actual filtering, so it is consistent.
 * */

class KDEVPLATFORMLANGUAGE_EXPORT QuickOpenDataProviderBase
    : public QObject
{
    Q_OBJECT

public:
    ~QuickOpenDataProviderBase() override;

    /**
     * For efficiency, all changes to the filter-text are provided by the following 3 difference-operations.
     * */

    /**
     * Search-text was changed.
     * This is called whenever the search-text was changed, and the UI should be updated.
     * Store the text to track the exact difference.
     * */
    virtual void setFilterText(const QString& text) = 0;

    /**
     * Filter-text should be completely reset and the context re-computed.
     * */
    virtual void reset() = 0;

    /**
     * Returns the count of items this provider currently represents
     * */
    virtual uint itemCount() const = 0;

    /**
     * Returns the count of *unfiltered* items this provider currently represents
     */
    virtual uint unfilteredItemCount() const = 0;

    /**
     * Returns the data-item for a given row.
     *
     * Generally, the items must addressed alphabetically,
     * they will be displayed in the same order in the
     * quickopen list.
     *
     * For performance-reasons the underlying models should
     * create the QuickOpenDataBase items on demand, because only
     * those that will really be shown will be requested.
     *
     * @param row Index of item to be returned.
     * */
    virtual QuickOpenDataPointer data(uint row) const  = 0;

    /**
     * If the data-provider supports multiple different scopes/items, this will be called
     * with the enabled scopes/items.
     * If the data-provider supports only one scope/item, this can be ignored.
     * The lists contains all scopes/items, even those that are not supported by this provider.
     * */
    virtual void enableData(const QStringList& items, const QStringList& scopes);
};

/**
 * Try parsing string according to "path_to_file":"line number" template. "line number" may be empty.
 * @param from Source string
 * @param path Set to parsed path to file, or left unchanged if @p from doesn't match the template. May refer to the same object as @p from
 * @param lineNumber Set to parsed line number, zero if "line number" is empty or left unchanged if @p from doesn't match the template.
 * @return Whether @p from did match the expected template.
 * */
bool KDEVPLATFORMLANGUAGE_EXPORT extractLineNumber(const QString& from, QString& path, uint& lineNumber);
}

Q_DECLARE_INTERFACE(KDevelop::QuickOpenFileSetInterface, "org.kdevelop.QuickOpenFileSetInterface")
Q_DECLARE_INTERFACE(KDevelop::QuickOpenEmbeddedWidgetInterface, "org.kdevelop.QuickOpenEmbeddedWidgetInterface")

#endif
