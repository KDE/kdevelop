#include <qlistbox.h>


#include <ktrader.h>
#include <kservice.h>
#include <kdebug.h>


#include "editorchooser_widget.h"


EditorChooserWidget::EditorChooserWidget(QWidget *parent, const char *name)
  : EditChooser(parent, name)
{
  load();
}


void EditorChooserWidget::load()
{
  EditorList->clear();
  
  // ask the trader which editors he has to offer
  KTrader::OfferList offers = KTrader::self()->query(QString::fromLatin1("KDevelop/Editor"), QString::null);

  // add the entries to the listview
  KTrader::OfferList::Iterator it;
  for (it = offers.begin(); it != offers.end(); ++it)
  {
    EditorList->insertItem((*it)->comment());
  }
}



#include "editorchooser_widget.moc"
