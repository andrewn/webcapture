TEMPLATE = app
TARGET = webcapture
SOURCES = webcapture.cpp
QT += webkit network

mac {
  CONFIG -= app_bundle
}

unix {
	LIBS += -L/opt/fontconfig-2.4.2/lib
}