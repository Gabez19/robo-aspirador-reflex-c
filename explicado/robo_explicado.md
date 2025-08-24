# Robô de Limpeza

Este projeto implementa um robô simples que percorre uma grade, limpa sujeiras e evita ficar preso em loops.

---

## 1. Estrutura e Tipos de Dados
- **Ponto**: representa coordenadas na grade (`linha`, `coluna`).
- **Acao (enum)**: ações possíveis → `LIMPAR`, `MOVER_N`, `MOVER_S`, `MOVER_L`, `MOVER_O`, `FICAR`.
- **Log**: histórico das ações (fila circular).
- **Mapa**:
  - `N`, `M`: dimensões da grade.
  - `T`: passos máximos.
  - `g`: grade (`.`, `*`, `#`, `S`).
  - `S`: posição inicial.
  - `sujeira_total`: quantidade inicial de sujeira.

---

## 2. Variáveis Globais
- **`passos_sem_limpar`**: conta passos desde a última limpeza. Se passar de **100**, o robô para.
- **`historico_posicoes`**: últimas 50 posições para evitar ciclos curtos (**anti-loop**).

---

## 3. Função `decide_reflex` (Inteligência do Robô)
O robô segue regras em ordem de prioridade:

1. **Limpar a célula atual**  
   Se estiver sobre sujeira, executa `LIMPAR`.

2. **Mover para um vizinho sujo**  
   Se houver sujeira ao lado, move-se diretamente até ela.

3. **Estratégia Anti-Loop**  
   Se passaram **30 passos sem limpar**, tenta explorar posições novas.

4. **Varredura Zig-Zag**  
   - Coluna par → move para Leste.  
   - Coluna ímpar → move para Oeste.  
   - Se bloqueado, desce uma linha.

5. **Fallback**  
   Se nada funcionar → tenta **Norte**, depois **Sul**.  
   Se não houver saída → `FICAR`.

---

## 4. Execução Principal (`main`)
1. **Configuração**: lê dimensões (`N`, `M`), tempo máximo (`T`) e a grade.  
2. **Loop de Simulação**:  
   - Escolhe ação com `decide_reflex`.  
   - Executa com `aplicar_acao`.  
   - Atualiza contadores e histórico.  
3. **Condições de Parada**:  
   - Toda sujeira foi limpa.  
   - Mais de **100 passos sem limpar**.  
   - Limite máximo de passos atingido.  
4. **Resultados**: imprime tempo de CPU, sujeira removida e diagnóstico de loop.

---
