/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qapplication.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpopupmenu.h>
#include <kaction.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kparts/partmanager.h>
//#include <kprinter.h>
#include <kprocess.h>
#include <kstdaction.h>

#include "splitter.h"
#include "texteditor.h"
#include "editorpart.h"


// FIXME: Check whether this really needs to inherit ReadOnlyPart

EditorPart::EditorPart(QWidget *parent, const char */*name*/)
    : KParts::ReadOnlyPart(parent, "editorpart")
{
    QWidget *vbox = new QWidget(parent, "editorvbox");
    vbox->setBackgroundMode(QWidget::NoBackground);
    vbox->setFocusPolicy(QWidget::StrongFocus);
    setWidget(vbox);

    view = 0;
    layout = 0;

    statusbox = new SplitterBar(vbox);
    locationLabel = new QLabel(statusbox);
    //    locationLabel->setFrameStyle(QFrame::Panel);
    overwriteLabel = new QLabel(i18n("OVR"), statusbox);
    overwriteLabel->setFrameStyle(/*QFrame::Panel |*/ QFrame::Sunken);
    overwriteLabel->setFixedWidth(overwriteLabel->sizeHint().width());
    cursorposLabel = new QLabel(i18n("L0000 C000"), statusbox);
    cursorposLabel->setFrameStyle(/*QFrame::Panel |*/ QFrame::Sunken);
    cursorposLabel->setFixedWidth(cursorposLabel->sizeHint().width());

    QBoxLayout *statusLayout = new QHBoxLayout(statusbox, statusbox->frameWidth());
    statusLayout->addWidget(locationLabel, 1);
    statusLayout->addSpacing(5);
    statusLayout->addWidget(overwriteLabel, 0);
    statusLayout->addSpacing(5);
    statusLayout->addWidget(cursorposLabel, 0);

    setXML("<!DOCTYPE kpartgui SYSTEM \"kpartgui.dtd\">\n"
           "<kpartgui version=\"1\" name=\"editorpart\">\n"
           "<MenuBar>\n"
           "  <Menu name=\"file\">\n"
           "    <Action name=\"file_print\" />\n"
           "  </Menu>\n"
           "  <Menu name=\"edit\">\n"
           "    <Action name=\"edit_undo\" group=\"undoredo\" />\n"
           "    <Action name=\"edit_redo\" group=\"undoredo\" />\n"
           "    <Action name=\"edit_undohistory\" group=\"undoredo\" />\n"
           "    <Action name=\"edit_cut\" group=\"cutcopypaste\" />\n"
           "    <Action name=\"edit_copy\" group=\"cutcopypaste\" />\n"
           "    <Action name=\"edit_paste\" group=\"cutcopypaste\" />\n"
           "    <Action name=\"edit_find\" group=\"find\" />\n"
           "    <Action name=\"edit_findnext\" group=\"find\" />\n"
           "    <Action name=\"edit_replace\" group=\"find\" />\n"
           "  </Menu>\n"
           "</MenuBar>\n"
           "<ToolBar name=\"mainToolBar\" >\n"
           "  <Action name=\"file_print\" />\n"
           "</ToolBar>\n"
           "</kpartgui>"
           );
    
    KStdAction::print(this, SLOT(slotPrint()), actionCollection());
    KStdAction::undo(this, SLOT(slotUndo()), actionCollection());
    KStdAction::redo(this, SLOT(slotRedo()), actionCollection());
    (void) new KAction(i18n("Undo/Redo &History..."), 0, this, SLOT(slotUndoHistory()),
                       actionCollection(), "edit_undohistory");
    KStdAction::cut(this, SLOT(slotCut()), actionCollection());
    KStdAction::copy(this, SLOT(slotCopy()), actionCollection());
    KStdAction::paste(this, SLOT(slotPaste()), actionCollection());
    KStdAction::find(this, SLOT(slotFind()), actionCollection());
    KStdAction::findNext(this, SLOT(slotFindAgain()), actionCollection());
    KStdAction::replace(this, SLOT(slotReplace()), actionCollection());
}


EditorPart::~EditorPart()
{
}


bool EditorPart::openFile()
{
    // ReadOnlyPart::openFile() is abstract, so we fake an implementation here
    return true;
}


void EditorPart::gotoDocument(TextEditorDocument *doc, int lineNum)
{
    if (!view || view->editorDocument() != doc) {
        delete view;
        view = new TextEditorView(doc, widget(), "text editor view");
        view->child(0, "KWriteView")->installEventFilter(this);
        connect( view, SIGNAL(undoAvailable(bool)),
                 this, SLOT(updateUndoAvailable(bool)) );
        connect( view, SIGNAL(redoAvailable(bool)),
                 this, SLOT(updateRedoAvailable(bool)) );
        connect( view, SIGNAL(cursorPositionChanged()),
                 this, SLOT(updateCursorPosition()) );
        connect( view, SIGNAL(overwriteModeChanged()),
                 this, SLOT(updateOverwriteMode()) );
        connect( view, SIGNAL(popupMenu(int, int)),
                 this, SLOT(popupMenu(int, int)) );
        connect( view, SIGNAL(toggledBreakpoint(int)),
                 this, SLOT(toggleBreakpoint(int)) );
        connect( view, SIGNAL(editedBreakpoint(int)),
                 this, SLOT(editBreakpoint(int)) );
        connect( view, SIGNAL(toggledBreakpointEnabled(int)),
                 this, SLOT(toggleBreakpointEnabled(int)) );
        widget()->setFocusProxy(view);
        view->show();
        updateWindowCaption();
        
        kdDebug(9000) << "EditorPart::gotoDoc: " << doc->url().path() << endl;
        delete layout;
        layout = new QVBoxLayout(widget());
        layout->addWidget(view, 1);
        layout->addWidget(statusbox, 0);
        layout->activate();

	emit wentToSourceFile(doc->fileName());
    }
    updateUndoAvailable(false);
    updateRedoAvailable(false);
    updateOverwriteMode();
    updateCursorPosition();
    view->setCursorPosition(lineNum, 0);
    view->update();
}


