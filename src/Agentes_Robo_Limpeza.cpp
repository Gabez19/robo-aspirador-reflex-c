#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
  #include <cstdlib>
  #define MALLOC(T,n)  ( (T*) std::malloc(sizeof(T)*(n)) )
  #define FREE(p)      std::free(p)
#else
  #include <stdlib.h>
  #define MALLOC(T,n)  ( (T*) malloc(sizeof(T)*(n)) )
  #define FREE(p)      free(p)
#endif

/* ===== Tipos ===== */
typedef struct { int r, c; } Ponto;
typedef enum { LIMPAR=0, MOVER_N, MOVER_S, MOVER_L, MOVER_O, FICAR } Acao;

typedef struct { Acao *v; int cap, ini, sz; } Log;
typedef struct {
    int N, M, T;
    char **g;     /* grade: '.' '*' '#' ; 'S' é lido e convertido para '.' */
    Ponto S;      /* posição inicial */
    int sujeira_total;
} Mapa;

/* ===== VARIÁVEIS GLOBAIS PARA DETECÇÃO DE LOOP ===== */
static int passos_sem_limpar = 0;
static Ponto historico_posicoes[50];
static int idx_historico = 0;

/* ===== I/O auxiliar ===== */
static void flush_stdin(void){
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {}
}
static void espera_enter(void){
    printf("Pressione Enter para continuar...");
    int ch = getchar();
    if (ch != '\n' && ch != EOF) flush_stdin();
}

/* ===== Log ===== */
static void log_init(Log* L, int cap){
    L->v = MALLOC(Acao, cap);
    L->cap = cap; L->ini = 0; L->sz = 0;
}
static void log_push(Log* L, Acao a){
    if (L->cap <= 0) return;
    int pos = (L->ini + L->sz) % L->cap;
    if (L->sz < L->cap) L->sz++;
    else L->ini = (L->ini + 1) % L->cap;
    L->v[pos] = a;
}

/* ===== Ambiente ===== */
static int dentro(const Mapa* M, int r, int c){
    return r>=0 && r<M->N && c>=0 && c<M->M;
}
static int eh_sujo(const Mapa* M, int r, int c){
    return dentro(M,r,c) && M->g[r][c]=='*';
}
static int eh_bloqueio(const Mapa* M, int r, int c){
    return !dentro(M,r,c) || M->g[r][c]=='#';
}

/* ===== DETECÇÃO DE LOOP ===== */
static int posicao_repetida_recentemente(Ponto pos) {
    for (int i = 0; i < (idx_historico < 10 ? idx_historico : 10); i++) {
        int idx = (idx_historico - 1 - i + 50) % 50;
        if (historico_posicoes[idx].r == pos.r && historico_posicoes[idx].c == pos.c) {
            return 1; // encontrou posição repetida nos últimos 10 passos
        }
    }
    return 0;
}

static void adicionar_posicao_historico(Ponto pos) {
    historico_posicoes[idx_historico % 50] = pos;
    idx_historico++;
}

static const char* nome_acao(Acao a){
    switch(a){
        case LIMPAR: return "LIMPAR";
        case MOVER_N: return "MOVER_N (norte)";
        case MOVER_S: return "MOVER_S (sul)";
        case MOVER_L: return "MOVER_L (oeste)";
        case MOVER_O: return "MOVER_O (leste)";
        default: return "FICAR";
    }
}

static void imprimir_mapa(const Mapa* M, Ponto pos){
    printf("\nMapa (R = robo):\n");
    for (int i=0;i<M->N;i++){
        for (int j=0;j<M->M;j++){
            if (i==pos.r && j==pos.c) putchar('R');
            else putchar(M->g[i][j]);
        }
        putchar('\n');
    }
}

