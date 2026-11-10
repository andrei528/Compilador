#include "Semantico.h"
#include "Constants.h"
#include "SemanticTable.h"

#include <stdexcept>
#include <QDebug>
#include <iostream>

using namespace std;

Semantico::Semantico()
{
    tipoAtual = TIPO_INT;
    proximoEscopo = 0;
    proximoEnderecoMemoria = 0;
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

    temporarios.push_back({1000, true});
    temporarios.push_back({1001, true});
    temporarios.push_back({1002, true});

    tamanhoVetorPendente = 0;
    vetorAtribuicaoPendente = "";
    temporarioIndiceVetor = -1;

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
    bool inicializado,
    int tamanho)

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
    novo.tamanho = tamanho;
    novo.enderecoMemoria = proximoEnderecoMemoria;

    proximoEnderecoMemoria += tamanho;

    novo.tiposParametros = {};

    tabela.push_back(novo);

    // Gera declaracao de memoria.
    if (modalidade == VARIAVEL || modalidade == PARAMETRO)
    {
        emitirDados(obterRotuloSimbolo(novo) + ": 0");
    }
    else if (modalidade == VETOR)
    {
        string declaracao = obterRotuloSimbolo(novo) + ": ";

        for (int i = 0; i < tamanho; i++)
        {
            if (i > 0)
                declaracao += ", ";

            declaracao += "0";
        }

        emitirDados(declaracao);
    }

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
        inserirSimbolo(
            identificadorPendente,
            posicaoIdentificadorPendente,
            VETOR,
            false,
            tamanhoVetorPendente
            );

        tamanhoVetorPendente = 0;
        break;

    case 5:
        inserirSimbolo(
            identificadorPendente,
            posicaoIdentificadorPendente,
            VETOR,
            true,
            tamanhoVetorPendente
            );

        tamanhoVetorPendente = 0;
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

        if (tabela[indice].modalidade == VETOR)
        {
            throw SemanticError(
                "O vetor '" + nome +
                    "' deve ser acessado utilizando um indice.",
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

        empilharOperando(
            obterRotuloSimbolo(tabela[indice]),
            false,
            false
            );

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

        if (tabela[indice].modalidade == FUNCAO)
        {
            throw SemanticError(
                "Nao e possivel atribuir um valor a funcao '" +
                    identificadorAtribuicao + "'.",
                posicaoAtribuicao
                );
        }

        if (tabela[indice].modalidade == VETOR)
        {
            throw SemanticError(
                "O vetor '" + identificadorAtribuicao +
                    "' deve ser acessado utilizando um indice.",
                posicaoAtribuicao
                );
        }

        break;
    }

    case 9:
    {
        if (pilhaOperandosCodigo.empty())
            throw runtime_error("Erro: valor da atribuicao nao encontrado.");

        Operando valor = pilhaOperandosCodigo.top();
        pilhaOperandosCodigo.pop();

        // Primeiro trata atribuicao a vetor.
        if (!vetorAtribuicaoPendente.empty())
        {
            int indice = buscarSimbolo(vetorAtribuicaoPendente);

            if (indice == -1)
                throw runtime_error("Erro: vetor nao encontrado na atribuicao.");

            if (tabela[indice].tipo != tipoExpressaoAtual)
                throw SemanticError(
                    "Tipos incompativeis na atribuicao.",
                    token->getPosition()
                    );

            int temporarioValor = obterTemporario();

            if (valor.imediato)
                emitirInstrucao("LDI " + valor.valor);
            else
                emitirInstrucao("LD " + valor.valor);

            emitirInstrucao("STO " + to_string(temporarioValor));

            emitirInstrucao("LD " + to_string(temporarioIndiceVetor));
            emitirInstrucao("STO $indr");

            emitirInstrucao("LD " + to_string(temporarioValor));
            emitirInstrucao("STOV " + obterRotuloSimbolo(tabela[indice]));

            liberarTemporario(temporarioIndiceVetor);
            liberarTemporario(temporarioValor);

            if (valor.temporario)
                liberarTemporario(stoi(valor.valor));

            tabela[indice].inicializado = true;

            vetorAtribuicaoPendente = "";
            temporarioIndiceVetor = -1;
        }
        else
        {
            // Atribuicao normal a variavel.
            int indice = buscarSimbolo(identificadorAtribuicao);

            if (indice == -1)
                throw SemanticError(
                    "Identificador '" + identificadorAtribuicao + "' nao declarado.",
                    posicaoAtribuicao
                    );

            if (tabela[indice].tipo != tipoExpressaoAtual)
                throw SemanticError(
                    "Tipos incompativeis na atribuicao.",
                    token->getPosition()
                    );

            if (valor.imediato)
                emitirInstrucao("LDI " + valor.valor);
            else
                emitirInstrucao("LD " + valor.valor);

            emitirInstrucao("STO " + obterRotuloSimbolo(tabela[indice]));

            if (valor.temporario)
                liberarTemporario(stoi(valor.valor));

            tabela[indice].inicializado = true;
        }

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
        empilharOperando(token->getLexeme(), false, true);
        break;

    case 19:
        tipoExpressaoAtual = TIPO_FLOAT;
        tipoExpressaoDefinido = true;
        empilharOperando(token->getLexeme(), false, true);
        break;

    case 20:
        tipoExpressaoAtual = TIPO_CHAR;
        tipoExpressaoDefinido = true;
        empilharOperando(token->getLexeme(), false, true);
        break;

    case 21:
        tipoExpressaoAtual = TIPO_STRING;
        tipoExpressaoDefinido = true;
        empilharOperando(token->getLexeme(), false, true);
        break;

    case 22:
        tipoExpressaoAtual = TIPO_BOOL;
        tipoExpressaoDefinido = true;
        empilharOperando(token->getLexeme(), false, true);
        break;

    case 23:
    {
        // Guarda o tipo do operando esquerdo.
        OperacaoPendente op;
        op.tipoEsquerdo = tipoExpressaoAtual;
        op.operacao = SemanticTable::SUM;

        pilhaOperacoes.push(op);

        // Recupera o operando esquerdo para a geração de código.
        if (pilhaOperandosCodigo.empty())
            throw runtime_error("Erro: operando esquerdo nao encontrado.");

        Operando esquerdo = pilhaOperandosCodigo.top();
        pilhaOperandosCodigo.pop();

        int enderecoEsquerdo;

        if (esquerdo.temporario)
        {
            // O resultado de uma operação anterior pode ser reutilizado.
            enderecoEsquerdo = stoi(esquerdo.valor);
        }
        else
        {
            enderecoEsquerdo = obterTemporario();

            if (esquerdo.imediato)
                emitirInstrucao("LDI " + esquerdo.valor);
            else
                emitirInstrucao("LD " + esquerdo.valor);

            emitirInstrucao("STO " + to_string(enderecoEsquerdo));
        }

        pilhaEnderecosOperacao.push(enderecoEsquerdo);

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

        case t_OP_BIT_AND:
            pilhaOperacoes.top().operacao = SemanticTable::BIT_AND;
            break;

        case t_OP_BIT_OR:
            pilhaOperacoes.top().operacao = SemanticTable::BIT_OR;
            break;

        case t_OP_BIT_XOR:
            pilhaOperacoes.top().operacao = SemanticTable::BIT_XOR;
            break;

        case t_OP_BIT_SLL:
            pilhaOperacoes.top().operacao = SemanticTable::BIT_SLL;
            break;

        case t_OP_BIT_SRL:
            pilhaOperacoes.top().operacao = SemanticTable::BIT_SRL;
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

        int resultado;

        if (op.operacao == SemanticTable::BIT_AND ||
            op.operacao == SemanticTable::BIT_OR ||
            op.operacao == SemanticTable::BIT_XOR ||
            op.operacao == SemanticTable::BIT_SLL ||
            op.operacao == SemanticTable::BIT_SRL)
        {
            if (op.tipoEsquerdo != SemanticTable::INT ||
                tipoExpressaoAtual != SemanticTable::INT)
            {
                throw SemanticError(
                    "Operacao bit a bit exige operandos inteiros.",
                    token->getPosition()
                    );
            }

            resultado = SemanticTable::INT;
        }
        else
        {
            resultado = SemanticTable::resultType(
                static_cast<int>(op.tipoEsquerdo),
                static_cast<int>(tipoExpressaoAtual),
                static_cast<int>(op.operacao)
                );
        }

        if (resultado == SemanticTable::ERR)
        {
            throw SemanticError(
                "Tipos incompativeis na expressao.",
                token->getPosition()
                );
        }

        if (pilhaEnderecosOperacao.empty() ||
            pilhaOperandosCodigo.empty())
        {
            throw runtime_error(
                "Erro: operandos insuficientes para gerar a operacao."
                );
        }

        int enderecoEsquerdo = pilhaEnderecosOperacao.top();
        pilhaEnderecosOperacao.pop();

        Operando direito = pilhaOperandosCodigo.top();
        pilhaOperandosCodigo.pop();

        if (op.operacao == SemanticTable::SUM ||
            op.operacao == SemanticTable::SUB ||
            op.operacao == SemanticTable::BIT_AND ||
            op.operacao == SemanticTable::BIT_OR ||
            op.operacao == SemanticTable::BIT_XOR ||
            op.operacao == SemanticTable::BIT_SLL ||
            op.operacao == SemanticTable::BIT_SRL)
        {
            int enderecoDireito = -1;

            bool deslocamento =
                op.operacao == SemanticTable::BIT_SLL ||
                op.operacao == SemanticTable::BIT_SRL;

            if (deslocamento)
            {
                if (!direito.imediato)
                {
                    throw SemanticError(
                        "A quantidade do deslocamento deve ser um valor inteiro constante.",
                        token->getPosition()
                        );
                }
            }
            else
            {
                if (direito.temporario)
                {
                    enderecoDireito = stoi(direito.valor);
                }
                else
                {
                    enderecoDireito = obterTemporario();

                    if (direito.imediato)
                        emitirInstrucao("LDI " + direito.valor);
                    else
                        emitirInstrucao("LD " + direito.valor);

                    emitirInstrucao("STO " + to_string(enderecoDireito));
                }
            }

            emitirInstrucao("LD " + to_string(enderecoEsquerdo));

            if (op.operacao == SemanticTable::SUM)
                emitirInstrucao("ADD " + to_string(enderecoDireito));
            else if (op.operacao == SemanticTable::SUB)
                emitirInstrucao("SUB " + to_string(enderecoDireito));
            else if (op.operacao == SemanticTable::BIT_AND)
                emitirInstrucao("AND " + to_string(enderecoDireito));
            else if (op.operacao == SemanticTable::BIT_OR)
                emitirInstrucao("OR " + to_string(enderecoDireito));
            else if (op.operacao == SemanticTable::BIT_XOR)
                emitirInstrucao("XOR " + to_string(enderecoDireito));
            else if (op.operacao == SemanticTable::BIT_SLL)
                emitirInstrucao("SLL " + direito.valor);
            else if (op.operacao == SemanticTable::BIT_SRL)
                emitirInstrucao("SRL " + direito.valor);

            emitirInstrucao("STO " + to_string(enderecoEsquerdo));

            // Libera o temporário do operando direito.
            if (enderecoDireito != -1 &&
                enderecoDireito != enderecoEsquerdo)
            {
                liberarTemporario(enderecoDireito);
            }

            // O resultado permanece no temporário esquerdo.
            Operando novoOperando;
            novoOperando.valor = to_string(enderecoEsquerdo);
            novoOperando.temporario = true;
            novoOperando.imediato = false;

            pilhaOperandosCodigo.push(novoOperando);
        }
        else
        {
            // Operações relacionais ainda não geram código.
            liberarTemporario(enderecoEsquerdo);

            if (direito.temporario)
                liberarTemporario(stoi(direito.valor));
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
        string nome = token->getLexeme();

        int indice = buscarSimbolo(nome);

        if (indice == -1)
        {
            throw SemanticError(
                "Identificador '" + nome +
                    "' nao declarado.",
                token->getPosition()
                );
        }

        if (tabela[indice].modalidade != VETOR)
        {
            throw SemanticError(
                "Identificador '" + nome +
                    "' nao e um vetor.",
                token->getPosition()
                );
        }

        tabela[indice].utilizado = true;
        pilhaVetoresAcesso.push(nome);

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

    case 31:
    {
        if (pilhaOperandosCodigo.empty())
        {
            throw SemanticError(
                "Tamanho do vetor nao informado.",
                token->getPosition()
                );
        }

        Operando tamanho = pilhaOperandosCodigo.top();
        pilhaOperandosCodigo.pop();

        if (tipoExpressaoAtual != TIPO_INT ||
            tamanho.temporario ||
            !tamanho.imediato)
        {
            throw SemanticError(
                "O tamanho do vetor deve ser um literal inteiro.",
                token->getPosition()
                );
        }

        try
        {
            tamanhoVetorPendente = stoi(tamanho.valor);
        }
        catch (...)
        {
            throw SemanticError(
                "Tamanho do vetor invalido.",
                token->getPosition()
                );
        }

        if (tamanhoVetorPendente <= 0)
        {
            throw SemanticError(
                "O tamanho do vetor deve ser maior que zero.",
                token->getPosition()
                );
        }

        break;
    }

    case 32:
    {
        if (pilhaVetoresAcesso.empty())
        {
            throw runtime_error(
                "Erro: vetor nao encontrado para atribuicao."
                );
        }

        if (pilhaOperandosCodigo.empty())
        {
            throw runtime_error(
                "Erro: indice do vetor nao encontrado."
                );
        }

        string nomeVetor = pilhaVetoresAcesso.top();
        pilhaVetoresAcesso.pop();

        int indice = buscarSimbolo(nomeVetor);

        if (indice == -1)
        {
            throw runtime_error("Erro: vetor nao encontrado.");
        }

        if (tipoExpressaoAtual != TIPO_INT)
        {
            throw SemanticError(
                "O indice do vetor deve ser inteiro.",
                token->getPosition()
                );
        }

        Operando indiceVetor = pilhaOperandosCodigo.top();
        pilhaOperandosCodigo.pop();

        temporarioIndiceVetor = obterTemporario();

        if (indiceVetor.imediato)
            emitirInstrucao("LDI " + indiceVetor.valor);
        else
            emitirInstrucao("LD " + indiceVetor.valor);

        emitirInstrucao(
            "STO " + to_string(temporarioIndiceVetor)
            );

        if (indiceVetor.temporario)
            liberarTemporario(stoi(indiceVetor.valor));

        vetorAtribuicaoPendente = nomeVetor;

        break;
    }

    case 33:
    {
        if (pilhaVetoresAcesso.empty())
        {
            throw runtime_error(
                "Erro: vetor nao encontrado para leitura."
                );
        }

        if (pilhaOperandosCodigo.empty())
        {
            throw runtime_error(
                "Erro: indice do vetor nao encontrado."
                );
        }

        string nomeVetor = pilhaVetoresAcesso.top();
        pilhaVetoresAcesso.pop();

        int indice = buscarSimbolo(nomeVetor);

        if (indice == -1)
        {
            throw runtime_error("Erro: vetor nao encontrado.");
        }

        if (tipoExpressaoAtual != TIPO_INT)
        {
            throw SemanticError(
                "O indice do vetor deve ser inteiro.",
                token->getPosition()
                );
        }

        Operando indiceVetor = pilhaOperandosCodigo.top();
        pilhaOperandosCodigo.pop();

        if (indiceVetor.imediato)
            emitirInstrucao("LDI " + indiceVetor.valor);
        else
            emitirInstrucao("LD " + indiceVetor.valor);

        emitirInstrucao("STO $indr");
        emitirInstrucao(
            "LDV " + obterRotuloSimbolo(tabela[indice])
            );

        int enderecoResultado = obterTemporario();

        emitirInstrucao(
            "STO " + to_string(enderecoResultado)
            );

        if (indiceVetor.temporario)
            liberarTemporario(stoi(indiceVetor.valor));

        empilharOperando(
            to_string(enderecoResultado),
            true,
            false
            );

        tipoExpressaoAtual = tabela[indice].tipo;
        tipoExpressaoDefinido = true;

        break;
    }

    case 34:
    {
        if (pilhaOperandosCodigo.empty())
        {
            throw runtime_error(
                "Erro: operando nao encontrado para saida."
                );
        }

        Operando operando = pilhaOperandosCodigo.top();
        pilhaOperandosCodigo.pop();

        // Carrega o valor no acumulador.
        if (operando.imediato)
        {
            emitirInstrucao("LDI " + operando.valor);
        }
        else
        {
            emitirInstrucao("LD " + operando.valor);
        }

        // Envia o valor para a porta de saida do BIP.
        emitirInstrucao("STO $out_port");

        // Libera o temporario depois de utilizar seu valor.
        if (operando.temporario)
        {
            liberarTemporario(stoi(operando.valor));
        }

        tipoExpressaoDefinido = false;

        break;
    }

    case 35:
    {
        // Entrada em elemento de vetor.
        if (!vetorAtribuicaoPendente.empty())
        {
            int indice = buscarSimbolo(vetorAtribuicaoPendente);

            if (indice == -1)
            {
                throw runtime_error(
                    "Erro: vetor nao encontrado na entrada."
                    );
            }

            // Recebe o valor da porta de entrada.
            int temporarioValor = obterTemporario();

            emitirInstrucao("LD $in_port");
            emitirInstrucao(
                "STO " + to_string(temporarioValor)
                );

            // Recupera o indice preservado anteriormente.
            emitirInstrucao(
                "LD " + to_string(temporarioIndiceVetor)
                );
            emitirInstrucao("STO $indr");

            // Armazena o valor na posicao do vetor.
            emitirInstrucao(
                "LD " + to_string(temporarioValor)
                );
            emitirInstrucao(
                "STOV " + obterRotuloSimbolo(tabela[indice])
                );

            liberarTemporario(temporarioIndiceVetor);
            liberarTemporario(temporarioValor);

            tabela[indice].inicializado = true;

            temporarioIndiceVetor = -1;
            vetorAtribuicaoPendente = "";

        }
        // Entrada em variável simples.
        else
        {
            int indice = buscarSimbolo(identificadorAtribuicao);

            if (indice == -1)
            {
                throw SemanticError(
                    "Identificador '" + identificadorAtribuicao +
                        "' nao declarado.",
                    posicaoAtribuicao
                    );
            }

            if (tabela[indice].modalidade != VARIAVEL &&
                tabela[indice].modalidade != PARAMETRO)
            {
                throw SemanticError(
                    "Entrada invalida para esse identificador.",
                    posicaoAtribuicao
                    );
            }

            emitirInstrucao("LD $in_port");
            emitirInstrucao(
                "STO " + obterRotuloSimbolo(tabela[indice])
                );

            tabela[indice].inicializado = true;

            identificadorAtribuicao = "";
            posicaoAtribuicao = -1;
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
            simbolo.modalidade == PARAMETRO ||
            simbolo.modalidade == VETOR))
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

int Semantico::obterTemporario()
{
    for (Temporario& temp : temporarios)
    {
        if (temp.livre)
        {
            temp.livre = false;
            return temp.endereco;
        }
    }

    throw runtime_error(
        "Erro: os tres temporarios do BIP estao ocupados."
        );
}

void Semantico::liberarTemporario(int endereco)
{
    for (Temporario& temp : temporarios)
    {
        if (temp.endereco == endereco)
        {
            temp.livre = true;
            return;
        }
    }

    throw runtime_error(
        "Erro: endereco temporario invalido."
        );
}

void Semantico::emitirDados(const string& codigo)
{
    codigoDados.push_back(codigo);
}

void Semantico::emitirInstrucao(const string& codigo)
{
    codigoTexto.push_back(codigo);
}

string Semantico::gerarCodigoAssembly() const
{
    string resultado;

    resultado += ".data\n";

    for (const string& dado : codigoDados)
    {
        resultado += "    " + dado + "\n";
    }

    resultado += "\n.text\n";
    resultado += "_PRINCIPAL:\n";

    for (const string& instrucao : codigoTexto)
    {
        resultado += "    " + instrucao + "\n";
    }

    resultado += "    HLT 0\n";

    return resultado;
}

string Semantico::getCodigoAssembly() const
{
    return gerarCodigoAssembly();
}

void Semantico::limparCodigoAssembly()
{
    codigoDados.clear();
    codigoTexto.clear();

    for (Temporario& temp : temporarios)
    {
        temp.livre = true;
    }
}

string Semantico::obterRotuloSimbolo(const Simbolo& simbolo) const
{
    return simbolo.nome + "_E" + to_string(simbolo.escopo);
}

void Semantico::empilharOperando(
    const string& valor,
    bool temporario,
    bool imediato)
{
    Operando operando;

    operando.valor = valor;
    operando.temporario = temporario;
    operando.imediato = imediato;

    pilhaOperandosCodigo.push(operando);
}