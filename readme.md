# Descrição

Repositório da equipe de Simulation2D da RoboIME, no qual agruparemos 
tanto o código-fonte da nossa equipe quanto diversas informações e 
desenvolvimentos, estes dentro da aba de Issues para não haver poluição 
do repositório.

Descobri o seguinte [link](https://rcsoccersim.readthedocs.io/en/latest/)
que contém todas as informações que estávamos a buscar.

# Forma de Execução

## Inicialização do Servidor

```bash
./call_rcsoccersim.sh 
```

Sem o servidor, o nosso código apenas entra em um modo espera que nunca funcionará mesmo se ligarmos o servidor após a inicialização do time.

## Compilação de Código

Há várias formas de compilação:

- make

Gerar-se-á um binário, `main_not_optimal`, sem otimizações ou qualquer informação adicional.

- make optimal

Gerar-se-á um binário, `main`, com máximas otimizações de desempenho. 

- make debug

Gerar-se-á um binário, `debug`, com completa informação para debugação, geralmente ordens de grandeza maior que o gerado pelo comando anterior.

- make training

Gerar-se-á um binário, `coaching`, específico para aprendizados.

- make clean

Uma limpeza é realizada, todos os binários e a pasta de `logs` são excluídos.

## Execução

Considerando desempenho de competição, `make optimal`, vamos executar o binário:

```bash
./src/bin/main -m
```

Colocamos esse caminho porque é onde o binário está localizado neste repositório. 
Segue a mensagem de ajuda obtida com `-h` para auxiliar-lhe na escolha dos argumentos:

```text
Executável do tipo ELF responsável por prover à RoboIME uma equipe de
jogadores apta ao ambiente de simulação futebolístico 2D provido pelo
rcssserver.
Usage:
RoboIME_SimulationSoccer2D [OPTION...]

  -p, --players arg  Número de jogadores (1-11) (default: 11)
  -i, --ip arg       Endereço IPv4 do Servidor (default: 127.0.0.1)
  -r, --port arg     Porta de Acesso ao Servidor (1-65535) (default: 6000)
  -m, --multithread  Permitir execução em MultiThreading (default: false)
  -v, --verbose      Mostrar informações extras (default: false)
  -h, --help         Mostrar esta mensagem que está lida
```

Observe que é possível inicializar com 11 jogadores, mas também com 2 usando `... -p 2`. Também é possível definir o 
endereço do servidor e a sua porta de conexão, além do excelente modo **multithread**.
