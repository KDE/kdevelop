#ifndef _KMDIDOCKWIDGET_H_
#define _KMDIDOCKWIDGET_H_

#ifndef NO_KDE
#include <kdeversion.h>
#endif

#if defined(KDE_MAKE_VERSION)
# if KDE_VERSION < KDE_MAKE_VERSION(3,1,90)
#  ifndef OLDER_THAN_KDE_3_2
#   define OLDER_THAN_KDE_3_2
#  endif
# endif
#else
# define OLDER_THAN_KDE_3_2
#endif

#if defined(NO_KDE) || defined(OLDER_THAN_KDE_3_2)
# include <kdockwidget_compat.h>
using namespace KDockWidget_Compat;
#else
# include <kdockwidget.h>
# include <kdockwidget_p.h>
#endif

#ifndef NO_KDE
#include <kparts/dockmainwindow.h>
#else
#include "dummykpartsdockmainwindow.h"
#endif

class KMdiDockWidget : public KDockWidget
{
  Q_OBJECT
public:
  KMdiDockWidget( KDockManager* dockManager, const char* name,
                  const QPixmap &pixmap, QWidget* parent = 0L, const QString& strCaption = 0L,
                  const QString& strTabPageLabel = " ", WFlags f = 0)
  : KDockWidget(dockManager, name, pixmap, parent, strCaption, strTabPageLabel, f) {}

  void loseFormerBrotherDockWidget()
  {
     KDockWidget::loseFormerBrotherDockWidget();
  }

  void setLatestKDockContainer(QWidget* w)
  {
     KDockWidget::setLatestKDockContainer(w);
  }
};

#ifdef OLDER_THAN_KDE_3_2
// this is a big trick to have another namespace for compatibility
// but still to have moc not being confused with the #ifdefs
# define KParts KDockWidget_Compat
#endif

class KMdiDockMainWindow : public KParts::DockMainWindow
{
   Q_OBJECT
public:
  KMdiDockMainWindow( QWidget* parentWidget, const char* name = "", WFlags flags = WType_TopLevel | WDestructiveClose)
  : KParts::DockMainWindow(parentWidget, name, flags) {}

  KMdiDockWidget* createDockWidget( const QString& name, const QPixmap &pixmap, QWidget* parent = 0L, const QString& strCaption = 0L, const QString& strTabPageLabel = " ")
  {
      KParts::DockMainWindow* that = (KParts::DockMainWindow*) this;
      return (KMdiDockWidget*) that->createDockWidget( name, pixmap, parent, strCaption, strTabPageLabel);
  }
};

class KMdiDockContainerBase : public KDockContainer
{
public:
  KMdiDockContainerBase() : KDockContainer() {}
};

#undef KParts // as KDockWidget_Compat

#endif
