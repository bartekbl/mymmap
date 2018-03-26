TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libmymap/release/ -llibmymap
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libmymap/debug/ -llibmymap
else:unix: LIBS += -L$$OUT_PWD/../libmymap/ -llibmymap

INCLUDEPATH += $$PWD/../libmymap
DEPENDPATH += $$PWD/../libmymap
