from pathlib import Path
from time import sleep
import os
import socket

class NetworkManager:
    def __init__(self, ip: str, porta: int, if_is_player: bool ):
        self._addr = (ip, porta)
        self._sock = socket.socket(
            socket.AF_INET,
            socket.SOCK_DGRAM
        )
        self._init(if_is_player)

    def send(self, message: str) -> None:
        self._sock.sendto(
            (message + '\0').encode(),
            self._addr
        )

    def receive(self) -> str | None:
        # Dado que ele só precisa se mover seguindo a demonstração
        # Não precisamos nos importar com as mensagens que chegam para eles
        try:
            data, server = self._sock.recvfrom(4096)

            # Atualização de porta
            if server[1] != self._addr[1]:
                self._addr = (self._addr[0], server[1])

            return data.decode()

        except (BlockingIOError, KeyboardInterrupt):
            return None

    def _init(self, if_is_player: bool) -> None:
        self._sock.setblocking(False)
        if if_is_player:
            self.send("(init RoboIME (version 18))")
        else:
            self.send("(init (version 18))")

class Player:
    # Se conecta e envia comandos de movimento, apenas.
    SKILL_MAX_COUNTERS = [10, 10, 10]
    def __init__(self):
        self.counter = [0, 0, 0]
        self.sender = NetworkManager("127.0.0.1", 6000, True)

    def _dash(self, power: int = 100):
        if self.counter[0] != Player.SKILL_MAX_COUNTERS[0]:
            return

        # Sem gestão de energia e stamina
        self.sender.send(
            f"(dash {power})"
        )

        self.counter[0] += 1

    def _turn(self, angle_body: int, angle_neck: int):
        # Ambos ângulos em graus
        if self.counter[1] != Player.SKILL_MAX_COUNTERS[1]:
            return

        if angle_body != 0:
            self.sender.send(
                f"(turn {angle_body})"
            )

        if angle_neck != 0:
            self.sender.send(
                f"(turn_neck {angle_neck})"
            )

        self.counter[1] += 1

    def _kick(self, power: int, direction: int):
        if self.counter[2] != Player.SKILL_MAX_COUNTERS[2]:
            return

        self.sender.send(
            f"(kick {power} {direction})"
        )

        self.counter[2] += 1

    def _wait(self):
        # Esse não precisa de counter
        sleep(0.5)

class Trainer:
    # Se conecta e envia comandos de controle do servidor.
    # Basicamente, será usado para mover os jogadores para posição inicial, reiniciando a demonstração

    def __init__(self):
        self.sender = NetworkManager("127.0.0.1", 6001, False)

        self.players = [
            Player() for _ in range(12)
        ]

        # Para Controle de Cena
        self.initial_position = Trainer.load_formation(
            Path(__file__).parent.parent / "booting" / "TacticalFormations.hpp"
        )
        self.path_to_possibles_demonstrations = [
            os.path.join(
                Path(__file__).parent / "demonstrations",
                possible_demonstration
            ) for possible_demonstration in os.listdir(
                Path(__file__).parent / "demonstrations"
            ) if possible_demonstration.endswith(".txt")
        ]

    @staticmethod
    def load_formation(path: Path):
        positions = []

        with open(path) as f:
            for line in f:
                if line.endswith("= {\n"):
                    break

            for line in f:
                if line.endswith("};\n"):
                    break
                line = line.strip()
                line = line.split(',')
                positions.append((line[0], line[1]))

        return positions

    def _reset_position(self):
        # Resetará a posição dos jogadores e da bola
        pass

    def _execute_demonstration(self):
        # Executará cada demonstração
        # Cada arquivo de demonstração terá 11 colunas
        # Cada coluna referenciará um jogador
        pass

    def user_interface(self):
        # Controle de Entrada do User
        def print_menu() -> list[str]:
            print("Demonstrações Disponíveis")
            print("=" * 40)

            entries = [f"{i} - {os.path.basename(name).replace('.txt', '')}" for i, name in
                       enumerate(self.path_to_possibles_demonstrations)]
            width = max(len(entry) for entry in entries) + 4

            for i in range(0, len(entries), 2):
                left = entries[i]

                if i + 1 < len(entries):
                    right = entries[i + 1]
                    print(f"{left:<{width}}{right}")
                else:
                    print(left)
            return entries

        while True:

            possible_demonstrations = print_menu()
            try:
                choice = int(
                    input(f"\nEscolha uma opção: ")
                )

                if 0 <= choice < len(self.path_to_possibles_demonstrations):
                    print(f"Executando [ {possible_demonstrations[choice].split(' ')[-1]} ]")

                    # Colocamos aqui o sanha
                    sleep(5)

                    os.system("cls" if os.name == "nt" else "clear")
                else:
                    print("Opção Inválida.")

            except ValueError:
                print("Digite um número válido.")


if __name__ == '__main__':
    t = Trainer()
    t.user_interface()
