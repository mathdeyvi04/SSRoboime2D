#!/bin/bash

rm -rf logs

LOG_DIR="./logs"
mkdir -p "$LOG_DIR"

SERVER_ARGS=(
    "server::game_logging=true"
    "server::text_log_dir=$LOG_DIR"
    "server::game_log_dir=$LOG_DIR"
)

if [[ "$1" == "--trainer_w_referee" ]]; then
    echo "Iniciando servidor com suporte a Trainer e Árbitro..."
    SERVER_ARGS+=("-coach_w_referee")
fi

if [[ "$1" == "--trainer" ]]; then
    echo "Iniciando servidor com suporte a Trainer..."
    SERVER_ARGS+=("-coach")
fi

echo "Iniciando rcssserver..."
rcssserver "${SERVER_ARGS[@]}" > "$LOG_DIR/server.log" 2>&1 &
SERVER_PID=$!

sleep 1

echo "Iniciando monitor..."
rcssmonitor > "$LOG_DIR/monitor.log" 2>&1

kill "$SERVER_PID"