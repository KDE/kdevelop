SET(QEDITOR_SUBDIR qeditor)

SUBDIRS( editor-chooser ${QEDITOR_SUBDIR} )



#original Makefile.am contents follow:

### This makes QEditor compilation always active
### if include_qeditor
#QEDITOR_SUBDIR = qeditor
### endif
#
#SUBDIRS = editor-chooser $(QEDITOR_SUBDIR)
