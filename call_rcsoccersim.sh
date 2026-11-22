#!/bin/bash

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
LOCAL_SERVER="$SCRIPT_DIR/.local/rcssserver-runtime/bin/rcssserver"
if [[ -n "${RCSSSERVER_BIN:-}" ]]; then
	SERVER_BIN="$RCSSSERVER_BIN"
elif [[ -x "$LOCAL_SERVER" ]]; then
	SERVER_BIN="$LOCAL_SERVER"
else
	SERVER_BIN="rcssserver"
fi

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
if [[ "$SERVER_BIN" == "$LOCAL_SERVER" ]]; then
	LD_LIBRARY_PATH="$SCRIPT_DIR/.local/rcssserver-runtime/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" \
		"$SERVER_BIN" "${SERVER_ARGS[@]}" > "$LOG_DIR/server.log" 2>&1 &
else
	"$SERVER_BIN" "${SERVER_ARGS[@]}" > "$LOG_DIR/server.log" 2>&1 &
fi
SERVER_PID=$!

sleep 1

if ! kill -0 "$SERVER_PID" 2>/dev/null; then
	echo "Falha ao iniciar o rcssserver ($SERVER_BIN). Detalhes:"
	sed -n '1,120p' "$LOG_DIR/server.log"
	exit 1
fi

echo "Iniciando monitor..."
rcssmonitor > "$LOG_DIR/monitor.log" 2>&1

kill "$SERVER_PID"
