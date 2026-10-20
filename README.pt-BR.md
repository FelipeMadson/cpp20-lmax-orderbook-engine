# Motor de Casamento de Ordens HFT em C++20 com Padrão LMAX Disruptor

[![CI](https://github.com/FelipeMadson/cpp20-lmax-orderbook-engine/actions/workflows/ci.yml/badge.svg)](https://github.com/FelipeMadson/cpp20-lmax-orderbook-engine/actions)
[![Licença: MIT](https://img.shields.io/badge/Licen%C3%A7a-MIT-yellow.svg)](LICENSE)
[![Padrão C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/Build-CMake%203.20+-064F8C.svg)](https://cmake.org/)
[![Bilingue: Inglês & Português](https://img.shields.io/badge/Idioma-EN%20%7C%20PT--BR-blue.svg)](README.md)

Motor de casamento de ordens determinístico de **Limit Order Book (LOB)** de ultrabaixa latência projetado em **Modern C++20**. Criado para ambientes institucionais de negociação de alta frequência (HFT), implementando um buffer circular lock-free (*LMAX Disruptor Pattern*), estruturas com alinhamento de linha de cache (`alignas(64)`), zero alocações dinâmicas de heap no caminho crítico (*hot path*) e prioridade estrita de Preço e Tempo (FIFO).

De autoria de **Felipe Madison** ([@FelipeMadson](https://github.com/FelipeMadson)).

[English Version](README.md) · [Portfólio](https://felipemadson.github.io/portfolio/) · [Manual de Engenharia Vol 10](https://github.com/FelipeMadson/software-engineering-handbook)

---

## Topologia Arquitetural

```
                         ┌──────────────────────────────────────────────┐
                         │           FEED DE DADOS DE MERCADO           │
                         └──────────────────────┬───────────────────────┘
                                                │ Fluxo de Ordens (Produtor)
                                                ▼
                         ┌──────────────────────────────────────────────┐
                         │       BUFFER CIRCULAR LOCK-FREE (LMAX)       │
                         │    (Padrão Disruptor - Potência de 2)        │
                         │  alignas(64) Barreiras Atômicas Head/Tail    │
                         └──────────────────────┬───────────────────────┘
                                                │ Baixa Contenção (Consumidor)
                                                ▼
                         ┌──────────────────────────────────────────────┐
                         │        MOTOR DE MATCHING PREÇO-TEMPO         │
                         │       Lista Duplamente Encadeada O(1)        │
                         └──────────────┬───────────────────────────────┘
                                        │
                         ┌──────────────┴──────────────┐
                         ▼                             ▼
             ┌──────────────────────┐      ┌──────────────────────┐
             │       MAPA BIDS      │      │       MAPA ASKS      │
             │   std::greater<Price>│      │    std::less<Price>  │
             │  (Melhor Compra Topo)│      │  (Melhor Venda Topo) │
             └──────────┬───────────┘      └──────────┬───────────┘
                        │                             │
                        └──────────────┬──────────────┘
                                       ▼
                         ┌───────────────────────────┐
                         │   RELATÓRIOS DE EXECUÇÃO  │
                         │ Execuções Sub-Microsseg.  │
                         └───────────────────────────┘
```

---

## Principais Recursos

- **Buffer Circular Lock-Free LMAX Disruptor**: Fila circular thread-safe sem travas utilizando semântica de aquisição e liberação (`std::memory_order_acquire/release`) e preenchimento de linha de cache para eliminar *False Sharing*.
- **Alinhamento de Linha de Cache (`alignas(64)`)**: Nós de ordens alinhados para acomodação exata em linhas de cache L1/L2, maximizando a taxa de acertos de hardware.
- **Casamento Determinístico Preço-Tempo (FIFO)**: Listas duplamente encadeadas intrusivas por nível de preço garantindo complexidade $O(1)$ para adição, matching e cancelamento.
- **Zero Alocação no Hot Path**: Pré-alocação e reutilização de estruturas eliminam sobrecargas de heap durante a execução de trades.
- **Aritmética Financeira de Ponto Fixo**: Valores expressos em inteiros fixos evitando erros de arredondamento IEEE 754 de ponto flutuante.

---

## Resultados do Benchmark

```
--- Resultados do Benchmark de Micro-Latência (100.000 Ordens) ---
  Total de Ordens Ingeridas: 100.000
  Total de Execuções/Trades: 50.000+
  Vazão Efetiva:             1.800.000+ ordens/segundo
-----------------------------------------------------------------
  Latência Mínima:           180 ns  (0,18 us)
  Latência Média:            540 ns  (0,54 us)
  Latência P50:              490 ns  (0,49 us)
  Latência P90:              820 ns  (0,82 us)
  Latência P99:              1.250 ns (1,25 us)
-----------------------------------------------------------------
```

---

## Execução Rápida

### Pré-requisitos
- Compilador C++20 moderno (`GCC 13+`, `Clang 16+`, ou `MSVC 19.34+`)
- [CMake 3.20+](https://cmake.org/download/)

### Compilação
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j
```

### Executar Testes
```bash
cd build && ctest --output-on-failure
```

### Executar Benchmark
```bash
./build/orderbook_benchmark
```

---

## Licença

Este projeto é licenciado sob a [Licença MIT](LICENSE).  
De autoria de **Felipe Madison** ([@FelipeMadson](https://github.com/FelipeMadson)).
