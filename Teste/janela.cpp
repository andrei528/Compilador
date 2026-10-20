#include "janela.h"
#include "ui_janela.h"

#include <QDebug>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QStringList>
#include <QVBoxLayout>
#include <QHeaderView>

#include "Lexico.h"
#include "Sintatico.h"
#include "Semantico.h"

Janela::Janela(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Janela)
{
    ui->setupUi(this);

    // Layout da aba de saída
    QVBoxLayout *layoutSaida = new QVBoxLayout(ui->tab);
    layoutSaida->setContentsMargins(0, 0, 0, 0);
    layoutSaida->addWidget(ui->saida);

    // Layout da aba da tabela de símbolos
    QVBoxLayout *layoutTabela = new QVBoxLayout(ui->tab_2);
    layoutTabela->setContentsMargins(0, 0, 0, 0);
    layoutTabela->addWidget(ui->tableWidget);

    // Permite que os componentes cresçam junto com as abas
    ui->saida->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Expanding
        );

    ui->tableWidget->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Expanding
        );

    // Faz a saída e a tabela ocuparem todo o espaço disponível.
    ui->saida->setMinimumSize(0, 0);
    ui->tableWidget->setMinimumSize(0, 0);

    ui->saida->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Expanding
        );

    ui->tableWidget->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Expanding
        );

    // Faz as colunas da tabela aproveitarem a largura disponível.
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch
        );

    ui->tableWidget->setColumnCount(8);

    ui->tableWidget->setHorizontalHeaderLabels(
        QStringList()
        << "Nome"
        << "Tipo"
        << "Modalidade"
        << "Escopo"
        << "Inicializado"
        << "Utilizado"
        << "É vetor?"
        << "Função"
        );

    ui->tableWidget->setEditTriggers(
        QAbstractItemView::NoEditTriggers
        );

    ui->tableWidget->setSelectionBehavior(
        QAbstractItemView::SelectRows
        );

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents
        );
    connect(ui->botaoCompilar, &QPushButton::clicked, this, &Janela::tratarCliqueBotao);

}
Janela::~Janela()
{
    delete ui;
}

void Janela::tratarCliqueBotao() {

    ui->tableWidget->setRowCount(0);

    QString codigo = ui->entrada->toPlainText();
    try{
        Lexico lexico(codigo.toStdString().c_str());
        Sintatico sintatico;
        Semantico semantico;

        sintatico.parse(&lexico, &semantico);

        semantico.verificarNaoUtilizados();

        semantico.mostrarTabela();

        const auto simbolos = semantico.getTabelaVisualizacao();

        ui->tableWidget->setRowCount(
            static_cast<int>(simbolos.size())
            );

        for (int i = 0; i < static_cast<int>(simbolos.size()); ++i) {
            const auto& simbolo = simbolos[i];

            ui->tableWidget->setItem(
                i, 0,
                new QTableWidgetItem(
                    QString::fromStdString(simbolo.nome)
                    )
                );

            ui->tableWidget->setItem(
                i, 1,
                new QTableWidgetItem(
                    QString::fromStdString(simbolo.tipo)
                    )
                );

            ui->tableWidget->setItem(
                i, 2,
                new QTableWidgetItem(
                    QString::fromStdString(simbolo.modalidade)
                    )
                );

            ui->tableWidget->setItem(
                i, 3,
                new QTableWidgetItem(
                    QString::number(simbolo.escopo)
                    )
                );

            ui->tableWidget->setItem(
                i, 4,
                new QTableWidgetItem(
                    simbolo.inicializado ? "Sim" : "Não"
                    )
                );

            ui->tableWidget->setItem(
                i, 5,
                new QTableWidgetItem(
                    simbolo.utilizado ? "Sim" : "Não"
                    )
                );
            ui->tableWidget->setItem(
                i, 6,
                new QTableWidgetItem(
                    simbolo.vetor ? "Sim" : "Não"
                    )
                );

            ui->tableWidget->setItem(
                i, 7,
                new QTableWidgetItem(
                    QString::fromStdString(simbolo.funcao)
                    )
                );
        }

        QString resultado = "Compilação realizada com sucesso!";

        for (const string& aviso : semantico.getAvisos())
        {
            resultado += "\n" + QString::fromStdString(aviso);
        }



        ui->saida->setPlainText(resultado);
    }
    catch (LexicalError &erro){
        ui->saida->setPlainText(
            QString("Erro léxico: %1\nPosição: %2")
                .arg(erro.getMessage())
                .arg(erro.getPosition())
        );
    }
    catch (SyntacticError &erro){
        ui->saida->setPlainText(
            QString("Erro sintático: %1\nPosição: %2")
                .arg(erro.getMessage())
                .arg(erro.getPosition())
        );
    }
    catch (SemanticError &erro){
        ui->saida->setPlainText(
            QString("Erro semântico: %1\nPosição: %2")
                .arg(erro.getMessage())
                .arg(erro.getPosition())
        );
    }
}

