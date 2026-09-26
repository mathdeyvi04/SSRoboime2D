#!/bin/bash

rm -rf logs

LOG_DIR="./logs"
mkdir -p "$LOG_DIR"

SERVER_ARGS=(
	"server::game_logging=true"
	"server::text_log_dir=$LOG_DIR"
	"server::game_log_dir=$LOG_DIR"
)

if [[ "$1" == "--synch" ]]; then
	echo "Iniciando servidor no modo síncrono..."
	SERVER_ARGS+=("server::synch_mode=true")
fi

if [[ "$1" == "--trainer_w_referee" ]]; then
	echo "Iniciando servidor com suporte a Trainer e Árbitro..."
	SERVER_ARGS+=("server::coach_w_referee=true")
	SERVER_ARGS+=("server::synch_mode=true")
fi

if [[ "$1" == "--trainer" ]]; then
	echo "Iniciando servidor com suporte a Trainer..."
	SERVER_ARGS+=("server::coach=true")
	SERVER_ARGS+=("server::synch_mode=true")
fi

echo "Iniciando rcssserver..."
rcssserver "${SERVER_ARGS[@]}" > "$LOG_DIR/server.log" 2>&1 &
SERVER_PID=$!

sleep 1

if ! kill -0 "$SERVER_PID" 2>/dev/null; then
	echo "Falha ao iniciar o rcssserver. Verifique se a porta 6000 já está em uso."
	sed -n '1,120p' "$LOG_DIR/server.log"
	exit 1
fi

echo "Iniciando monitor..."
rcssmonitor > "$LOG_DIR/monitor.log" 2>&1

kill "$SERVER_PID"