/* ===== Política do agente reflex MELHORADA ===== */
static Acao decide_reflex(const Mapa* M, Ponto pos, char* motivo, size_t motivo_len){
    // Regra 1: Se a célula atual é suja -> LIMPAR
    if (M->g[pos.r][pos.c]=='*'){
        snprintf(motivo, motivo_len, "Regra 1: célula atual suja -> LIMPAR.");
        return LIMPAR;
    }

    // Regra 2: Se existe vizinho sujo -> mover até ele
    if (eh_sujo(M,pos.r-1,pos.c)){
        snprintf(motivo, motivo_len, "Regra 2: vizinho sujo ao norte -> mover para N.");
        return MOVER_N;
    }
    if (eh_sujo(M,pos.r+1,pos.c)){
        snprintf(motivo, motivo_len, "Regra 2: vizinho sujo ao sul -> mover para S.");
        return MOVER_S;
    }
    if (eh_sujo(M,pos.r,pos.c-1)){
        snprintf(motivo, motivo_len, "Regra 2: vizinho sujo a oeste -> mover para L.");
        return MOVER_L;
    }
    if (eh_sujo(M,pos.r,pos.c+1)){
        snprintf(motivo, motivo_len, "Regra 2: vizinho sujo a leste -> mover para O.");
        return MOVER_O;
    }

    // ESTRATÉGIA ANTI-LOOP: Se muitos passos sem limpeza, muda comportamento
    if (passos_sem_limpar > 30) {
        // Tenta exploração aleatória/diferente
        if (!eh_bloqueio(M,pos.r-1,pos.c) && !posicao_repetida_recentemente((Ponto){pos.r-1, pos.c})){
            snprintf(motivo, motivo_len, "Anti-loop: exploracao norte.");
            return MOVER_N;
        }
        if (!eh_bloqueio(M,pos.r,pos.c+1) && !posicao_repetida_recentemente((Ponto){pos.r, pos.c+1})){
            snprintf(motivo, motivo_len, "Anti-loop: exploracao leste.");
            return MOVER_O;
        }
        if (!eh_bloqueio(M,pos.r+1,pos.c) && !posicao_repetida_recentemente((Ponto){pos.r+1, pos.c})){
            snprintf(motivo, motivo_len, "Anti-loop: exploracao sul.");
            return MOVER_S;
        }
        if (!eh_bloqueio(M,pos.r,pos.c-1) && !posicao_repetida_recentemente((Ponto){pos.r, pos.c-1})){
            snprintf(motivo, motivo_len, "Anti-loop: exploracao oeste.");
            return MOVER_L;
        }
    }

    // Regra 3: Varredura zig-zag (original)
    if (pos.c % 2 == 0){
        if (!eh_bloqueio(M,pos.r,pos.c+1)){
            snprintf(motivo, motivo_len, "Regra 3 (zig-zag): coluna par -> tentar leste.");
            return MOVER_O;
        } else if (!eh_bloqueio(M,pos.r+1,pos.c)){
            snprintf(motivo, motivo_len, "Regra 3 (zig-zag): leste bloqueado -> descer.");
            return MOVER_S;
        } else if (!eh_bloqueio(M,pos.r,pos.c-1)){
            snprintf(motivo, motivo_len, "Regra 3 (zig-zag): descer bloqueado -> oeste.");
            return MOVER_L;
        }
    } else {
        if (!eh_bloqueio(M,pos.r,pos.c-1)){
            snprintf(motivo, motivo_len, "Regra 3 (zig-zag): coluna impar -> tentar oeste.");
            return MOVER_L;
        } else if (!eh_bloqueio(M,pos.r+1,pos.c)){
            snprintf(motivo, motivo_len, "Regra 3 (zig-zag): oeste bloqueado -> descer.");
            return MOVER_S;
        } else if (!eh_bloqueio(M,pos.r,pos.c+1)){
            snprintf(motivo, motivo_len, "Regra 3 (zig-zag): descer bloqueado -> leste.");
            return MOVER_O;
        }
    }

    // Fallback melhorado
    if (!eh_bloqueio(M,pos.r-1,pos.c)){
        snprintf(motivo, motivo_len, "Fallback: subir (norte).");
        return MOVER_N;
    }
    if (!eh_bloqueio(M,pos.r+1,pos.c)){
        snprintf(motivo, motivo_len, "Fallback: descer (sul).");
        return MOVER_S;
    }
    snprintf(motivo, motivo_len, "Fallback: ficar parado.");
    return FICAR;
}

