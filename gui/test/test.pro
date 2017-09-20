#lessThan(QT_MAJOR_VERSION, 5): error(requires >= Qt 5 (You used: $$QT_VERSION))

CONFIG  += ordered
TEMPLATE = subdirs

SUBDIRS = benchmark \
    filelist \
    projectfile \
    translationhandler \
    xmlreport \
    xmlreportv2
