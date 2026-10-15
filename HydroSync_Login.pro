QT += core gui widgets

CONFIG += c++17

TEMPLATE = app
TARGET = HydroSync_Login

# Évite les problèmes d'accents avec le compilateur Microsoft
msvc: QMAKE_CXXFLAGS += /utf-8

# ---- Fichiers propres au projet LOCAL (ne pas copier dans HydroSync.pro) ----
SOURCES += \
    main.cpp \
    login.cpp

HEADERS += \
    login.h

FORMS += \
    login.ui

# ---- Module Login : à AJOUTER dans HydroSync.pro lors de l'intégration ----
SOURCES += \
    loginfondwidget.cpp

HEADERS += \
    loginfondwidget.h

RESOURCES += \
    login.qrc