static int aplicar_acao(Mapa* M, Ponto* pos, Acao a, int* limpezas, int* bloqueios){
    int nr = pos->r, nc = pos->c;
    switch(a){
        case LIMPAR:
            if (M->g[nr][nc]=='*'){
                M->g[nr][nc]='.';
                (*limpezas)++;
                if (M->sujeira_total > 0) M->sujeira_total--;
                passos_sem_limpar = 0; // RESET contador ao limpar
            }
            return 1;
        case MOVER_N: nr--; break;
        case MOVER_S: nr++; break;
        case MOVER_L: nc--; break;
        case MOVER_O: nc++; break;
        case FICAR:   return 1;
    }
    if (eh_bloqueio(M,nr,nc)){ 
        (*bloqueios)++; 
        passos_sem_limpar++; // incrementa se bloqueado
        return 0; 
    }
    
    // Movimento bem-sucedido
    pos->r = nr; pos->c = nc; 
    adicionar_posicao_historico(*pos);
    passos_sem_limpar++; // incrementa (será resetado se limpar)
    return 1;
}

/* ===== Validação de linha de mapa ===== */
static int linha_valida(const char* s, int M){
    if ((int)strlen(s) != M) return 0;
    for (int j=0;j<M;j++){
        char c = s[j];
        if (c!='.' && c!='*' && c!='#' && c!='S') return 0;
    }
    return 1;
}

