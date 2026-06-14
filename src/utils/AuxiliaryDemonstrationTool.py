from pathlib import Path
from time import sleep
import os
import socket

class NetworkManager:
    def __init__(self, ip: str, porta: int):
        self._addr = (ip, porta)
        self._sock = socket.socket(
            socket.AF_INET,
            socket.SOCK_DGRAM
        )
        self._init()

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

    def _init(self) -> None:
        self._sock.setblocking(False)
        self.send("(init RoboIME (version 18))")
        self.receive()

class Player:
    # Se conecta e envia comandos de movimento, apenas.
    SKILL_MAX_COUNTERS = [10, 10, 10]
    def __init__(self):
        self.counter = [0, 0, 0]
        self.sender = NetworkManager("127.0.0.1", 6000)

    def dash(self, power: int = 100):
        if self.counter[0] != Player.SKILL_MAX_COUNTERS[0]:
            return

        # Sem gestão de energia e stamina
        self.sender.send(
            f"(dash {power})"
        )

        self.counter[0] += 1

    def turn(self, angle_body: int, angle_neck: int):
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

    def kick(self, power: int, direction: int):
        if self.counter[2] != Player.SKILL_MAX_COUNTERS[2]:
            return

        self.sender.send(
            f"(kick {power} {direction})"
        )

        self.counter[2] += 1

    def wait(self):
        # Esse não precisa de counter
        sleep(0.5)

class Trainer:
    # Se conecta e envia comandos de controle do servidor.

    def __init__(self):
        self.sender = NetworkManager("127.0.0.1", 6001)



if __name__ == '__main__':
    dir_atual = Path(__file__).parent
    possible_demonstrations = os.listdir(dir_atual / "demonstrations")