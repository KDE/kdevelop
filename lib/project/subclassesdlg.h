
#ifndef SUBCLASSESDLG_H
#define SUBCLASSESDLG_H
#include "domutil.h"
#include "subclassesdlgbase.h"

class SubclassesDlg : public SubclassesDlgBase
{
  Q_OBJECT

public:
    SubclassesDlg(QString form, DomUtil::PairList &config, QString projectDir, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SubclassesDlg();
  /*$PUBLIC_FUNCTIONS$*/

public slots:
  /*$PUBLIC_SLOTS$*/
    virtual void accept();

protected:
  /*$PROTECTED_FUNCTIONS$*/

protected slots:
  /*$PROTECTED_SLOTS$*/
  virtual void          newRelation();
  virtual void          removeRelation();
  virtual void          changeCurrentURL(const QString &str);
  virtual void          currentRelationChanged(QListBoxItem *item);

private:
    QString m_form;
    DomUtil::PairList &m_config;
    QString m_projectDir;
};

#endif