/* ===== Main (interativo) ===== */
int main(void){
    printf("=== Robo Aspirador (Agente Reflex MELHORADO) ===\n");
    printf("MELHORIAS:\n");
    printf("- Deteccao de loop por falta de progresso (>100 passos sem limpeza)\n");
    printf("- Deteccao de ciclo de posicoes\n");
    printf("- Estrategia anti-loop quando preso\n");
    printf("- Limite maximo de 500 passos de seguranca\n\n");
    
    printf("Instrucoes:\n");
    printf("1) Informe N (linhas), M (colunas) e T (passos maximos).\n");
    printf("2) Em seguida, informe cada uma das N linhas do mapa com %s caracteres:\n", "M");
    printf("   . = vazio limpo   * = sujeira   # = obstaculo   S = inicio do robo\n");
    printf("Exemplo:\n");
    printf("5 7 80\n");
    printf("S..*..#\n..#..*..\n..#....\n..*..#.\n.......\n\n");

    Mapa M; M.g = NULL; M.sujeira_total = 0;
    printf("Digite N M T: ");
    if (scanf("%d %d %d", &M.N, &M.M, &M.T) != 3){
        printf("Entrada invalida. Encerrando.\n");
        return 1;
    }
    if (M.N<=0 || M.M<=0 || M.T<0){
        printf("Parametros invalidos. Encerrando.\n");
        return 1;
    }
    flush_stdin();

    M.g = MALLOC(char*, M.N);
    if (!M.g){ printf("Falha de memoria.\n"); return 1; }

    char *linha = MALLOC(char, (size_t)M.M + 64);
    if (!linha){ printf("Falha de memoria.\n"); return 1; }

    int encontrouS = 0;
    for (int i=0;i<M.N;i++){
        while (1){
            printf("Digite a linha %d (exatos %d chars com . * # S): ", i, M.M);
            if (!fgets(linha, (int)((size_t)M.M + 60), stdin)){
                printf("Falha ao ler a linha. Tente novamente.\n");
                continue;
            }
            /* remove \n se veio no buffer */
            size_t L = strlen(linha);
            if (L>0 && linha[L-1]=='\n') linha[L-1]='\0';

            if (!linha_valida(linha, M.M)){
                printf("Linha invalida. Deve ter %d chars e apenas . * # S.\n", M.M);
                continue;
            }
            break;
        }

        M.g[i] = MALLOC(char, (size_t)M.M + 1);
        strncpy(M.g[i], linha, (size_t)M.M);
        M.g[i][M.M] = '\0';

        for (int j=0;j<M.M;j++){
            if (M.g[i][j]=='S'){
                M.S.r=i; M.S.c=j; M.g[i][j]='.';
                encontrouS = 1;
            } else if (M.g[i][j]=='*'){
                M.sujeira_total++;
            }
        }
    }
    FREE(linha);

    if (!encontrouS){
        /* se nao houver S, escolhe a primeira celula limpa como inicio */
        int set = 0;
        for (int i=0;i<M.N && !set;i++)
            for (int j=0;j<M.M && !set;j++)
                if (M.g[i][j]=='.'){ M.S.r=i; M.S.c=j; set=1; }
        if (!set){ M.S.r=0; M.S.c=0; } /* fallback */
        printf("Aviso: nenhuma posicao 'S' encontrada; inicio ajustado para (%d,%d).\n", M.S.r, M.S.c);
    }

    printf("\nDeseja modo passo-a-passo? (1=sim, 0=nao): ");
    int passo = 0;
    if (scanf("%d", &passo)!=1){ passo=0; }
    flush_stdin();

    const int sujeira_inicial = M.sujeira_total;
    Ponto pos = M.S;
    Log L; log_init(&L, 64);
    int limpezas=0, bloqueios=0, passos=0;
    
    // RESET variáveis globais
    passos_sem_limpar = 0;
    idx_historico = 0;

    if (passo){
        printf("\nLegenda: . limpo   * sujo   # obstaculo   R robo\n");
        imprimir_mapa(&M, pos);
        printf("Sujeira inicial: %d\n", M.sujeira_total);
        espera_enter();
    }

    clock_t t0 = clock();
    
    // LIMITE DE SEGURANÇA: máximo 500 passos ou T (o que for menor)
    int limite_seguranca = (M.T > 500) ? 500 : M.T;
    
    for (int t=0; t<limite_seguranca; t++){
        // Condições de parada
        if (M.sujeira_total==0) {
            printf("Sucesso! Toda sujeira foi removida.\n");
            break;
        }
        
        if (passos_sem_limpar > 100) {
            printf("Parou por falta de progresso (>100 passos sem limpeza).\n");
            break;
        }

        char motivo[128];
        Acao a = decide_reflex(&M, pos, motivo, sizeof(motivo));
        int ok = aplicar_acao(&M, &pos, a, &limpezas, &bloqueios);
        log_push(&L, a);
        passos++;

        if (passo){
            printf("\nPasso %d | Acao: %s | %s | %s\n",
                   passos, nome_acao(a), motivo, ok? "ok" : "bloqueado");
            printf("Passos sem limpeza: %d\n", passos_sem_limpar);
            imprimir_mapa(&M, pos);
            printf("Sujeira restante: %d | Limpezas: %d | Tentativas bloqueadas: %d\n",
                   M.sujeira_total, limpezas, bloqueios);
            espera_enter();
        }
    }
    clock_t t1 = clock();
    double cpu = (double)(t1 - t0) / CLOCKS_PER_SEC;

    printf("\n=== Resultado da simulacao ===\n");
    printf("Passos executados: %d (limite T=%d, seguranca=%d)\n", passos, M.T, limite_seguranca);
    printf("Limpezas realizadas: %d\n", limpezas);
    printf("Tentativas bloqueadas: %d\n", bloqueios);
    printf("Passos sem limpeza no final: %d\n", passos_sem_limpar);
    printf("Sujeira inicial=%d | restante=%d | removida=%.1f%%\n",
           sujeira_inicial, M.sujeira_total,
           sujeira_inicial ? 100.0*(sujeira_inicial - M.sujeira_total)/sujeira_inicial : 100.0);
    printf("Tempo de CPU: %.6fs\n", cpu);

    // Diagnóstico adicional
    if (M.sujeira_total > 0 && passos_sem_limpar > 100) {
        printf("\n=== DIAGNOSTICO ===\n");
        printf("PROBLEMA: Robo ficou preso em loop!\n");
        printf("CAUSA PROVAVEL: Sujeira inacessivel ou estrategia inadequada.\n");
        printf("SUGESTAO: Verifique se ha sujeira cercada por obstaculos.\n");
    }

    printf("\nDeseja ver o mapa final? (1=sim, 0=nao): ");
    int ver_final=0; if (scanf("%d", &ver_final)!=1) ver_final=0;
    if (ver_final){
        imprimir_mapa(&M, pos);
        printf("Posicao final do robo: (%d,%d)\n", pos.r, pos.c);
    }

    for (int i=0;i<M.N;i++) FREE(M.g[i]);
    FREE(M.g);
    FREE(L.v);
    return 0;
}