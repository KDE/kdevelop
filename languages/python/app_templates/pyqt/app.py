#!/usr/bin/env python

#############################################################################
# %{APPNAME} - PyQT application template for KDevelop
#
# Translated from C++ qmakeapp.cpp
# (qmakeapp.cpp - Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.)
#
# This file is part of an example program for Qt.  This example
# program may be used, distributed and modified without limitation.
#
#############################################################################

import sys
import os.path
from qt import *

import documentdetails

false = 0
true = 1

def load_QPixMap(fileName):
    f = open(os.path.join(sys.path[0],fileName),"r")
    result = QPixmap(f.readlines())
    f.close()
    return result

class %{APPNAME}(QMainWindow):
    """An application called %{APPNAME}."""

    def __init__(self):
        QMainWindow.__init__(self, None, "%{APPNAME}")
        self.initIcons()
        self.setup()
        self.initPrinter()
        self.initToolBar()
        self.initMenu()
        self.initMainWidget()
        self.setCaption(self.appTitle)

    def setup(self):

        self.appTitle = "%{APPNAME} - PyQT Application Example"

        self.menu = [
            ('&File',
              [('&New', self.slotNewFile),
               ('&Open...', self.slotOpenFile, self.openIcon),
               ('&Save', self.slotSaveFile, self.saveIcon),
               ('Save &as...', self.slotSaveAs),
               ('&Print...', self.slotPrint, self.printIcon),
               ('&Edit details...', self.slotEditDetails),
               (None,)]),
             ('&Help',
              [('&About', self.slotAbout),
               ('About &QT', self.slotAboutQT),
               (None, ),
               ('What\'s this', self.slotWhatsThis)])
             ]

        self.toolbar = [
            ("filetools", "file operations", "File Operations", 
             [("fileopen",
               (self.openIcon, "open file", "Open File", self.slotOpenFile),
               "<p>Click this button to open a <em>new file</em>. <br>" + \
               "You can also select the <b>Open</b> command " + \
               "from the <b>File</b> menu.</p>"),
              ("filesave",
               (self.saveIcon, "save file", "Save File", self.slotSaveFile),
               "<p>Click this button to save the file you " + \
                 "are editing. You will be prompted for a file name.\n" + \
                 "You can also select the <b>Save</b> command " + \
                 "from the <b>File</b> menu.</p>"),
              ("fileprint",
               (self.printIcon, "print file", "Print File", self.slotPrint),
               "Click this button to print the file you " + \
               "are editing.\n You can also select the Print " + \
               "command from the File menu."
               )])]

        self.fileName = ""


    def initMainWidget(self):
        self.editor = QTextEdit(self, "editor")
        self.editor.setFocus()
        self.setCentralWidget(self.editor)
        self.statusBar().message("Ready", 2000)
        self.resize(450,600)

    def initPrinter(self):
        self.printer = QPrinter()

    def initIcons(self):
        self.openIcon = QIconSet( load_QPixMap("fileopen.pyxpm"))
        self.saveIcon = QIconSet( load_QPixMap("filesave.pyxpm"))
        self.printIcon = QIconSet( load_QPixMap("fileprint.pyxpm"))

    def initToolBar(self):

        self.toolbars = {}
        for (toolBarID, toolBarName, toolBarDescr, toolBarButtons) in self.toolbar:
            self.toolbars[toolBarID] = QToolBar(self, toolBarName)
            self.toolbars[toolBarID].setLabel(toolBarDescr)
            for (buttonID, (buttonIcon, buttonName, buttonText, buttonSlot), whatsThisText) in toolBarButtons:
                toolButton = QToolButton(buttonIcon, buttonText, "", buttonSlot, self.toolbars[toolBarID], buttonName)
                QWhatsThis.add( toolButton, whatsThisText )
            self.whatsThisButton = QWhatsThis.whatsThisButton(self.toolbars[toolBarID])


    def initMenu(self):

        for (menuName, subMenu) in self.menu:
            menu = QPopupMenu(self)
            self.menuBar().insertItem( menuName, menu )
            for menuOption in subMenu:
                if len(menuOption)==1:
                    menu.insertSeparator()
                elif len(menuOption)==2:
                    menu.insertItem( menuOption[0], menuOption[1] )
                elif len(menuOption)==3:
                    menu.insertItem( menuOption[2], \
                                     menuOption[0], \
                                     menuOption[1] )

        

    def slotNewFile(self):
        ed = %{APPNAME}()
        ed.setCaption(self.appTitle)
        ed.show()

    def slotOpenFile(self):
        fileName = str(QFileDialog.getOpenFileName(None, None, self))
        print fileName
        if not fileName=="":
            self.load(fileName)
        else:
            self.statusBar().message("Loading aborted", 2000)

    def slotSaveFile(self):
        if self.fileName=="":
            self.saveAs()
            return
        self.save()


    def slotSaveAs(self):
        pass

    def slotPrint(self):
        margin = 10
        pageno = 1
        if self.printer.setup(self):
            self.statusBar().message("Printing...");
            p = QPainter()
            if not p.begin(self.printer): return

            p.setFont( self.editor.font() )
            yPos = 0
            fontMetrics = p.fontMetrics()
            metrics = QPaintDeviceMetrics(self.printer)
            for i in range(self.editor.lines()):
                if margin + yPos > metrics.height() - margin:
                    pageNo += 1
                    self.statusBar().message("Printing (page " + pageNo + ")...")
                    self.printer.newPage()
                    yPos = 0
                p.drawText( margin, margin+yPos, \
                            metrics.width(), \
                            fontMetrics.lineSpacing(), \
                            QPainter.ExpandTabs | QPainter.DontClip, \
                            self.editor.text(i) )
                yPos += fontMetrics.lineSpacing()
            p.end()
            self.statusBar().message("Printing completed", 2000)
        else:
            self.statusBar().message("Printing aborted", 2000)
            
    def slotClose(self):
        pass
    
    def slotQuit(self):
        pass
        
    def slotEditDetails(self):
        docDet = documentdetails.documentDetails(self)
        docDet.show()

    def slotWhatsThis(self):
        self.whatsThis()

    def slotAbout(self):
        QMessageBox.about(self, self.appTitle, \
                          "This example demonstrates the simple use of\n" + \
                          "QMainWindow, QMenuBar and QToolBar using QT\n" + \
                          "and Python.")

    def slotAboutQT(self):
        QMessageBox.aboutQt(self, self.appTitle)

    def load(self,fileName):
        f = open(fileName,"r")
        if f:
            self.editor.setText( f.read() )
            self.editor.setModified(false)
            self.setCaption(fileName)
            self.statusBar().message("Loaded document " + fileName, 2000)
            self.fileName = fileName
        else:
            self.statusBar().message("Could not load " + fileName, 5000)
            return

    def saveAs(self):
        fileName = str(QFileDialog.getSaveFileName(None, None, self))
        if fileName!="":
            self.fileName = fileName
            self.save()
        else:
            self.statusBar().message("Saving aborted", 2000)

    def save(self):
        if self.fileName=="":
            self.saveAs()
            return
        text = str(self.editor.text())
        try:
            f = open(self.fileName, "w")
            f.write(text)
        except:
            self.statusBar().message("Could not write to " + self.fileName)
            f.close()
            return
        f.close()
        self.editor.setModified(false)
        self.setCaption(self.fileName)
        self.statusBar().message( "File " + self.fileName + " saved", 2000)

    def closeEvent(self, closeEvent):
        if not self.editor.isModified():
            closeEvent.accept()
            return

        yesNoCancel = QMessageBox.information(self, self.appTitle,
                                              "Do you want to save the changes\n" + \
                                              "to the document?", \
                                              "Yes", "No", "Cancel",
                                              0, 1)
        if yesNoCancel == 0:
            self.save()
            closeEvent.accept()
        elif yesNoCancel == 1:
            closeEvent.accept()
        else:
            closeEvent.ignore()
            
                                              


def main(args):
    app=QApplication(args)
    mainWindow = %{APPNAME}()
    mainWindow.show()
    app.connect(app, SIGNAL("lastWindowClosed()"), app, SLOT("quit()"))
    app.exec_loop()



if __name__ == "__main__":
    main(sys.argv)
