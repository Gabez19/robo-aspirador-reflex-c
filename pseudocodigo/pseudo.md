# Pseudocódigo: Robô Aspirador Agente Reflex

## Estruturas de Dados

```
ESTRUTURA Ponto:
    inteiro r, c  // linha e coluna

ENUMERAÇÃO Acao:
    LIMPAR, MOVER_N, MOVER_S, MOVER_L, MOVER_O, FICAR

ESTRUTURA Log:
    vetor[Acao] v
    inteiro cap, ini, sz  // capacidade, início, tamanho

ESTRUTURA Mapa:
    inteiro N, M, T  // linhas, colunas, tempo máximo
    matriz[char] g   // grade do ambiente
    Ponto S          // posição inicial
    inteiro sujeira_total
```

## Funções Auxiliares

```
FUNÇÃO dentro(mapa, r, c):
    RETORNA r >= 0 E r < mapa.N E c >= 0 E c < mapa.M

FUNÇÃO eh_sujo(mapa, r, c):
    RETORNA dentro(mapa, r, c) E mapa.g[r][c] == '*'

FUNÇÃO eh_bloqueio(mapa, r, c):
    RETORNA NÃO dentro(mapa, r, c) OU mapa.g[r][c] == '#'

FUNÇÃO nome_acao(acao):
    ESCOLHA acao:
        CASO LIMPAR: RETORNA "LIMPAR"
        CASO MOVER_N: RETORNA "MOVER_N (norte)"
        CASO MOVER_S: RETORNA "MOVER_S (sul)"
        CASO MOVER_L: RETORNA "MOVER_L (oeste)"
        CASO MOVER_O: RETORNA "MOVER_O (leste)"
        PADRÃO: RETORNA "FICAR"

FUNÇÃO imprimir_mapa(mapa, posicao_robo):
    PARA i = 0 ATÉ mapa.N-1:
        PARA j = 0 ATÉ mapa.M-1:
            SE i == posicao_robo.r E j == posicao_robo.c:
                IMPRIMA 'R'
            SENÃO:
                IMPRIMA mapa.g[i][j]
        IMPRIMA nova_linha
```

## Núcleo do Agente: Função de Decisão

```
FUNÇÃO decide_reflex(mapa, posicao, REF motivo):
    // REGRA 1: Limpar se célula atual está suja
    SE mapa.g[posicao.r][posicao.c] == '*':
        motivo = "Regra 1: célula atual suja -> LIMPAR"
        RETORNA LIMPAR
    
    // REGRA 2: Mover para vizinho sujo (prioridade: N, S, L, O)
    SE eh_sujo(mapa, posicao.r-1, posicao.c):
        motivo = "Regra 2: vizinho sujo ao norte -> mover para N"
        RETORNA MOVER_N
    
    SE eh_sujo(mapa, posicao.r+1, posicao.c):
        motivo = "Regra 2: vizinho sujo ao sul -> mover para S"
        RETORNA MOVER_S
    
    SE eh_sujo(mapa, posicao.r, posicao.c-1):
        motivo = "Regra 2: vizinho sujo a oeste -> mover para L"
        RETORNA MOVER_L
    
    SE eh_sujo(mapa, posicao.r, posicao.c+1):
        motivo = "Regra 2: vizinho sujo a leste -> mover para O"
        RETORNA MOVER_O
    
    // REGRA 3: Varredura zig-zag por colunas
    SE posicao.c % 2 == 0:  // Coluna par
        SE NÃO eh_bloqueio(mapa, posicao.r, posicao.c+1):
            motivo = "Regra 3: coluna par -> tentar leste"
            RETORNA MOVER_O
        SE NÃO eh_bloqueio(mapa, posicao.r+1, posicao.c):
            motivo = "Regra 3: leste bloqueado -> descer"
            RETORNA MOVER_S
        SE NÃO eh_bloqueio(mapa, posicao.r, posicao.c-1):
            motivo = "Regra 3: descer bloqueado -> oeste"
            RETORNA MOVER_L
    SENÃO:  // Coluna ímpar
        SE NÃO eh_bloqueio(mapa, posicao.r, posicao.c-1):
            motivo = "Regra 3: coluna ímpar -> tentar oeste"
            RETORNA MOVER_L
        SE NÃO eh_bloqueio(mapa, posicao.r+1, posicao.c):
            motivo = "Regra 3: oeste bloqueado -> descer"
            RETORNA MOVER_S
        SE NÃO eh_bloqueio(mapa, posicao.r, posicao.c+1):
            motivo = "Regra 3: descer bloqueado -> leste"
            RETORNA MOVER_O
    
    // FALLBACK: Tenta movimentos básicos
    SE NÃO eh_bloqueio(mapa, posicao.r-1, posicao.c):
        motivo = "Fallback: subir (norte)"
        RETORNA MOVER_N
    
    SE NÃO eh_bloqueio(mapa, posicao.r+1, posicao.c):
        motivo = "Fallback: descer (sul)"
        RETORNA MOVER_S
    
    SE NÃO eh_bloqueio(mapa, posicao.r, posicao.c-1):
        motivo = "Fallback: oeste"
        RETORNA MOVER_L
    
    SE NÃO eh_bloqueio(mapa, posicao.r, posicao.c+1):
        motivo = "Fallback: leste"
        RETORNA MOVER_O
    
    motivo = "Fallback: ficar parado"
    RETORNA FICAR
```

