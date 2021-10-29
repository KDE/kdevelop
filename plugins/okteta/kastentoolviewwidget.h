/*
    SPDX-FileCopyrightText: 2010 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KASTENTOOLVIEWWIDGET_H
#define KASTENTOOLVIEWWIDGET_H

// Qt
#include <QWidget>

namespace Kasten {
class AbstractToolView;
}
namespace Sublime {
class MainWindow;
class View;
}


namespace KDevelop
{

class KastenToolViewWidget : public QWidget
{
  Q_OBJECT

  public:
    KastenToolViewWidget( Kasten::AbstractToolView* toolView, QWidget* parent );

    ~KastenToolViewWidget() override;

  protected Q_SLOTS:
    void onMainWindowAdded( Sublime::MainWindow* mainWindow );
    void onActiveViewChanged( Sublime::View* view );

  protected:
    Kasten::AbstractToolView* mToolView;
};

}

#endif
