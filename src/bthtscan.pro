QT += bluetooth widgets core
QT -= gui

TARGET = bthtscan
TEMPLATE = app
SOURCES = agent.cpp ble_agent.cpp main.cpp
HEADERS = agent.h ble_agent.h global.h

CONFIG += debug_and_release
CONFIG += cmdline
