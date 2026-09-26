# Teste integrado das ações básicas

O modo `--basic-actions-test` distribui papéis entre os jogadores sem alterar
o comportamento normal do agente.

## Papéis

| Jogador | Ensaio |
|---|---|
| 1 | Busca/intercepta a bola e passa para o jogador 2 por índice |
| 2 | Intercepta o passe e chuta para `(52.5, 0)` |
| 3 | Patrulha entre duas coordenadas usando `Walk` |
| 4 | Segue o jogador 3 usando percepção e `Walk(index)` |
| 5 | Alterna foco entre bola e gol e centraliza o pescoço |
| 6 | Executa diretamente a sequência turn, dash e turn_neck |
| 7–11 | Caminham para faixas distintas e mantêm foco na bola |

Todos os papéis produzem telemetria `BASIC_TEST` a cada 20 ciclos quando o
modo `--verbose` está ativo. A telemetria contém visibilidade, idade da
informação, distância, direção relativa e disponibilidade de comando corporal,
obtidas pela consulta agregada `target_info(index)`.

## Execução

Em três terminais:

Antes de começar, feche qualquer monitor/servidor de uma execução anterior.
O cenário depende de uma sessão nova para que os seis clientes recebam as
camisas 1 a 6. O script agora detecta e informa quando a porta 6000 já está
ocupada, em vez de conectar o monitor ao servidor antigo silenciosamente.

```bash
# Terminal 1
bash call_rcsoccersim.sh --trainer_w_referee

# Terminal 2
make debug
./src/bin/debug --players 6 --multithread --verbose --basic-actions-test

# Terminal 3
make training
./src/bin/coaching --scenario basic-actions
```

O treinador carrega automaticamente `scenarios/basic-actions.txt` e envia um
comando por ciclo. Depois do último comando, o terminal volta ao modo interativo.
O equivalente manual seria enviar:

```text
(change_mode before_kick_off)
(move (player RoboIME 1) -20 0 0 0 0)
(move (player RoboIME 2) -5 0 180 0 0)
(move (player RoboIME 3) -15 12 0 0 0)
(move (player RoboIME 4) -25 12 0 0 0)
(move (player RoboIME 5) -10 -12 0 0 0)
(move (player RoboIME 6) -20 -20 0 0 0)
(move (ball) -9.2 0 0 0 0)
(change_mode play_on)
```

Para testar predição com a bola em movimento, reinicie o estado e use:

```bash
./src/bin/coaching --scenario moving-ball
```

Esse nome carrega `scenarios/moving-ball.txt`, cujo conteúdo é:

```text
(change_mode before_kick_off)
(move (player RoboIME 1) -20 0 0 0 0)
(move (player RoboIME 2) -5 0 180 0 0)
(move (player RoboIME 3) -15 12 0 0 0)
(move (player RoboIME 4) -25 12 0 0 0)
(move (player RoboIME 5) -10 -12 0 0 0)
(move (player RoboIME 6) -20 -20 0 0 0)
(move (ball) -10 0 0 1 0)
(change_mode play_on)
```

## Verificação

```bash
rg 'BASIC_TEST' logs
rg '\((dash|turn|turn_neck|kick) ' logs
```

O monitor deve mostrar simultaneamente o passe entre 1 e 2, a patrulha do 3,
o acompanhamento feito pelo 4, a varredura do 5 e a sequência motora do 6.

Também é possível carregar um arquivo fora da pasta `scenarios`:

```bash
./src/bin/coaching --scenario /caminho/para/meu-cenario.txt
```

Linhas vazias e linhas iniciadas por `#` são ignoradas.
