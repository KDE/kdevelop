#ifndef IMPORTEXISTINGDLG_H
#define IMPORTEXISTINGDLG_H

#include <qvariant.h>
#include <qdialog.h>
#include <kfile.h>

#include "importexistingdlgbase.h"

class FileSelectorWidget;
class AutoProjectWidget;
class AutoProjectPart;
class SubprojectItem;
class TargetItem;
class KIconView;


class ImportExistingDialog : public ImportExistingDlgBase
{ 
    Q_OBJECT

public:
    ImportExistingDialog( AutoProjectPart* part, KFile::Mode, const QString& destLabel, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~ImportExistingDialog();

private:
    FileSelectorWidget* sourceSelector;
    KIconView* importView;

    AutoProjectPart* m_part;


};

#endif // IMPORTEXISTINGDLG_H
