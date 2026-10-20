/********************************************************************************
** Form generated from reading UI file 'janela.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_JANELA_H
#define UI_JANELA_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Janela
{
public:
    QWidget *centralWidget;
    QPlainTextEdit *entrada;
    QWidget *widget;
    QHBoxLayout *horizontalLayout;
    QTabWidget *tabWidget;
    QWidget *tab;
    QTextEdit *textEdit;
    QWidget *tab_2;
    QTableWidget *tableWidget;
    QPushButton *botaoCompilar;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *Janela)
    {
        if (Janela->objectName().isEmpty())
            Janela->setObjectName("Janela");
        Janela->resize(599, 598);
        QFont font;
        font.setPointSize(14);
        Janela->setFont(font);
        centralWidget = new QWidget(Janela);
        centralWidget->setObjectName("centralWidget");
        entrada = new QPlainTextEdit(centralWidget);
        entrada->setObjectName("entrada");
        entrada->setGeometry(QRect(9, 9, 581, 301));
        widget = new QWidget(centralWidget);
        widget->setObjectName("widget");
        widget->setGeometry(QRect(9, 319, 581, 211));
        horizontalLayout = new QHBoxLayout(widget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        tabWidget = new QTabWidget(widget);
        tabWidget->setObjectName("tabWidget");
        tab = new QWidget();
        tab->setObjectName("tab");
        textEdit = new QTextEdit(tab);
        textEdit->setObjectName("textEdit");
        textEdit->setGeometry(QRect(0, 0, 478, 169));
        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName("tab_2");
        tableWidget = new QTableWidget(tab_2);
        tableWidget->setObjectName("tableWidget");
        tableWidget->setGeometry(QRect(0, 0, 478, 169));
        tabWidget->addTab(tab_2, QString());

        horizontalLayout->addWidget(tabWidget);

        botaoCompilar = new QPushButton(widget);
        botaoCompilar->setObjectName("botaoCompilar");

        horizontalLayout->addWidget(botaoCompilar);

        Janela->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(Janela);
        menuBar->setObjectName("menuBar");
        menuBar->setGeometry(QRect(0, 0, 599, 31));
        Janela->setMenuBar(menuBar);
        mainToolBar = new QToolBar(Janela);
        mainToolBar->setObjectName("mainToolBar");
        Janela->addToolBar(Qt::ToolBarArea::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(Janela);
        statusBar->setObjectName("statusBar");
        Janela->setStatusBar(statusBar);

        retranslateUi(Janela);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(Janela);
    } // setupUi

    void retranslateUi(QMainWindow *Janela)
    {
        Janela->setWindowTitle(QCoreApplication::translate("Janela", "Janela", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QCoreApplication::translate("Janela", "Tab 1", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QCoreApplication::translate("Janela", "Tab 2", nullptr));
        botaoCompilar->setText(QCoreApplication::translate("Janela", "TESTE123", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Janela: public Ui_Janela {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_JANELA_H
