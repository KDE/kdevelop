#include <qcombobox.h>


#include <kapp.h>
#include <ktrader.h>
#include <kservice.h>
#include <kdebug.h>
#include <kconfig.h>


#include "editorchooser_widget.h"


EditorChooserWidget::EditorChooserWidget(QWidget *parent, const char *name)
  : EditChooser(parent, name)
{
  load();
}


void EditorChooserWidget::load()
{
  EditorPart->clear();
  
  // ask the trader which editors he has to offer
  KTrader::OfferList offers = KTrader::self()->query(QString::fromLatin1("KDevelop/Editor"), QString::null);

  // find the editor to use
  KConfig *config = kapp->config();
  config->setGroup("Editor");
  QString editor = config->readEntry("EmbeddedEditor", "");
  
  // add the entries to the listview
  KTrader::OfferList::Iterator it;
  int index=-1, current=0;
  for (it = offers.begin(); it != offers.end(); ++it)
  {
    EditorPart->insertItem((*it)->comment());
    if ((*it)->name() == editor)
      index = current;
    ++current;
  }

  if (index >=0)
    EditorPart->setCurrentItem(index);
}


void EditorChooserWidget::save()
{
  KConfig *config = kapp->config();   
  config->setGroup("Editor");

  KTrader::OfferList offers = KTrader::self()->query(QString::fromLatin1("KDevelop/Editor"), QString::null);

  KTrader::OfferList::Iterator it;
  for (it = offers.begin(); it != offers.end(); ++it)
    if ((*it)->comment() == EditorPart->currentText())
      config->writeEntry("EmbeddedEditor", (*it)->name());

  config->sync();
}


void EditorChooserWidget::accept()
{
  save();
}


#include "editorchooser_widget.moc"
