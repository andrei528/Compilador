
#ifndef SEMANTICO_H
#define SEMANTICO_H

#include "Token.h"
#include "SemanticError.h"
#include "SemanticTable.h"

#include <string>
#include <vector>
#include <stack>

using namespace std;
class Semantico
{
private:
    enum Tipo {
        TIPO_INT,
        TIPO_FLOAT,
        TIPO_CHAR,
        TIPO_STRING,
        TIPO_BOOL
    };

    enum Modalidade {
        VARIAVEL,
        VETOR,
        PARAMETRO,
        FUNCAO
    };

    struct Simbolo {
        string nome;
        Tipo tipo;
        Modalidade modalidade;
        int escopo;
        bool inicializado;
        bool utilizado;
        int posicao;

        int enderecoMemoria;
        int tamanhoVetor;
        int tamanho;

        // Informações para funções
        vector<Tipo> tiposParametros;
        bool possuiRetorno;
        bool retornoVoid;
    };

    struct Temporario {
        int endereco;
        bool livre;
    };

    struct Operando {
        string valor;
        bool temporario;
        bool imediato;
    };

    vector<string> codigoDados;
    vector<string> codigoTexto;

    vector<Temporario> temporarios;

    //stack<string> pilhaOperandos;
    //stack<string> pilhaOperacoesCodigo;

    stack<Operando> pilhaOperandosCodigo;
    stack<int> pilhaTemporariosOperacao;
    stack<Tipo> pilhaTiposOperandos;
    stack<int> pilhaEnderecosOperacao;

    int obterTemporario();
    void liberarTemporario(int endereco);

    void emitirDados(const string& codigo);
    void emitirInstrucao(const string& codigo);

    string gerarCodigoAssembly() const;

    int proximoEnderecoMemoria;

    string obterRotuloSimbolo(const Simbolo& simbolo) const;

    std::vector<Simbolo> tabela;

    Tipo tipoAtual;
    stack<int> pilhaEscopos;
    int proximoEscopo;

    string identificadorPendente;
    int posicaoIdentificadorPendente;

    string nomeTipo(Tipo tipo) const;
    string nomeModalidade(Modalidade modalidade) const;

    void inserirSimbolo(
        const string &nome,
        int posicao,
        Modalidade modalidade,
        bool inicializado,
        int tamanho = 1
        );

    int buscarSimbolo(const std::string &nome) const;

    string identificadorAtribuicao;
    int posicaoAtribuicao;

    void entrarEscopo();
    void sairEscopo();
    int indiceFuncaoAtual;
    struct ChamadaEmAndamento
    {
        int indiceFuncao;
        vector<Tipo> tiposArgumentos;
    };

    stack<ChamadaEmAndamento> pilhaChamadas;

    Tipo tipoExpressaoAtual;
    bool tipoExpressaoDefinido;

    struct OperacaoPendente
    {
        Tipo tipoEsquerdo;
        SemanticTable::Operations operacao;
    };

    stack<OperacaoPendente> pilhaOperacoes;
    Tipo tipoRetornoFuncaoAtual;


    struct ContextoFuncao {
        int indice;
        Tipo tipoRetorno;
    };

    stack<ContextoFuncao> pilhaFuncoes;

    int tamanhoVetorPendente;
    string vetorAtribuicaoPendente;
    int temporarioIndiceVetor;
    stack<string> pilhaVetoresAcesso;
public:

    struct SimboloVisualizacao {
        string nome;
        string tipo;
        string modalidade;
        int escopo;
        bool inicializado;
        bool utilizado;
        int posicao;
        bool vetor;
        string funcao;
    };

    vector<SimboloVisualizacao> getTabelaVisualizacao() const;

    Semantico();

    void executeAction(int action, const Token *token);

    const std::vector<Simbolo>& getTabela() const;

    void mostrarTabela() const;

    vector<string> avisos;

    void adicionarAviso(const string& mensagem);
    const vector<string>& getAvisos() const;
    void verificarNaoUtilizados();

    bool retornoVoidPendente;

    void empilharOperando(
        const string& valor,
        bool temporario,
        bool imediato
        );

    // Retorna o Assembly BIP completo.
    string getCodigoAssembly() const;

    // Limpa o código gerado para uma nova compilacao.
    void limparCodigoAssembly();

};

#endif

