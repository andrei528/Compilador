#include "Semantico.h"
#include "Constants.h"
#include "SemanticTable.h"

#include <QDebug>

#include <iostream>

using namespace std;

Semantico::Semantico()
{
    tipoAtual = TIPO_INT;
    proximoEscopo = 0;
    pilhaEscopos.push(proximoEscopo);

    identificadorPendente = "";
    posicaoIdentificadorPendente = -1;

    identificadorAtribuicao = "";
    posicaoAtribuicao = -1;
    indiceFuncaoAtual = -1;
    tipoExpressaoAtual = TIPO_INT;
    tipoExpressaoDefinido = false;
    tipoRetornoFuncaoAtual = TIPO_INT;

    retornoVoidPendente = false;
}

int Semantico::buscarSimbolo(const std::string &nome) const
{
    // Percorre a tabela de trás para frente.
    // O símbolo mais recente de um escopo visível tem prioridade.
    for (int i = static_cast<int>(tabela.size()) - 1; i >= 0; --i)
    {
        if (tabela[i].nome != nome)
            continue;

        // O identificador só pode ser encontrado se
        // seu escopo ainda estiver ativo na pilha.
        stack<int> copia = pilhaEscopos;

        while (!copia.empty())
        {
            if (copia.top() == tabela[i].escopo)
                return i;

            copia.pop();
        }
    }

    return -1;
}
string Semantico::nomeTipo(Tipo tipo) const
{
    switch (tipo)
    {
    case TIPO_INT:    return "int";
    case TIPO_FLOAT:  return "float";
    case TIPO_CHAR:   return "char";
    case TIPO_STRING: return "string";
    case TIPO_BOOL:   return "bool";
    }

    return "desconhecido";
}

string Semantico::nomeModalidade(Modalidade modalidade) const
{
    switch (modalidade)
    {
    case VARIAVEL:  return "variavel";
    case VETOR:     return "vetor";
    case PARAMETRO: return "parametro";
    case FUNCAO:    return "funcao";
    }

    return "desconhecida";
}

void Semantico::inserirSimbolo(
    const string &nome,
    int posicao,
    Modalidade modalidade,
    bool inicializado)

{
    if (nome.empty())
        return;
    for (const Simbolo &simbolo : tabela)
    {
        if (simbolo.nome == nome &&
            simbolo.escopo == pilhaEscopos.top())
        {
            throw SemanticError(
                "Identificador '" + nome +
                    "' ja declarado neste escopo.",
                posicao
                );
        }
    }

    Simbolo novo;

    novo.nome = nome;
    novo.tipo = tipoAtual;
    novo.modalidade = modalidade;
    novo.escopo = pilhaEscopos.top();
    novo.inicializado = inicializado;
    novo.utilizado = false;
    novo.posicao = posicao;
    novo.possuiRetorno = false;
    novo.retornoVoid = false;

    novo.tiposParametros = {};

    tabela.push_back(novo);

    cout << "Cadastrado: " << novo.nome
         << " | Tipo: " << nomeTipo(novo.tipo)
         << " | Modalidade: " << nomeModalidade(novo.modalidade)
         << " | Escopo: " << novo.escopo
         << " | Inicializado: "
         << (novo.inicializado ? "sim" : "nao")
         << endl;
}

