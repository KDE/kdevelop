#ifndef IMPORTEXISTINGDLG_H
#define IMPORTEXISTINGDLG_H

#include <qvariant.h>
#include <qdialog.h>
#include <kfile.h>
//#include <kiconview.h>

#include "importexistingdlgbase.h"
#include "kfiledndiconview.h"
#include "fileselectorwidget.h"

class FileSelectorWidget;
class AutoProjectWidget;
class AutoProjectPart;
class SubprojectItem;
class TargetItem;
class KFileItem;

class KImportIconView : public KFileDnDIconView
{
	Q_OBJECT
	
public:
	KImportIconView ( const QString& strIntro, QWidget* parent = 0, const char* name = 0 );
	~KImportIconView() {}
	
	void somethingDropped ( bool dropped );

protected:
    void drawContents ( QPainter *p, int cx, int cy, int cw, int ch );

private:
    QString m_strIntro;
	bool m_bDropped;
};


class ImportExistingDialog : public ImportExistingDlgBase
{ 
    Q_OBJECT

public:
    ImportExistingDialog( AutoProjectPart* part, AutoProjectWidget *widget, SubprojectItem* spitem, TargetItem* titem, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ImportExistingDialog( AutoProjectPart* part, AutoProjectWidget *widget, SubprojectItem* spitem, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~ImportExistingDialog();

private:
    FileSelectorWidget* sourceSelector;
    KImportIconView* importView;

    AutoProjectPart* m_part;
	AutoProjectWidget* m_widget;
	
	bool m_bImportingFiles;

	TargetItem* m_titem;
	SubprojectItem* m_spitem;
	
/*	KURL::List m_addedFiles;*/
  	KFileItemList m_importList;

protected:
	virtual void resizeEvent ( QResizeEvent* ev );
	void init();
	void importItems ();

protected slots:
	void slotAddSelected();
	void slotAddAll();
	void slotRemoveAll();
	void slotRemoveSelected();
	void slotDropped ( KFileView* view, QDropEvent* ev );

	void slotOk();
	void slotHelp();
};

#endif // IMPORTEXISTINGDLG_H