## Função de Execução de Ação

```
FUNÇÃO aplicar_acao(REF mapa, REF posicao, acao, REF limpezas, REF bloqueios):
    nova_r = posicao.r
    nova_c = posicao.c
    
    ESCOLHA acao:
        CASO LIMPAR:
            SE mapa.g[nova_r][nova_c] == '*':
                mapa.g[nova_r][nova_c] = '.'
                limpezas++
                SE mapa.sujeira_total > 0:
                    mapa.sujeira_total--
            RETORNA verdadeiro
        
        CASO MOVER_N: nova_r--
        CASO MOVER_S: nova_r++
        CASO MOVER_L: nova_c--
        CASO MOVER_O: nova_c++
        CASO FICAR: RETORNA verdadeiro
    
    SE eh_bloqueio(mapa, nova_r, nova_c):
        bloqueios++
        RETORNA falso
    
    posicao.r = nova_r
    posicao.c = nova_c
    RETORNA verdadeiro
```

## Validação de Entrada

```
FUNÇÃO linha_valida(linha, tamanho_esperado):
    SE comprimento(linha) != tamanho_esperado:
        RETORNA falso
    
    PARA cada caractere c em linha:
        SE c não está em {'.', '*', '#', 'S'}:
            RETORNA falso
    
    RETORNA verdadeiro
```

## Programa Principal

