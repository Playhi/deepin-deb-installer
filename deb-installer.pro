#-------------------------------------------------
#
# Project created by QtCreator 2019-07-31T11:03:10
#
#-------------------------------------------------

QT       += core gui widgets concurrent dtkwidget dtkgui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = deepin-deb-installer
TEMPLATE = app
LIBS += -L /usr/lib
CONFIG += c++11 link_pkgconfig
PKGCONFIG +=  libqapt
# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        debinstaller.cpp \
        deblistmodel.cpp \
        filechoosewidget.cpp \
        infocontrolbutton.cpp \
        main.cpp \
        multipleinstallpage.cpp \
        packagelistview.cpp \
        packageslistdelegate.cpp \
        packagesmanager.cpp \
        singleinstallpage.cpp \
        uninstallconfirmpage.cpp \
        widgets/bluebutton.cpp \
        widgets/graybutton.cpp \
        workerprogress.cpp

HEADERS += \
    debinstaller.h \
    deblistmodel.h \
    environments.h \
    filechoosewidget.h \
    infocontrolbutton.h \
    multipleinstallpage.h \
    packagelistview.h \
    packageslistdelegate.h \
    packagesmanager.h \
    result.h \
    singleinstallpage.h \
    uninstallconfirmpage.h \
    utils.h \
    widgets/bluebutton.h \
    widgets/graybutton.h \
    workerprogress.h

FORMS +=

#!system($$PWD/translate_generation.sh): error("Failed to generate translation")

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    deepin-deb-installer.applications \
    deepin-deb-installer.desktop \
    translations/deepin-deb-installer.qm \
    translations/deepin-deb-installer.ts \
    translations/deepin-deb-installer_am_ET.qm \
    translations/deepin-deb-installer_am_ET.ts \
    translations/deepin-deb-installer_ar.qm \
    translations/deepin-deb-installer_ar.ts \
    translations/deepin-deb-installer_ast.qm \
    translations/deepin-deb-installer_ast.ts \
    translations/deepin-deb-installer_bg.qm \
    translations/deepin-deb-installer_bg.ts \
    translations/deepin-deb-installer_ca.qm \
    translations/deepin-deb-installer_ca.ts \
    translations/deepin-deb-installer_cs.qm \
    translations/deepin-deb-installer_cs.ts \
    translations/deepin-deb-installer_da.qm \
    translations/deepin-deb-installer_da.ts \
    translations/deepin-deb-installer_de.qm \
    translations/deepin-deb-installer_de.ts \
    translations/deepin-deb-installer_en_AU.qm \
    translations/deepin-deb-installer_en_AU.ts \
    translations/deepin-deb-installer_es.qm \
    translations/deepin-deb-installer_es.ts \
    translations/deepin-deb-installer_es_419.qm \
    translations/deepin-deb-installer_es_419.ts \
    translations/deepin-deb-installer_et.qm \
    translations/deepin-deb-installer_et.ts \
    translations/deepin-deb-installer_fi.qm \
    translations/deepin-deb-installer_fi.ts \
    translations/deepin-deb-installer_fr.qm \
    translations/deepin-deb-installer_fr.ts \
    translations/deepin-deb-installer_gl_ES.qm \
    translations/deepin-deb-installer_gl_ES.ts \
    translations/deepin-deb-installer_he.qm \
    translations/deepin-deb-installer_he.ts \
    translations/deepin-deb-installer_hi_IN.qm \
    translations/deepin-deb-installer_hi_IN.ts \
    translations/deepin-deb-installer_hr.qm \
    translations/deepin-deb-installer_hr.ts \
    translations/deepin-deb-installer_hu.qm \
    translations/deepin-deb-installer_hu.ts \
    translations/deepin-deb-installer_id.qm \
    translations/deepin-deb-installer_id.ts \
    translations/deepin-deb-installer_it.qm \
    translations/deepin-deb-installer_it.ts \
    translations/deepin-deb-installer_ko.qm \
    translations/deepin-deb-installer_ko.ts \
    translations/deepin-deb-installer_lt.qm \
    translations/deepin-deb-installer_lt.ts \
    translations/deepin-deb-installer_mn.qm \
    translations/deepin-deb-installer_mn.ts \
    translations/deepin-deb-installer_ms.qm \
    translations/deepin-deb-installer_ms.ts \
    translations/deepin-deb-installer_ne.qm \
    translations/deepin-deb-installer_ne.ts \
    translations/deepin-deb-installer_nl.qm \
    translations/deepin-deb-installer_nl.ts \
    translations/deepin-deb-installer_pa.qm \
    translations/deepin-deb-installer_pa.ts \
    translations/deepin-deb-installer_pl.qm \
    translations/deepin-deb-installer_pl.ts \
    translations/deepin-deb-installer_pt.qm \
    translations/deepin-deb-installer_pt.ts \
    translations/deepin-deb-installer_pt_BR.qm \
    translations/deepin-deb-installer_pt_BR.ts \
    translations/deepin-deb-installer_ru.qm \
    translations/deepin-deb-installer_ru.ts \
    translations/deepin-deb-installer_sk.qm \
    translations/deepin-deb-installer_sk.ts \
    translations/deepin-deb-installer_sl.qm \
    translations/deepin-deb-installer_sl.ts \
    translations/deepin-deb-installer_sr.qm \
    translations/deepin-deb-installer_sr.ts \
    translations/deepin-deb-installer_tr.qm \
    translations/deepin-deb-installer_tr.ts \
    translations/deepin-deb-installer_uk.qm \
    translations/deepin-deb-installer_uk.ts \
    translations/deepin-deb-installer_zh_CN.qm \
    translations/deepin-deb-installer_zh_TW.qm \
    translations/deepin-deb-installer_zh_TW.ts \
    translations/desktop/desktop.ts \
    translations/desktop/desktop_ast.ts \
    translations/desktop/desktop_bg.ts \
    translations/desktop/desktop_ca.ts \
    translations/desktop/desktop_cs.ts \
    translations/desktop/desktop_da.ts \
    translations/desktop/desktop_de.ts \
    translations/desktop/desktop_en_AU.ts \
    translations/desktop/desktop_es.ts \
    translations/desktop/desktop_es_419.ts \
    translations/desktop/desktop_et.ts \
    translations/desktop/desktop_fi.ts \
    translations/desktop/desktop_fr.ts \
    translations/desktop/desktop_hu.ts \
    translations/desktop/desktop_id.ts \
    translations/desktop/desktop_it.ts \
    translations/desktop/desktop_ko.ts \
    translations/desktop/desktop_lt.ts \
    translations/desktop/desktop_mn.ts \
    translations/desktop/desktop_ms.ts \
    translations/desktop/desktop_ne.ts \
    translations/desktop/desktop_nl.ts \
    translations/desktop/desktop_pa.ts \
    translations/desktop/desktop_pl.ts \
    translations/desktop/desktop_pt.ts \
    translations/desktop/desktop_pt_BR.ts \
    translations/desktop/desktop_ru.ts \
    translations/desktop/desktop_sk.ts \
    translations/desktop/desktop_sq.ts \
    translations/desktop/desktop_sr.ts \
    translations/desktop/desktop_tr.ts \
    translations/desktop/desktop_uk.ts \
    translations/desktop/desktop_zh_CN.ts \
    translations/desktop/desktop_zh_TW.ts \
    translations/deepin-deb-installer_zh_CN.ts


RESOURCES += \
    resources/resources.qrc

TRANSLATIONS += ./translations/deepin-deb-installer_zh_CN.ts