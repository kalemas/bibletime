
CONFIG += clucene

#QT += widgets

DEFINES += BT_MINI

INCLUDEPATH += . \
    ../../../../sword/include \
    ../../../../sword/include/internal/regex \
    ../../../../bt/src/frontend-mini \
    ../../../../bt/src

SOURCES += \
    ../../../../sword/src/modules/common/zipcomprs.cpp \
    ../../../../sword/src/utilfuns/zlib/untgz.c \
    ../../../../sword/src/utilfuns/zlib/gzio.c \
    ../../../../sword/src/utilfuns/zlib/zutil.c \
    ../../../../sword/src/utilfuns/zlib/uncompr.c \
    ../../../../sword/src/utilfuns/zlib/trees.c \
    ../../../../sword/src/utilfuns/zlib/infutil.c \
    ../../../../sword/src/utilfuns/zlib/inftrees.c \
    ../../../../sword/src/utilfuns/zlib/inflate.c \
    ../../../../sword/src/utilfuns/zlib/inffast.c \
    ../../../../sword/src/utilfuns/zlib/infcodes.c \
    ../../../../sword/src/utilfuns/zlib/infblock.c \
    ../../../../sword/src/utilfuns/zlib/deflate.c \
    ../../../../sword/src/utilfuns/zlib/crc32.c \
    ../../../../sword/src/utilfuns/zlib/compress.c \
    ../../../../sword/src/utilfuns/zlib/adler32.c \
    ../../../../sword/src/mgr/ftplibftpt.cpp \
    ../../../../sword/src/utilfuns/ftplib.c \
    ../../../../sword/src/utilfuns/regex.c \
    ../../../../sword/src/frontend/swdisp.cpp \
    ../../../../sword/src/frontend/swlog.cpp \
    ../../../../sword/src/keys/swkey.cpp \
    ../../../../sword/src/keys/listkey.cpp \
    ../../../../sword/src/keys/strkey.cpp \
    ../../../../sword/src/keys/treekey.cpp \
    ../../../../sword/src/keys/treekeyidx.cpp \
    ../../../../sword/src/keys/versekey.cpp \
    ../../../../sword/src/keys/versetreekey.cpp \
    ../../../../sword/src/mgr/swconfig.cpp \
    ../../../../sword/src/mgr/swmgr.cpp \
    ../../../../sword/src/mgr/swfiltermgr.cpp \
    ../../../../sword/src/mgr/encfiltmgr.cpp \
    ../../../../sword/src/mgr/markupfiltmgr.cpp \
    ../../../../sword/src/mgr/filemgr.cpp \
    ../../../../sword/src/mgr/versificationmgr.cpp \
    ../../../../sword/src/mgr/ftptrans.cpp \
    ../../../../sword/src/mgr/swlocale.cpp \
    ../../../../sword/src/mgr/localemgr.cpp \
    ../../../../sword/src/mgr/swcacher.cpp \
    ../../../../sword/src/mgr/swsearchable.cpp \
    ../../../../sword/src/mgr/installmgr.cpp \
    ../../../../sword/src/mgr/stringmgr.cpp \
    ../../../../sword/src/modules/swmodule.cpp \
    ../../../../sword/src/modules/comments/swcom.cpp \
    ../../../../sword/src/modules/comments/hrefcom/hrefcom.cpp \
    ../../../../sword/src/modules/comments/rawcom/rawcom.cpp \
    ../../../../sword/src/modules/comments/rawcom4/rawcom4.cpp \
    ../../../../sword/src/modules/comments/rawfiles/rawfiles.cpp \
    ../../../../sword/src/modules/comments/zcom/zcom.cpp \
    ../../../../sword/src/modules/common/rawstr.cpp \
    ../../../../sword/src/modules/common/rawstr4.cpp \
    ../../../../sword/src/modules/common/swcomprs.cpp \
    ../../../../sword/src/modules/common/lzsscomprs.cpp \
    ../../../../sword/src/modules/common/rawverse.cpp \
    ../../../../sword/src/modules/common/rawverse4.cpp \
    ../../../../sword/src/modules/common/swcipher.cpp \
    ../../../../sword/src/modules/common/zverse.cpp \
    ../../../../sword/src/modules/common/zstr.cpp \
    ../../../../sword/src/modules/common/entriesblk.cpp \
    ../../../../sword/src/modules/common/sapphire.cpp \
    ../../../../sword/src/modules/filters/swbasicfilter.cpp \
    ../../../../sword/src/modules/filters/swoptfilter.cpp \
    ../../../../sword/src/modules/filters/gbfhtml.cpp \
    ../../../../sword/src/modules/filters/gbfxhtml.cpp \
    ../../../../sword/src/modules/filters/gbfhtmlhref.cpp \
    ../../../../sword/src/modules/filters/gbfwebif.cpp \
    ../../../../sword/src/modules/filters/gbfplain.cpp \
    ../../../../sword/src/modules/filters/gbfrtf.cpp \
    ../../../../sword/src/modules/filters/gbfstrongs.cpp \
    ../../../../sword/src/modules/filters/gbffootnotes.cpp \
    ../../../../sword/src/modules/filters/gbfheadings.cpp \
    ../../../../sword/src/modules/filters/gbfredletterwords.cpp \
    ../../../../sword/src/modules/filters/gbfmorph.cpp \
    ../../../../sword/src/modules/filters/gbfwordjs.cpp \
    ../../../../sword/src/modules/filters/thmlstrongs.cpp \
    ../../../../sword/src/modules/filters/thmlfootnotes.cpp \
    ../../../../sword/src/modules/filters/thmlheadings.cpp \
    ../../../../sword/src/modules/filters/thmlmorph.cpp \
    ../../../../sword/src/modules/filters/thmllemma.cpp \
    ../../../../sword/src/modules/filters/thmlscripref.cpp \
    ../../../../sword/src/modules/filters/thmlvariants.cpp \
    ../../../../sword/src/modules/filters/thmlgbf.cpp \
    ../../../../sword/src/modules/filters/thmlrtf.cpp \
    ../../../../sword/src/modules/filters/thmlhtml.cpp \
    ../../../../sword/src/modules/filters/thmlxhtml.cpp \
    ../../../../sword/src/modules/filters/thmlhtmlhref.cpp \
    ../../../../sword/src/modules/filters/thmlwebif.cpp \
    ../../../../sword/src/modules/filters/thmlwordjs.cpp \
    ../../../../sword/src/modules/filters/teiplain.cpp \
    ../../../../sword/src/modules/filters/teirtf.cpp \
    ../../../../sword/src/modules/filters/teihtmlhref.cpp \
    ../../../../sword/src/modules/filters/teixhtml.cpp \
    ../../../../sword/src/modules/filters/gbfthml.cpp \
    ../../../../sword/src/modules/filters/gbfosis.cpp \
    ../../../../sword/src/modules/filters/thmlosis.cpp \
    ../../../../sword/src/modules/filters/thmlplain.cpp \
    ../../../../sword/src/modules/filters/osisosis.cpp \
    ../../../../sword/src/modules/filters/osisheadings.cpp \
    ../../../../sword/src/modules/filters/osisfootnotes.cpp \
    ../../../../sword/src/modules/filters/osishtmlhref.cpp \
    ../../../../sword/src/modules/filters/osisxhtml.cpp \
    ../../../../sword/src/modules/filters/osiswebif.cpp \
    ../../../../sword/src/modules/filters/osismorph.cpp \
    ../../../../sword/src/modules/filters/osisstrongs.cpp \
    ../../../../sword/src/modules/filters/osisplain.cpp \
    ../../../../sword/src/modules/filters/osisrtf.cpp \
    ../../../../sword/src/modules/filters/osislemma.cpp \
    ../../../../sword/src/modules/filters/osisredletterwords.cpp \
    ../../../../sword/src/modules/filters/osisscripref.cpp \
    ../../../../sword/src/modules/filters/osisvariants.cpp \
    ../../../../sword/src/modules/filters/osiswordjs.cpp \
    ../../../../sword/src/modules/filters/osismorphsegmentation.cpp \
    ../../../../sword/src/modules/filters/osisruby.cpp \
    ../../../../sword/src/modules/filters/latin1utf8.cpp \
    ../../../../sword/src/modules/filters/latin1utf16.cpp \
    ../../../../sword/src/modules/filters/utf8utf16.cpp \
    ../../../../sword/src/modules/filters/utf16utf8.cpp \
    ../../../../sword/src/modules/filters/utf8html.cpp \
    ../../../../sword/src/modules/filters/utf8latin1.cpp \
    ../../../../sword/src/modules/filters/utf8cantillation.cpp \
    ../../../../sword/src/modules/filters/utf8hebrewpoints.cpp \
    ../../../../sword/src/modules/filters/utf8arabicpoints.cpp \
    ../../../../sword/src/modules/filters/utf8greekaccents.cpp \
    ../../../../sword/src/modules/filters/cipherfil.cpp \
    ../../../../sword/src/modules/filters/rtfhtml.cpp \
    ../../../../sword/src/modules/filters/plainfootnotes.cpp \
    ../../../../sword/src/modules/filters/plainhtml.cpp \
    ../../../../sword/src/modules/filters/greeklexattribs.cpp \
    ../../../../sword/src/modules/filters/unicodertf.cpp \
    ../../../../sword/src/modules/filters/papyriplain.cpp \
    ../../../../sword/src/modules/genbook/swgenbook.cpp \
    ../../../../sword/src/modules/genbook/rawgenbook/rawgenbook.cpp \
    ../../../../sword/src/modules/lexdict/swld.cpp \
    ../../../../sword/src/modules/lexdict/rawld/rawld.cpp \
    ../../../../sword/src/modules/lexdict/rawld4/rawld4.cpp \
    ../../../../sword/src/modules/lexdict/zld/zld.cpp \
    ../../../../sword/src/modules/texts/swtext.cpp \
    ../../../../sword/src/modules/texts/rawtext/rawtext.cpp \
    ../../../../sword/src/modules/texts/rawtext4/rawtext4.cpp \
    ../../../../sword/src/modules/texts/ztext/ztext.cpp \
    ../../../../sword/src/utilfuns/swobject.cpp \
    ../../../../sword/src/utilfuns/utilstr.cpp \
    ../../../../sword/src/utilfuns/utilxml.cpp \
    ../../../../sword/src/utilfuns/swunicod.cpp \
    ../../../../sword/src/utilfuns/swversion.cpp \
    ../../../../sword/src/utilfuns/swbuf.cpp \
    ../../../../sword/src/utilfuns/ftpparse.c \
    ../../../../sword/src/utilfuns/url.cpp \
    ../../../../sword/src/utilfuns/roman.cpp \
    ../../../../bt/src/frontend-mini/models/btminimoduletextmodel.cpp \
    ../../../../bt/src/frontend-mini/models/btminimodulenavigationmodel.cpp \
    ../../../../bt/src/frontend-mini/models/btminimodelsmodel.cpp \
    ../../../../bt/src/frontend-mini/ui/btministyle.cpp \
    ../../../../bt/src/frontend-mini/ui/btminipanel.cpp \
    ../../../../bt/src/frontend-mini/ui/btminimenu.cpp \
    ../../../../bt/src/frontend-mini/view/btminiview.cpp \
    ../../../../bt/src/frontend-mini/view/btminilayoutdelegate.cpp \
    ../../../../bt/src/frontend-mini/btmini.cpp \
    ../../../../bt/src/backend/cswordmodulesearch.cpp \
    ../../../../bt/src/backend/btmoduletreeitem.cpp \
    ../../../../bt/src/backend/btinstallbackend.cpp \
    ../../../../bt/src/backend/managers/referencemanager.cpp \
    ../../../../bt/src/backend/managers/cswordbackend.cpp \
    ../../../../bt/src/backend/managers/clanguagemgr.cpp \
    ../../../../bt/src/backend/managers/cdisplaytemplatemgr.cpp \
    ../../../../bt/src/backend/managers/btstringmgr.cpp \
    ../../../../bt/src/util/directory.cpp \
    ../../../../bt/src/util/cresmgr.cpp \
    ../../../../bt/src/backend/config/cbtconfig.cpp \
    ../../../../bt/src/backend/bookshelfmodel/btbookshelftreemodel.cpp \
    ../../../../bt/src/frontend/bookshelfmanager/btinstallmgr.cpp \
    ../../../../bt/src/backend/keys/cswordversekey.cpp \
    ../../../../bt/src/backend/keys/cswordldkey.cpp \
    ../../../../bt/src/backend/keys/cswordkey.cpp \
    ../../../../bt/src/backend/drivers/cswordmoduleinfo.cpp \
    ../../../../bt/src/backend/drivers/cswordlexiconmoduleinfo.cpp \
    ../../../../bt/src/backend/drivers/cswordcommentarymoduleinfo.cpp \
    ../../../../bt/src/backend/drivers/cswordbiblemoduleinfo.cpp \
    ../../../../bt/src/backend/rendering/ctextrendering.cpp \
    ../../../../bt/src/backend/rendering/chtmlexportrendering.cpp \
    ../../../../bt/src/backend/rendering/centrydisplay.cpp \
    ../../../../bt/src/backend/rendering/cdisplayrendering.cpp \
    ../../../../bt/src/backend/bookshelfmodel/moduleitem.cpp \
    ../../../../bt/src/backend/bookshelfmodel/languageitem.cpp \
    ../../../../bt/src/backend/bookshelfmodel/item.cpp \
    ../../../../bt/src/backend/bookshelfmodel/categoryitem.cpp \
    ../../../../bt/src/frontend/cinfodisplay.cpp \
    ../../../../bt/src/frontend/bookshelfmanager/installpage/btinstallthread.cpp \
    ../../../../bt/src/util/tool.cpp \
    ../../../../bt/src/util/dialogutil.cpp \
    ../../../../bt/src/backend/rendering/cchapterdisplay.cpp \
    ../../../../bt/src/backend/rendering/cbookdisplay.cpp \
    ../../../../bt/src/backend/filters/thmltohtml.cpp \
    ../../../../bt/src/backend/filters/teitohtml.cpp \
    ../../../../bt/src/backend/filters/plaintohtml.cpp \
    ../../../../bt/src/backend/filters/osistohtml.cpp \
    ../../../../bt/src/backend/filters/gbftohtml.cpp \
    ../../../../bt/src/backend/bookshelfmodel/btbookshelfmodel.cpp \
    ../../../../bt/src/backend/filters/thmltoplain.cpp \
    ../../../../bt/src/backend/drivers/cswordbookmoduleinfo.cpp \
    ../../../../bt/src/frontend/displaywindow/btactioncollection.cpp \
    ../../../../bt/src/backend/bookshelfmodel/indexingitem.cpp \
    ../../../../bt/src/backend/keys/cswordtreekey.cpp \
    ../../../../bt/src/frontend/crossrefrendering.cpp \
    ../../../../bt/src/backend/filters/osismorphsegmentation_bt.cpp


