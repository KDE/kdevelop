#include "uichooser_widget.h"

#include <qradiobutton.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>


UIChooserWidget::UIChooserWidget(QWidget *parent, const char *name)
  : UIChooser(parent, name)
{
  load();
}


void UIChooserWidget::load()
{
  KConfig *config = kapp->config();
  config->setGroup("UI");

  if (config->readEntry("MajorUIMode", "IDEAl") == "IDEAl")
  {
    modeIDEA->setChecked(true);
    return;
  }

  int mdi = config->readNumEntry("MDI mode", 1);

  switch (mdi)
  {
  case 2:
    modeTab->setChecked(true);
    break;
  case 0:
    modeToplevel->setChecked(true);
    break;
  default:
    modeMDI->setChecked(true);
    break;
  }
}


void UIChooserWidget::save()
{
  KConfig *config = kapp->config();
  config->setGroup("UI");

  if (modeIDEA->isChecked())
  {
    config->writeEntry("MajorUIMode", "IDEAl");
  }
  else
  {
    config->writeEntry("MajorUIMode", "QextMDI");

    if (modeTab->isChecked())
      config->writeEntry("MDI mode", 2);
    else if (modeToplevel->isChecked())
      config->writeEntry("MDI mode", 0);
    else
      config->writeEntry("MDI mode", 1);
  }

  config->sync();
}


void UIChooserWidget::accept()
{
  save();
}


#include "uichooser_widget.moc"