void EditorPart::updateWindowCaption()
{
    setWindowCaption(view->editorDocument()->fileName());
}


TextEditorView *EditorPart::editorView() const
{
    return view;
}


TextEditorDocument *EditorPart::editorDocument() const
{
    return view->editorDocument();
}


static bool isModified(int key)
{
    return  key == Qt::Key_Control
        || key == Qt::Key_Alt
        || key == Qt::Key_Shift;
}


bool EditorPart::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() != QEvent::KeyPress)
        return ReadOnlyPart::eventFilter(o, e);

    QKeyEvent *ke = static_cast<QKeyEvent*>(e);
    if (ke->state() & ControlButton && ke->key() == Key_S) {
        if (isearching) {
            kdDebug(9000) << "Repeat isearch" << endl;
            if (isearchFailed)
                isearchWrapped = true;
            if (editorView()->isearch(searchText, true, isearchFailed)) {
                QString msg = isearchWrapped?
                    i18n("Wrapped I-search:%1").arg(searchText)
                    : i18n("I-search:%1").arg(searchText);
                emit setStatusBarText(msg);
                isearchFailed = false;
            } else {
                QString msg = isearchWrapped?
                    i18n("Wrapped I-search failed.") : i18n("I-search failed.");
                emit setStatusBarText(msg);
                isearchFailed = true;
            }
        } else {
            kdDebug(9000) << "Start isearch" << endl;
            isearching = true;
            isearchFailed = false;
            isearchWrapped = false;
            searchText = QString::null;
            emit setStatusBarText(i18n("I-search:"));
        }
        return true;
    } else if (isearching && ke->text().length()) {
        kdDebug(9000) << "Adding char to isearch" << endl;
        searchText += ke->text();
        emit setStatusBarText(i18n("I-search:%1").arg(searchText));
        if (!editorView()->isearch(searchText, false, false)) {
            isearchFailed = true;
            emit setStatusBarText(i18n("I-search failed."));
        }
        return true;
    } else if (isearching && !ke->text().length() && !isModified(ke->key())) {
        kdDebug(9000) << "Aborting isearch" << endl;
        isearching = false;
        emit setStatusBarText(QString::fromLatin1(""));
        return false;
    }
    return ReadOnlyPart::eventFilter(o, e);
}


void EditorPart::setWindowCaption(const QString &caption)
{
    widget()->setCaption(caption);
    int pos = caption.findRev('/');
    locationLabel->setText((pos!=-1)? caption.right(caption.length()-pos-1) : caption);
}


void EditorPart::updateOverwriteMode()
{
    overwriteLabel->setText(view->isOverwriteMode()? i18n("OVR") : i18n("INS"));
}


void EditorPart::updateCursorPosition()
{
    int line, col;
    view->getCursorPosition(&line, &col);
    KLocale *loc = KGlobal::locale();
    cursorposLabel->setText(i18n("L%1 C%2")
                            .arg(loc->formatNumber(line+1, 0))
                            .arg(loc->formatNumber(col+1, 0)));
}


void EditorPart::popupMenu(int line, int col)
{
    kdDebug(9000) << "Contextmenu at " << line << "," << col << endl;
    QPopupMenu popup;
    emit contextMenu(&popup, view->textLine(line), col);
    if (popup.count())
        popup.exec(QCursor::pos());
}


void EditorPart::toggleBreakpoint(int lineNum)
{
    emit toggledBreakpoint(editorDocument()->url().path(), lineNum);
}


void EditorPart::editBreakpoint(int lineNum)
{
    emit editedBreakpoint(editorDocument()->url().path(), lineNum);
}


void EditorPart::toggleBreakpointEnabled(int lineNum)
{
    emit toggledBreakpointEnabled(editorDocument()->url().path(), lineNum);
}


void EditorPart::updateUndoAvailable(bool yes)
{
    action("edit_undo")->setEnabled(yes);
    bool either = (yes || action("edit_redo")->isEnabled());
    action("edit_undohistory")->setEnabled(either);
}


void EditorPart::updateRedoAvailable(bool yes)
{
    action("edit_redo")->setEnabled(yes);
    bool either = (yes || action("edit_undo")->isEnabled());
    action("edit_undohistory")->setEnabled(either);
}


void EditorPart::slotPrint()
{
    QString fileName = view->editorDocument()->fileName();
    KTempFile tempFile;
    KShellProcess proc("/bin/sh");
    proc << "a2ps " << fileName << "-o " << tempFile.name();
    proc.start(KProcess::Block);

    QStringList fileList;
    fileList << fileName;
    
//    KPrinter printer;
//    printer.printFiles(fileList, true);
}


void EditorPart::slotUndo()
{
    view->undo();
}


void EditorPart::slotRedo()
{
    view->undo();
}


void EditorPart::slotUndoHistory()
{
    view->undoHistory();
}


void EditorPart::slotCut()
{
    view->cut();
}


void EditorPart::slotCopy()
{
    view->copy();
}


void EditorPart::slotPaste()
{
    view->paste();
}


void EditorPart::slotFind()
{
    view->find();
}


void EditorPart::slotFindAgain()
{
    view->findAgain();
}


void EditorPart::slotReplace()
{
    view->replace();
}

#include "editorpart.moc"