HEADERS += \
    ../../../../bt/src/frontend-mini/models/btminimoduletextmodel.h \
    ../../../../bt/src/frontend-mini/models/btminimodulenavigationmodel.h \
    ../../../../bt/src/frontend-mini/models/btminimodelsmodel.h \
    ../../../../bt/src/frontend-mini/ui/btminipanel.h \
    ../../../../bt/src/frontend-mini/ui/btminimenu.h \
    ../../../../bt/src/frontend-mini/view/btminiview.h \
    ../../../../bt/src/frontend-mini/view/btminilayoutdelegate.h \
    ../../../../bt/src/backend/cswordmodulesearch.h \
    ../../../../bt/src/backend/btmoduletreeitem.h \
    ../../../../bt/src/backend/btinstallbackend.h \
    ../../../../bt/src/backend/managers/referencemanager.h \
    ../../../../bt/src/backend/managers/cswordbackend.h \
    ../../../../bt/src/backend/managers/clanguagemgr.h \
    ../../../../bt/src/backend/managers/cdisplaytemplatemgr.h \
    ../../../../bt/src/backend/managers/btstringmgr.h \
    ../../../../bt/src/util/directory.h \
    ../../../../bt/src/util/cresmgr.h \
    ../../../../bt/src/backend/config/cbtconfig.h \
    ../../../../bt/src/backend/bookshelfmodel/btbookshelftreemodel.h \
    ../../../../bt/src/frontend/bookshelfmanager/btinstallmgr.h \
    ../../../../bt/src/frontend/bookshelfmanager/installpage/btinstallthread.h \
    ../../../../bt/src/frontend/cinfodisplay.h \
    ../../../../bt/src/frontend/displaywindow/btactioncollection.h \
    ../../../../bt/src/frontend/crossrefrendering.h \
    ../../../../bt/src/backend/keys/cswordversekey.h \
    ../../../../bt/src/backend/keys/cswordldkey.h \
    ../../../../bt/src/backend/keys/cswordkey.h \
    ../../../../bt/src/backend/keys/cswordtreekey.h \
    ../../../../bt/src/backend/drivers/cswordmoduleinfo.h \
    ../../../../bt/src/backend/drivers/cswordlexiconmoduleinfo.h \
    ../../../../bt/src/backend/drivers/cswordcommentarymoduleinfo.h \
    ../../../../bt/src/backend/drivers/cswordbiblemoduleinfo.h \
    ../../../../bt/src/backend/rendering/ctextrendering.h \
    ../../../../bt/src/backend/rendering/chtmlexportrendering.h \
    ../../../../bt/src/backend/rendering/centrydisplay.h \
    ../../../../bt/src/backend/rendering/cdisplayrendering.h \
    ../../../../bt/src/backend/rendering/cplaintextexportrendering.h \
    ../../../../bt/src/backend/bookshelfmodel/moduleitem.h \
    ../../../../bt/src/backend/bookshelfmodel/languageitem.h \
    ../../../../bt/src/backend/bookshelfmodel/item.h \
    ../../../../bt/src/backend/bookshelfmodel/categoryitem.h \
    ../../../../bt/src/util/tool.h \
    ../../../../bt/src/util/dialogutil.h \
    ../../../../bt/src/util/btsignal.h \
    ../../../../bt/src/backend/rendering/cchapterdisplay.h \
    ../../../../bt/src/backend/rendering/cbookdisplay.h \
    ../../../../bt/src/backend/filters/thmltohtml.h \
    ../../../../bt/src/backend/filters/teitohtml.h \
    ../../../../bt/src/backend/filters/plaintohtml.h \
    ../../../../bt/src/backend/filters/osistohtml.h \
    ../../../../bt/src/backend/filters/gbftohtml.h \
    ../../../../bt/src/backend/bookshelfmodel/btbookshelfmodel.h \
    ../../../../bt/src/backend/filters/thmltoplain.h \
    ../../../../bt/src/backend/drivers/cswordbookmoduleinfo.h \
    ../../../../bt/src/backend/bookshelfmodel/indexingitem.h \
    ../../../../bt/src/backend/keys/cswordtreekey.h \
    ../../../../bt/src/frontend/crossrefrendering.h \
    ../../../../bt/src/backend/filters/osismorphsegmentation.h \
    ../../../../bt/src/frontend-mini/btmini.h \
    ../../../../bt/src/frontend-mini/ui/btministyle.cpp \
    CLucene/clucene-config.h

