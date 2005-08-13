
EXEC_PROGRAM(uic ARGS
   -nounload -tr tr2i18n
   -impl ${KDE_UIC_H_FILE}
   ${KDE_UIC_FILE}
   OUTPUT_VARIABLE _uic_CONTENTS
)

STRING(REGEX REPLACE "tr2i18n\\(\"\"\\)" "QString::null" _uic_CONTENTS "${_uic_CONTENTS}" )
STRING(REGEX REPLACE "tr2i18n\\(\"\", \"\"\\)" "QString::null" _uic_CONTENTS "${_uic_CONTENTS}" )

#	$(PERL) -pe "s,image([0-9][0-9]*)_data,img\$$1_editbookmarkdlg,g" >> editbookmarkdlg.cpp ;\
#	rm -f editbookmarkdlg.cpp.temp ;\

FILE(WRITE ${KDE_UIC_CPP_FILE} "#include <kdialog.h>\n#include <klocale.h>\n\n")
FILE(APPEND ${KDE_UIC_CPP_FILE} "${_uic_CONTENTS}")

