#include <qlistbox.h>
#include <kurlrequester.h>

#include "subclassesdlg.h"

SubclassesDlg::SubclassesDlg(QString form, DomUtil::PairList &config, QString projectDir, QWidget* parent, const char* name, bool modal, WFlags fl)
: SubclassesDlgBase(parent,name, modal,fl), m_form(form), m_config(config), m_projectDir(projectDir)
{
    subclass_url->setEnabled(false);
     
    DomUtil::PairList::iterator it;
    for ( it = config.begin(); it != config.end(); ++it )
    {
        if ((*it).second == form)
            subclasses_box->insertItem(projectDir + (*it).first);
    }
}

SubclassesDlg::~SubclassesDlg()
{
}

/*$SPECIALIZATION$*/
void SubclassesDlg::accept()
{
    QPtrList<DomUtil::Pair> pairsToRemove;
    
    DomUtil::PairList::iterator it;
    for ( it = m_config.begin(); it != m_config.end(); ++it )
    {
        if ((*it).second == m_form)
        {
            pairsToRemove.append(&(*it));
        }
    }
    
    DomUtil::Pair *pair;
    for ( pair = pairsToRemove.first(); pair; pair = pairsToRemove.next() )
    {
        m_config.remove(*pair);
    }
    
    for (int i = 0; i < subclasses_box->count(); ++i)
    {
        m_config << DomUtil::Pair(subclasses_box->text(i).remove(0,m_projectDir.length()), m_form);
    }
    
    SubclassesDlgBase::accept();
}


void SubclassesDlg::newRelation()
{
    subclasses_box->setFocus();
    subclasses_box->insertItem("");
    subclasses_box->setCurrentItem(subclasses_box->count()-1);
    subclass_url->setEnabled(true);
    subclass_url->setURL("");
    subclass_url->setFocus();    
}

void SubclassesDlg::removeRelation()
{
    if (subclasses_box->currentItem() > -1)
    {
        QListBoxItem *item = subclasses_box->item(subclasses_box->currentItem());      
        int itemIdx = subclasses_box->currentItem();
        
        if (item->prev())
        {
            subclasses_box->setCurrentItem(item->prev());
            subclass_url->setURL(item->prev()->text());
        }
        else if (item->next())
        {
            subclasses_box->setCurrentItem(item->next());
            subclass_url->setURL(item->next()->text());
        }
        else
        {
            subclass_url->setEnabled(false);
            subclass_url->setURL("");
        }
        subclasses_box->removeItem(itemIdx);
    }
}

void SubclassesDlg::changeCurrentURL(const QString &str)
{
    if ( subclasses_box->currentItem() > -1 )
    {
        subclasses_box->changeItem(str, subclasses_box->currentItem());
    }
}

void SubclassesDlg::currentRelationChanged( QListBoxItem * item )
{
    if (item)
    {
        subclass_url->setEnabled(true);
        subclass_url->setURL(item->text());
    }
}

