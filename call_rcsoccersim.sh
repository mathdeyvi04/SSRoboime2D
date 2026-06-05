#!/bin/bash

rm -rf logs

# Diretório onde os logs serão armazenados
LOG_DIR="./logs"

# Garante que o diretório exista
mkdir -p "$LOG_DIR"

# Execução de servidor + monitor
rcsoccersim
