TEMPLATE = app
TARGET = webcapture
SOURCES = webcapture.cpp
QT += webkit network
mac {
  CONFIG -= app_bundle
}