win32 {
LIBS += -lws2_32
}

RESOURCES += \
    ../../../../bt/src/frontend-mini/ui/btministyle.qrc \
    ../../../btmini.qrc

OTHER_FILES += ../../../src/frontend-mini/todo.txt \
    android/AndroidManifest.xml \
    android/res/drawable/icon.png \
    android/res/drawable/logo.png \
    android/res/drawable-hdpi/icon.png \
    android/res/drawable-ldpi/icon.png \
    android/res/drawable-mdpi/icon.png \
    android/res/layout/splash.xml \
    android/res/values/libs.xml \
    android/res/values/strings.xml \
    android/res/values-de/strings.xml \
    android/res/values-el/strings.xml \
    android/res/values-es/strings.xml \
    android/res/values-et/strings.xml \
    android/res/values-fa/strings.xml \
    android/res/values-fr/strings.xml \
    android/res/values-id/strings.xml \
    android/res/values-it/strings.xml \
    android/res/values-ja/strings.xml \
    android/res/values-ms/strings.xml \
    android/res/values-nb/strings.xml \
    android/res/values-nl/strings.xml \
    android/res/values-pl/strings.xml \
    android/res/values-pt-rBR/strings.xml \
    android/res/values-ro/strings.xml \
    android/res/values-rs/strings.xml \
    android/res/values-ru/strings.xml \
    android/res/values-zh-rCN/strings.xml \
    android/res/values-zh-rTW/strings.xml \
    android/src/org/kde/necessitas/ministro/IMinistro.aidl \
    android/src/org/kde/necessitas/ministro/IMinistroCallback.aidl \
    android/src/org/kde/necessitas/origo/QtActivity.java \
    android/src/org/kde/necessitas/origo/QtApplication.java \
    android/version.xml

