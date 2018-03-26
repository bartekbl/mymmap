TEMPLATE = lib
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    libmymap.c \
    libmymap-rbtree.c \
    libmymap-rbtree-dump.c \
    libmymap-rbtree-insert.c \
    libmymap-rbtree-remove.c

HEADERS += \
    libmymap.h \
    libmymap-rbtree.h \
    libmymap-rbtree-dump.h \
    libmymap-rbtree-internal.h
