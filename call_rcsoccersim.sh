#!/bin/bash

# Diretório onde os logs serão armazenados
LOG_DIR="./logs"

# Garante que o diretório exista
mkdir -p "$LOG_DIR"

# Função de limpeza pré-execução
cleanup() {
    if [ -d "$LOG_DIR" ]; then
        # Remove apenas logs conhecidos do simulador
        find "$LOG_DIR" -type f \( -name "*.rcg" -o -name "*.rcl" \) -delete
    fi
}

# Limpeza antes da execução
cleanup

# Execução de servidor + monitor
rcsoccersim