void Semantico::executeAction(int action, const Token *token)
{

    if (token == nullptr)
        return;

    switch (action)
    {
    case 1:
        switch (token->getId())
        {
        case t_PR_INT:
            tipoAtual = TIPO_INT;
            retornoVoidPendente = false;
            break;

        case t_PR_FLOAT:
            tipoAtual = TIPO_FLOAT;
            break;

        case t_PR_CHAR:
            tipoAtual = TIPO_CHAR;
            break;

        case t_PR_STRING:
            tipoAtual = TIPO_STRING;
            break;

        case t_PR_BOOL:
            tipoAtual = TIPO_BOOL;
            break;

        default:
            throw SemanticError(
                "Token inesperado na definicao do tipo.",
                token->getPosition()
                );
        }
        break;

    case 2:
        // Variavel sem inicializacao.
        inserirSimbolo(
            identificadorPendente,
            posicaoIdentificadorPendente,
            VARIAVEL,
            false
            );
        break;

    case 3:
        // Variavel com inicializacao.
        inserirSimbolo(
            identificadorPendente,
            posicaoIdentificadorPendente,
            VARIAVEL,
            true
            );
        break;

    case 4:
        // Vetor sem inicializacao.
        inserirSimbolo(
            identificadorPendente,
            posicaoIdentificadorPendente,
            VETOR,
            false
            );
        break;

    case 5:
        // Vetor com inicializacao.
        inserirSimbolo(
            identificadorPendente,
            posicaoIdentificadorPendente,
            VETOR,
            true
            );
        break;

    case 6:
        // Guarda o identificador antes de analisar o restante.
        identificadorPendente = token->getLexeme();
        posicaoIdentificadorPendente = token->getPosition();
        break;

    case 7:
    {
        string nome = token->getLexeme();
        int indice = buscarSimbolo(nome);

        if (indice == -1)
        {
            throw SemanticError(
                "Identificador '" + nome + "' nao declarado.",
                token->getPosition()
                );
        }

        if (tabela[indice].modalidade == FUNCAO)
        {
            throw SemanticError(
                "A funcao '" + nome +
                    "' deve ser utilizada com uma chamada.",
                token->getPosition()
                );
        }

        tabela[indice].utilizado = true;

        if (!tabela[indice].inicializado)
        {
            adicionarAviso(
                "Aviso: identificador '" + nome +
                "' utilizado antes da inicializacao."
                );
        }

        tipoExpressaoAtual = tabela[indice].tipo;
        tipoExpressaoDefinido = true;

        break;
    }


    case 8:
    {
        identificadorAtribuicao = token->getLexeme();
        posicaoAtribuicao = token->getPosition();

        int indice = buscarSimbolo(identificadorAtribuicao);

        if (indice == -1)
        {
            throw SemanticError(
                "Identificador '" + identificadorAtribuicao +
                    "' nao declarado.",
                posicaoAtribuicao
                );
        }

        break;
    }


    case 9:
    {
        int indice = buscarSimbolo(identificadorAtribuicao);

        if (indice != -1)
        {
            int resultado = SemanticTable::atribType(
                static_cast<int>(tabela[indice].tipo),
                static_cast<int>(tipoExpressaoAtual)
                );

            if (resultado == SemanticTable::ERR)
            {
                throw SemanticError(
                    "Tipos incompativeis na atribuicao.",
                    token->getPosition()
                    );
            }
            else if (resultado == SemanticTable::WAR)
            {
                adicionarAviso(
                    "Aviso: Possivel perda de precisao na atribuicao para '" +
                    identificadorAtribuicao + "'."
                    );
            }

            tabela[indice].inicializado = true;
        }

        tipoExpressaoDefinido = false;
        break;
    }

    case 10:
        entrarEscopo();
        break;

    case 11:
        sairEscopo();
        break;

    case 12:
        identificadorPendente = token->getLexeme();
        posicaoIdentificadorPendente = token->getPosition();

        inserirSimbolo(
            identificadorPendente,
            posicaoIdentificadorPendente,
            PARAMETRO,
            true
            );

        if (indiceFuncaoAtual != -1)
        {
            tabela[indiceFuncaoAtual].tiposParametros.push_back(
                tipoAtual
                );
        }
        break;
    case 13:
    {
        identificadorPendente = token->getLexeme();
        posicaoIdentificadorPendente = token->getPosition();

        // Registra a função no escopo atual.
        inserirSimbolo(
            identificadorPendente,
            posicaoIdentificadorPendente,
            FUNCAO,
            true
            );

        tabela.back().retornoVoid = retornoVoidPendente;

        // Preserva o contexto da função anterior.
        pilhaFuncoes.push(ContextoFuncao{
            indiceFuncaoAtual,
            tipoRetornoFuncaoAtual
        });

        // Ativa a nova função.
        indiceFuncaoAtual = static_cast<int>(tabela.size()) - 1;
        tipoRetornoFuncaoAtual = tipoAtual;

        retornoVoidPendente = false;

        break;
    }

    case 14:
    {
        if (indiceFuncaoAtual != -1)
        {
            const Simbolo& funcao = tabela[indiceFuncaoAtual];

            if (!funcao.retornoVoid && !funcao.possuiRetorno)
            {
                throw SemanticError(
                    "A funcao '" + funcao.nome +
                        "' deve possuir um return.",
                    token->getPosition()
                    );
            }
        }

        sairEscopo();

        if (!pilhaFuncoes.empty())
        {
            indiceFuncaoAtual = pilhaFuncoes.top().indice;
            tipoRetornoFuncaoAtual = pilhaFuncoes.top().tipoRetorno;
            pilhaFuncoes.pop();
        }
        else
        {
            indiceFuncaoAtual = -1;
            tipoRetornoFuncaoAtual = TIPO_INT;
        }

        break;
    }

    case 15:
    {
        string nome = token->getLexeme();
        int indice = buscarSimbolo(nome);

        if (indice == -1)
        {
            throw SemanticError(
                "Funcao '" + nome + "' nao declarada.",
                token->getPosition()
                );
        }

        if (tabela[indice].modalidade != FUNCAO)
        {
            throw SemanticError(
                "Identificador '" + nome + "' nao e uma funcao.",
                token->getPosition()
                );
        }

        tabela[indice].utilizado = true;

        ChamadaEmAndamento chamada;
        chamada.indiceFuncao = indice;
        pilhaChamadas.push(chamada);

        break;
    }

    case 16:
    {
        if (!pilhaChamadas.empty() && tipoExpressaoDefinido)
        {
            pilhaChamadas.top().tiposArgumentos.push_back(
                tipoExpressaoAtual
                );
        }

        break;
    }


    case 17:
    {
        if (pilhaChamadas.empty())
            break;

        ChamadaEmAndamento chamada = pilhaChamadas.top();
        pilhaChamadas.pop();

        const Simbolo &funcao = tabela[chamada.indiceFuncao];

        if (chamada.tiposArgumentos.size() != funcao.tiposParametros.size())
        {
            throw SemanticError(
                "Quantidade incorreta de argumentos na chamada da funcao '" +
                    funcao.nome + "'.",
                token->getPosition()
                );
        }

        for (size_t i = 0; i < chamada.tiposArgumentos.size(); i++)
        {
            int resultado = SemanticTable::atribType(
                static_cast<int>(funcao.tiposParametros[i]),
                static_cast<int>(chamada.tiposArgumentos[i])
                );

            if (resultado == SemanticTable::ERR)
            {
                throw SemanticError(
                    "Tipo incompativel no argumento " +
                        to_string(i + 1) +
                        " da chamada da funcao '" + funcao.nome + "'.",
                    token->getPosition()
                    );
            }
            else if (resultado == SemanticTable::WAR)
            {
                adicionarAviso(
                    "Possivel perda de precisao no argumento " +
                    to_string(i + 1) +
                    " da chamada da funcao '" + funcao.nome + "'."
                    );
            }
        }

        tipoExpressaoAtual = funcao.tipo;
        tipoExpressaoDefinido = true;

        break;
    }


    case 18:
        tipoExpressaoAtual = TIPO_INT;
        tipoExpressaoDefinido = true;
        break;

    case 19:
        tipoExpressaoAtual = TIPO_FLOAT;
        tipoExpressaoDefinido = true;
        break;

    case 20:
        tipoExpressaoAtual = TIPO_CHAR;
        tipoExpressaoDefinido = true;
        break;

    case 21:
        tipoExpressaoAtual = TIPO_STRING;
        tipoExpressaoDefinido = true;
        break;

    case 22:
        tipoExpressaoAtual = TIPO_BOOL;
        tipoExpressaoDefinido = true;
        break;

    case 23:
    {
        // Guarda o tipo do operando esquerdo.
        OperacaoPendente op;
        op.tipoEsquerdo = tipoExpressaoAtual;
        op.operacao = SemanticTable::SUM;

        pilhaOperacoes.push(op);
        break;
    }

    case 24:
    {
        if (pilhaOperacoes.empty())
            break;

        switch (token->getId())
        {
        case t_OP_SUM:
            pilhaOperacoes.top().operacao = SemanticTable::SUM;
            break;

        case t_OP_SUB:
            pilhaOperacoes.top().operacao = SemanticTable::SUB;
            break;

        case t_OP_MULT:
            pilhaOperacoes.top().operacao = SemanticTable::MUL;
            break;

        case t_OP_DIV:
            pilhaOperacoes.top().operacao = SemanticTable::DIV;
            break;

        case t_OP_MOD:
            pilhaOperacoes.top().operacao = SemanticTable::MOD;
            break;

        case t_OP_MAIOR:
        case t_OP_MENOR:
        case t_OP_MAIOR_IG:
        case t_OP_MENOR_IG:
        case t_OP_IG:
        case t_OP_DIF:
            pilhaOperacoes.top().operacao = SemanticTable::REL;
            break;

        default:
            break;
        }

        break;
    }

    case 25:
    {
        if (pilhaOperacoes.empty() || !tipoExpressaoDefinido)
            break;

        OperacaoPendente op = pilhaOperacoes.top();
        pilhaOperacoes.pop();

        int resultado = SemanticTable::resultType(
            static_cast<int>(op.tipoEsquerdo),
            static_cast<int>(tipoExpressaoAtual),
            static_cast<int>(op.operacao)
            );

        if (resultado == SemanticTable::ERR)
        {
            throw SemanticError(
                "Tipos incompativeis na expressao.",
                token->getPosition()
                );
        }

        tipoExpressaoAtual = static_cast<Tipo>(resultado);
        tipoExpressaoDefinido = true;

        break;
    }

    case 26:
    {
        if (indiceFuncaoAtual == -1)
        {
            throw SemanticError(
                "Comando return fora de uma funcao.",
                token->getPosition()
                );
        }

        if (tabela[indiceFuncaoAtual].retornoVoid)
        {
            tabela[indiceFuncaoAtual].possuiRetorno = true;
            break;
        }

        throw SemanticError(
            "A funcao '" + tabela[indiceFuncaoAtual].nome +
                "' deve retornar um valor.",
            token->getPosition()
            );
    }

    case 27:
    {
        if (indiceFuncaoAtual == -1)
        {
            throw SemanticError(
                "Comando return fora de uma funcao.",
                token->getPosition()
                );
        }

        if (tabela[indiceFuncaoAtual].retornoVoid)
        {
            throw SemanticError(
                "A funcao '" + tabela[indiceFuncaoAtual].nome +
                    "' nao deve retornar um valor.",
                token->getPosition()
                );
        }

        if (!tipoExpressaoDefinido)
        {
            throw SemanticError(
                "Nao foi possivel determinar o tipo do retorno.",
                token->getPosition()
                );
        }

        int resultado = SemanticTable::atribType(
            static_cast<int>(tabela[indiceFuncaoAtual].tipo),
            static_cast<int>(tipoExpressaoAtual)
            );

        if (resultado == SemanticTable::ERR)
        {
            throw SemanticError(
                "Tipo de retorno incompativel com a funcao '" +
                    tabela[indiceFuncaoAtual].nome + "'.",
                token->getPosition()
                );
        }
        else if (resultado == SemanticTable::WAR)
        {
            adicionarAviso(
                "Aviso: Possivel perda de precisao no retorno da funcao '" +
                tabela[indiceFuncaoAtual].nome + "'."
                );
        }

        tabela[indiceFuncaoAtual].possuiRetorno = true;
        tipoExpressaoDefinido = false;
        break;
    }

    case 28:
    {
        cout << "ACAO 28 EXECUTADA";
        identificadorAtribuicao = token->getLexeme();
        posicaoAtribuicao = token->getPosition();

        int indice = buscarSimbolo(identificadorAtribuicao);

        if (indice == -1)
        {
            throw SemanticError(
                "Identificador '" + identificadorAtribuicao +
                    "' nao declarado.",
                posicaoAtribuicao
                );
        }

        if (tabela[indice].modalidade != VETOR)
        {
            throw SemanticError(
                "Identificador '" + identificadorAtribuicao +
                    "' nao e um vetor.",
                posicaoAtribuicao
                );
        }

        tabela[indice].utilizado = true;
        break;
    }

    case 29:
    {
        retornoVoidPendente = true;
        break;
    }

    case 30:
    {
        if (!tipoExpressaoDefinido)
        {
            throw SemanticError(
                "Nao foi possivel determinar o tipo da condicao.",
                token->getPosition()
                );
        }

        if (tipoExpressaoAtual == TIPO_CHAR ||
            tipoExpressaoAtual == TIPO_STRING)
        {
            throw SemanticError(
                "Tipo incompativel na condicao. Esperado int, float ou bool.",
                token->getPosition()
                );
        }

        break;
    }

    default:
        break;
    }
}

