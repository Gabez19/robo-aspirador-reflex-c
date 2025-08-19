# Relatório de Resultados do Robô Aspirador

## 1. Quadro de Resultados

| Mapa    | Passos | Limpezas | Bloqueios | % Removida | Tempo CPU |
|---------|--------|----------|-----------|------------|-----------|
| Fácil   | 80     | 2        | 0         | 66.7%      | 0.000000s |
| Médio   | 160    | 6        | 0         | 75.0%      | 0.000000s |
| Difícil | 254    | 5        | 0         | 45.5%      | 0.000000s |

---

## 2. Decisões Passo-a-Passo

### Decisão 1

- **Passo:** 5  
- **Ação:** Limpar célula atual  
- **Regra:** Regra 1 – Limpar (célula atual suja)  
- **Justificativa:** O robô detectou sujeira na célula atual e aplicou a regra de limpeza antes de se mover.

### Decisão 2

- **Passo:** 12  
- **Ação:** Mover para a direita  
- **Regra:** Regra 3 – Zig-Zag (varredura sistemática)  
- **Justificativa:** Não havia sujeira na célula atual nem nos vizinhos; o robô seguiu a varredura em zig-zag para continuar cobrindo a área.

---

## 3. Comentário sobre Limitações

O robô apresenta algumas limitações:  
1. Dificuldade em lidar com obstáculos complexos ou inacessíveis, podendo travar ou entrar em loops.  
2. Algoritmo simples de decisão, sem planejamento de rotas ou otimização de passos.  
3. Depende apenas do estado atual, sem memória de células visitadas, podendo repetir movimentos.  
4. A varredura em zig-zag pode deixar áreas inacessíveis em mapas maiores.  
5. Não há estratégia para minimizar custo ou distância percorrida; o objetivo é apenas limpar o que está acessível.


## 4. Evidência de Execução

-   Print da tela final do mapa Fácil:

![Mapa Fácil](print_mapa_facil.png)

-   Print da tela final do mapa Médio:

![Mapa Médio](print_mapa_medio.png)

-   Print da tela final do mapa Difícil:

![Mapa Difícil](print_mapa_dificil.png)