TRANSLATIONS += ../../../src/frontend-mini/translations/bibletimemini_ru.ts


# clucene, could be turned off when porting on new plaftorm
clucene {
DEFINES += _UCS2 _CL_DISABLE_MULTITHREADING

symbian {
DEFINES += __GNUC__
}

INCLUDEPATH += ../../../../clucene/src

SOURCES += \
    ../../../../clucene/src/CLucene/analysis/AnalysisHeader.cpp \
    ../../../../clucene/src/CLucene/analysis/Analyzers.cpp \
    ../../../../clucene/src/CLucene/util/BitSet.cpp \
    ../../../../clucene/src/CLucene/search/BooleanQuery.cpp \
    ../../../../clucene/src/CLucene/search/BooleanScorer.cpp \
    ../../../../clucene/src/CLucene/index/CompoundFile.cpp \
    ../../../../clucene/src/CLucene/debug/condition.cpp \
    ../../../../clucene/src/CLucene/search/ConjunctionScorer.cpp \
    ../../../../clucene/src/CLucene/document/Document.cpp \
    ../../../../clucene/src/CLucene/index/DocumentWriter.cpp \
    ../../../../clucene/src/CLucene/util/Equators.cpp \
    ../../../../clucene/src/CLucene/debug/error.cpp \
    ../../../../clucene/src/CLucene/search/ExactPhraseScorer.cpp \
    ../../../../clucene/src/CLucene/search/Explanation.cpp \
    ../../../../clucene/src/CLucene/util/FastCharStream.cpp \
    ../../../../clucene/src/CLucene/document/Field.cpp \
    ../../../../clucene/src/CLucene/search/FieldCache.cpp \
    ../../../../clucene/src/CLucene/search/FieldCacheImpl.cpp \
    ../../../../clucene/src/CLucene/search/FieldDocSortedHitQueue.cpp \
    ../../../../clucene/src/CLucene/index/FieldInfos.cpp \
    ../../../../clucene/src/CLucene/search/FieldSortedHitQueue.cpp \
    ../../../../clucene/src/CLucene/index/FieldsReader.cpp \
    ../../../../clucene/src/CLucene/index/FieldsWriter.cpp \
    ../../../../clucene/src/CLucene/util/fileinputstream.cpp \
    ../../../../clucene/src/CLucene/search/FilteredTermEnum.cpp \
    ../../../../clucene/src/CLucene/store/FSDirectory.cpp \
    ../../../../clucene/src/CLucene/search/FuzzyQuery.cpp \
    ../../../../clucene/src/CLucene/config/gunichartables.cpp \
    ../../../../clucene/src/CLucene/search/HitQueue.cpp \
    ../../../../clucene/src/CLucene/search/Hits.cpp \
    ../../../../clucene/src/CLucene/store/IndexInput.cpp \
    ../../../../clucene/src/CLucene/store/IndexOutput.cpp \
    ../../../../clucene/src/CLucene/index/IndexReader.cpp \
    ../../../../clucene/src/CLucene/search/IndexSearcher.cpp \
    ../../../../clucene/src/CLucene/index/IndexWriter.cpp \
    ../../../../clucene/src/CLucene/queryParser/Lexer.cpp \
    ../../../../clucene/src/CLucene/store/Lock.cpp \
    ../../../../clucene/src/CLucene/debug/memtracking.cpp \
    ../../../../clucene/src/CLucene/util/Misc.cpp \
    ../../../../clucene/src/CLucene/index/MultiReader.cpp \
    ../../../../clucene/src/CLucene/search/MultiTermQuery.cpp \
    ../../../../clucene/src/CLucene/search/PhrasePositions.cpp \
    ../../../../clucene/src/CLucene/search/PhraseQuery.cpp \
    ../../../../clucene/src/CLucene/search/PhraseScorer.cpp \
    ../../../../clucene/src/CLucene/search/PrefixQuery.cpp \
    ../../../../clucene/src/CLucene/queryParser/QueryParser.cpp \
    ../../../../clucene/src/CLucene/queryParser/QueryParserBase.cpp \
    ../../../../clucene/src/CLucene/queryParser/QueryToken.cpp \
    ../../../../clucene/src/CLucene/store/RAMDirectory.cpp \
    ../../../../clucene/src/CLucene/search/RangeQuery.cpp \
    ../../../../clucene/src/CLucene/util/Reader.cpp \
    ../../../../clucene/src/CLucene/config/repl_tcscasecmp.cpp \
    ../../../../clucene/src/CLucene/config/repl_tcslwr.cpp \
    ../../../../clucene/src/CLucene/search/SearchHeader.cpp \
    ../../../../clucene/src/CLucene/index/SegmentInfos.cpp \
    ../../../../clucene/src/CLucene/index/SegmentMergeInfo.cpp \
    ../../../../clucene/src/CLucene/index/SegmentMergeQueue.cpp \
    ../../../../clucene/src/CLucene/index/SegmentMerger.cpp \
    ../../../../clucene/src/CLucene/index/SegmentReader.cpp \
    ../../../../clucene/src/CLucene/index/SegmentTermDocs.cpp \
    ../../../../clucene/src/CLucene/index/SegmentTermEnum.cpp \
    ../../../../clucene/src/CLucene/index/SegmentTermPositions.cpp \
    ../../../../clucene/src/CLucene/index/SegmentTermVector.cpp \
    ../../../../clucene/src/CLucene/search/Similarity.cpp \
    ../../../../clucene/src/CLucene/search/SloppyPhraseScorer.cpp \
    ../../../../clucene/src/CLucene/search/Sort.cpp \
    ../../../../clucene/src/CLucene/analysis/standard/StandardAnalyzer.cpp \
    ../../../../clucene/src/CLucene/analysis/standard/StandardFilter.cpp \
    ../../../../clucene/src/CLucene/analysis/standard/StandardTokenizer.cpp \
    ../../../../clucene/src/CLucene/StdHeader.cpp \
    ../../../../clucene/src/CLucene/util/StringBuffer.cpp \
    ../../../../clucene/src/CLucene/util/StringIntern.cpp \
    ../../../../clucene/src/CLucene/index/Term.cpp \
    ../../../../clucene/src/CLucene/index/TermInfo.cpp \
    ../../../../clucene/src/CLucene/index/TermInfosReader.cpp \
    ../../../../clucene/src/CLucene/index/TermInfosWriter.cpp \
    ../../../../clucene/src/CLucene/search/TermQuery.cpp \
    ../../../../clucene/src/CLucene/search/TermScorer.cpp \
    ../../../../clucene/src/CLucene/index/TermVectorReader.cpp \
    ../../../../clucene/src/CLucene/index/TermVectorWriter.cpp \
    ../../../../clucene/src/CLucene/util/ThreadLocal.cpp \
    ../../../../clucene/src/CLucene/config/threads.cpp \
    ../../../../clucene/src/CLucene/queryParser/TokenList.cpp \
    ../../../../clucene/src/CLucene/store/TransactionalRAMDirectory.cpp \
    ../../../../clucene/src/CLucene/config/utf8.cpp \
    ../../../../clucene/src/CLucene/search/WildcardQuery.cpp \
    ../../../../clucene/src/CLucene/search/WildcardTermEnum.cpp \
    ../../../../clucene/src/CLucene/config/repl_tcstoll.cpp \
    ../../../../clucene/src/CLucene/config/repl_lltot.cpp \
    ../../../../clucene/src/CLucene/config/repl_tprintf.cpp
}
else {
DEFINES += BT_NO_CLUCENE
}


# meego platform
unix {
include(deployment.pri)
qtcAddDeployment()

OTHER_FILES += \
    qtc_packaging/debian_harmattan/rules \
    qtc_packaging/debian_harmattan/README \
    qtc_packaging/debian_harmattan/manifest.aegis \
    qtc_packaging/debian_harmattan/copyright \
    qtc_packaging/debian_harmattan/control \
    qtc_packaging/debian_harmattan/compat \
    qtc_packaging/debian_harmattan/changelog

contains(MEEGO_EDITION,harmattan) {
    target.path = /opt/btmini/bin
    INSTALLS += target
}
}