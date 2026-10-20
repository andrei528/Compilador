```qmake
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Teste
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        SemanticTable.cpp \
        main.cpp \
        janela.cpp \
        Lexico.cpp \
        Sintatico.cpp \
        Semantico.cpp \
        Constants.cpp

HEADERS += \
        SemanticTable.h \
        janela.h \
        Lexico.h \
        Sintatico.h \
        Semantico.h \
        Constants.h \
        Token.h \
        AnalysisError.h \
        LexicalError.h \
        SyntacticError.h \
        SemanticError.h

FORMS += \
        janela.ui

DISTFILES += \
        Makefile \
        Makefile.Debug \
        Makefile.Release
```