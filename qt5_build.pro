#CONFIG+=console
QT+= widgets webview core gui webkit webkitwidgets
DEFINES += HUI_BUILD_CPP
DEFINES += HUI_BACKEND_QT5
DEFINES += HUI_BACKEND_QT5_building_moc
#HEADERS += hui_webview__webkit_qt5.cc
SOURCES += hui_webview__webkit_qt5.cc
TARGET = hui_qt5