void Semantico::mostrarTabela() const
{
    cout << "\n--- Tabela de Simbolos ---\n";

    cout << "Nome\tTipo\tEscopo\tInicializado\tUtilizado\n";

    for (const Simbolo &simbolo : tabela)
    {
        cout << simbolo.nome << "\t"
             << nomeTipo(simbolo.tipo) << "\t"
             << simbolo.escopo << "\t"
             << (simbolo.inicializado ? "Sim" : "Nao") << "\t\t"
             << (simbolo.utilizado ? "Sim" : "Nao")
             << endl;
    }
}

void Semantico::entrarEscopo()
{
    ++proximoEscopo;
    pilhaEscopos.push(proximoEscopo);

    cout << "Entrou no escopo: " << proximoEscopo << endl;
}

void Semantico::sairEscopo()
{
    if (pilhaEscopos.size() > 1)
    {
        cout << "Saiu do escopo: "
             << pilhaEscopos.top() << endl;

        pilhaEscopos.pop();
    }
}

void Semantico::adicionarAviso(const string& mensagem)
{
    avisos.push_back(mensagem);
}

const vector<string>& Semantico::getAvisos() const
{
    return avisos;
}

void Semantico::verificarNaoUtilizados()
{
    for (const Simbolo& simbolo : tabela)
    {
        if (!simbolo.utilizado &&
            (simbolo.modalidade == VARIAVEL ||
            simbolo.modalidade == PARAMETRO) ||
            simbolo.modalidade == VETOR)
        {
            adicionarAviso(
                "Aviso: identificador '" + simbolo.nome +
                "' declarado, mas nao utilizado."
                );
        }
    }
}

const vector<Semantico::Simbolo>& Semantico::getTabela() const
{
    return tabela;
}

vector<Semantico::SimboloVisualizacao>
Semantico::getTabelaVisualizacao() const
{
    vector<SimboloVisualizacao> resultado;

    for (const Simbolo& simbolo : tabela) {
        SimboloVisualizacao item;

        item.nome = simbolo.nome;
        item.tipo = nomeTipo(simbolo.tipo);
        item.modalidade = nomeModalidade(simbolo.modalidade);
        item.escopo = simbolo.escopo;
        item.inicializado = simbolo.inicializado;
        item.utilizado = simbolo.utilizado;

        item.vetor = (simbolo.modalidade == VETOR);

        if (simbolo.modalidade == FUNCAO) {
            item.funcao = simbolo.retornoVoid
                              ? "void"
                              : nomeTipo(simbolo.tipo);
        } else {
            item.funcao = "-";
        }

        resultado.push_back(item);
    }

    return resultado;
}