#include <qlayout.h>
#include <qpixmap.h>
#include <qapplication.h>


#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kglobalsettings.h>
#include <kdeversion.h>

#include <kmainwindow.h>
#include "toplevel.h"

#include "splashscreen.h"
#include "splashscreen.moc"

SplashScreen::SplashScreen()
  : QObject()
{
  m_splash = new QWidget( 0,
    "splash", WStyle_NoBorder | WStyle_Customize | WStyle_StaysOnTop /*| WX11BypassWM */);

  m_splash->installEventFilter( this );

  QVBoxLayout *vbox = new QVBoxLayout(m_splash);

  QLabel *pixmap = new QLabel(m_splash);
  QPixmap pm;
  if (TopLevel::mode == TopLevel::AssistantMode)
      pm.load(locate("data", "kdevelop/pics/kdevelop-splash.png"));
  else
      pm.load(locate("appdata", "pics/kdevelop-splash.png"));
  pixmap->setPixmap(pm);
  vbox->addWidget(pixmap);

  m_message = new QLabel(m_splash);
  m_message->setPaletteForegroundColor(Qt::white);
  m_message->setBackgroundColor(Qt::black);
  vbox->addWidget(m_message);

  showMessage(i18n("Starting core application"));

#if defined(KDE_IS_VERSION)
#if (KDE_IS_VERSION(3,1,90))
  QRect rect = KGlobalSettings::splashScreenDesktopGeometry();
#else
  QRect rect = QApplication::desktop()->screenGeometry(
    QApplication::desktop()->screenNumber(QPoint(0,0)));
#endif
#else
  QRect rect = QApplication::desktop()->screenGeometry(
    QApplication::desktop()->screenNumber(QPoint(0,0)));
#endif
  m_splash->move(rect.x() + (rect.width() - m_splash->sizeHint().width()) / 2,
     rect.y() + (rect.height() - m_splash->sizeHint().height()) / 2);
  m_splash->setFixedSize(m_splash->sizeHint());

  m_splash->show();
}


SplashScreen::~SplashScreen()
{
  delete m_splash;
}


void SplashScreen::showMessage(const QString &message)
{
  kdDebug(9000) << "SPLASH: message=" << message << endl;

  m_message->setText(" " + message);
  kapp->processEvents();
}

bool SplashScreen::eventFilter(QObject* obj, QEvent* e)
{
  if (obj == m_splash && e->type() == QEvent::MouseButtonRelease) {
    m_splash->hide();
    return true;
  }
  return false;
}
