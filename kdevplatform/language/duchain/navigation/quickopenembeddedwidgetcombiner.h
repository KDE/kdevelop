/*
    SPDX-FileCopyrightText: 2019 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_QUICKOPENEMBEDDEDWIDGETCOMBINER_H
#define KDEVPLATFORM_QUICKOPENEMBEDDEDWIDGETCOMBINER_H

#include <QWidget>

#include <interfaces/quickopendataprovider.h>

namespace KDevelop {
class QuickOpenEmbeddedWidgetCombinerPrivate;

/**
 * A widget that implements the QuickOpenEmbeddedWidgetInterface by asking its direct children.
 *
 * I.e. add widgets into the combiner's layout. If the widgets support the QuickOpenEmbeddedWidgetInterface,
 * then they will be used to implement the keyboard navigation features.
 */
class KDEVPLATFORMLANGUAGE_EXPORT QuickOpenEmbeddedWidgetCombiner : public QWidget, public QuickOpenEmbeddedWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::QuickOpenEmbeddedWidgetInterface)

public:
    explicit QuickOpenEmbeddedWidgetCombiner(QWidget* parent = nullptr);
    ~QuickOpenEmbeddedWidgetCombiner() override;

    bool next() override;
    bool previous() override;
    bool up() override;
    bool down() override;
    void back() override;
    void accept() override;
    void resetNavigationState() override;

private:
    const QScopedPointer<QuickOpenEmbeddedWidgetCombinerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QuickOpenEmbeddedWidgetCombiner)
};
}

#endif // KDEVPLATFORM_QUICKOPENEMBEDDEDWIDGETCOMBINER_H