```
PROGRAMA PRINCIPAL:
    IMPRIMA instruções do programa
    
    // Leitura dos parâmetros
    LEIA N, M, T
    SE N <= 0 OU M <= 0 OU T < 0:
        IMPRIMA "Parâmetros inválidos"
        TERMINA
    
    // Inicialização do mapa
    mapa = novo Mapa(N, M, T)
    mapa.sujeira_total = 0
    encontrou_S = falso
    
    // Leitura do mapa linha por linha
    PARA i = 0 ATÉ N-1:
        REPITA:
            IMPRIMA "Digite a linha", i
            LEIA linha
            remova_quebra_linha(linha)
        ATÉ linha_valida(linha, M)
        
        mapa.g[i] = copia(linha)
        
        // Processa cada célula da linha
        PARA j = 0 ATÉ M-1:
            SE mapa.g[i][j] == 'S':
                mapa.S = Ponto(i, j)
                mapa.g[i][j] = '.'
                encontrou_S = verdadeiro
            SENÃO SE mapa.g[i][j] == '*':
                mapa.sujeira_total++
    
    // Define posição inicial se 'S' não foi encontrado
    SE NÃO encontrou_S:
        PARA i = 0 ATÉ N-1:
            PARA j = 0 ATÉ M-1:
                SE mapa.g[i][j] == '.':
                    mapa.S = Ponto(i, j)
                    PARE dos loops
        IMPRIMA "Aviso: posição S ajustada"
    
    // Pergunta sobre modo passo-a-passo
    LEIA modo_passo_a_passo
    
    // Inicialização da simulação
    sujeira_inicial = mapa.sujeira_total
    posicao = mapa.S
    log = novo Log(64)
    limpezas = 0
    bloqueios = 0
    passos = 0
    
    SE modo_passo_a_passo:
        imprimir_mapa(mapa, posicao)
        IMPRIMA "Sujeira inicial:", sujeira_inicial
        espera_enter()
    
    // Loop principal da simulação
    tempo_inicio = tempo_atual()
    
    PARA t = 0 ATÉ T-1:
        SE mapa.sujeira_total == 0:
            PARE  // Todas as sujeiras foram limpas
        
        // Decisão do agente
        motivo = ""
        acao = decide_reflex(mapa, posicao, REF motivo)
        
        // Execução da ação
        sucesso = aplicar_acao(REF mapa, REF posicao, acao, REF limpezas, REF bloqueios)
        
        // Logging
        log_push(REF log, acao)
        passos++
        
        // Modo passo-a-passo
        SE modo_passo_a_passo:
            IMPRIMA "Passo", passos, "| Ação:", nome_acao(acao)
            IMPRIMA "| Motivo:", motivo
            IMPRIMA "| Resultado:", (sucesso ? "ok" : "bloqueado")
            imprimir_mapa(mapa, posicao)
            IMPRIMA "Sujeira restante:", mapa.sujeira_total
            IMPRIMA "Limpezas:", limpezas, "| Bloqueios:", bloqueios
            espera_enter()
    
    tempo_fim = tempo_atual()
    tempo_cpu = tempo_fim - tempo_inicio
    
    // Relatório final
    IMPRIMA "=== Resultado da simulação ==="
    IMPRIMA "Passos executados:", passos, "(limite T=" + T + ")"
    IMPRIMA "Limpezas realizadas:", limpezas
    IMPRIMA "Tentativas bloqueadas:", bloqueios
    IMPRIMA "Sujeira inicial:", sujeira_inicial
    IMPRIMA "Sujeira restante:", mapa.sujeira_total
    
    SE sujeira_inicial > 0:
        percentual = 100.0 * (sujeira_inicial - mapa.sujeira_total) / sujeira_inicial
    SENÃO:
        percentual = 100.0
    
    IMPRIMA "Sujeira removida:", percentual, "%"
    IMPRIMA "Tempo de CPU:", tempo_cpu, "s"
    
    // Opção de ver mapa final
    LEIA ver_final
    SE ver_final:
        imprimir_mapa(mapa, posicao)
        IMPRIMA "Posição final do robô:", posicao.r, posicao.c
    
    // Limpeza de memória
    libera_memoria(mapa)
    libera_memoria(log)
    
FIM PROGRAMA
```

## Fluxo de Execução Resumido

1. **Inicialização**: Lê parâmetros e mapa do ambiente
2. **Configuração**: Define posição inicial e modo de execução
3. **Loop Principal**: Para cada passo até T ou até limpar tudo:
   - Aplicar regras de decisão (reflex)
   - Executar ação escolhida
   - Atualizar estado do ambiente
   - Registrar métricas
4. **Relatório**: Exibir estatísticas de desempenho
5. **Limpeza**: Liberar recursos de memória

## Características do Agente Reflex

- **Reativo**: Decide baseado apenas na percepção atual
- **Sem memória**: Não lembra de estados anteriores
- **Regras fixas**: Prioridade rígida das decisões
- **Varredura sistemática**: Padrão zig-zag quando não há sujeira próxima
- **Limitações**: Pode ficar preso, não otimiza caminho, não planeja