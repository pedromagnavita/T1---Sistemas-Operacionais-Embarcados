# Sistemas Operacionais Embarcados - Trabalho 1 (T1)

Projeto de Sistema Operacional de Tempo Real para microcontrolador PIC24FJ128GA010 simulado no Proteus.

## Estrutura do Projeto

- `T1.pdsprj`: Projeto de simulação no Proteus.
- `code/`: Código-fonte do sistema operacional e aplicações.
  - `inc/`: Arquivos de cabeçalho (`.h`).
  - `src/`: Implementação dos drivers, escalonador, kernel e aplicação (`.c`).
  - `firmware.hex` / `firmware.elf`: Binários compilados para a simulação.
  - `.vscode/`: Configuração de tarefas e compilação no VS Code (XC16).
