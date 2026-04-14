import socket as so
import customtkinter as ctk


class NetworkManager:

    def __init__(self, ip: str, port: int):
        self.addr = (ip, port)

        self.sock = so.socket(
            so.AF_INET,
            so.SOCK_DGRAM
        )
        # Definimos como não bloqueante aqui para não precisarmos de
        # threads na interface
        self.sock.setblocking(False)

    def send(self, message: str) -> None:
        self.sock.sendto(message.encode(), self.addr)

    def receive(self) -> str | None:
        try:
            data, server = self.sock.recvfrom(4096)

            # Atualização de porta
            if server[1] != self.addr[1]:
                self.addr = (self.addr[0], server[1])

            return data.decode()

        except (BlockingIOError, KeyboardInterrupt):
            return None

class App(ctk.CTk):
    def __init__(self, network_manager):
        self.network = network_manager

        super().__init__()

        self.title("AuxiliaryMessageTool")
        self.geometry("700x500")
        ctk.set_appearance_mode("dark")

        # Configuração do Grid
        self.grid_rowconfigure(0, weight=1)  # Frame de Log (expansível)
        self.grid_rowconfigure(1, weight=0)  # Frame de Comando (fixo)
        self.grid_columnconfigure(0, weight=1)

        # Definição do Frame de Mensagens
        self.log_frame = ctk.CTkFrame(self)
        self.log_frame.grid(row=0, column=0, padx=20, pady=(20, 10), sticky="nsew")
        self.log_frame.grid_rowconfigure(0, weight=0)  # Título Sense
        self.log_frame.grid_rowconfigure(1, weight=0)  # Conteúdo Sense
        self.log_frame.grid_rowconfigure(2, weight=0)  # Título See
        self.log_frame.grid_rowconfigure(3, weight=1)  # Conteúdo See (expande para ocupar o resto)
        self.log_frame.grid_columnconfigure(0, weight=1)

        # --- Sense Body ---
        self.sense_label_header = ctk.CTkLabel(self.log_frame, text="Último Sense Body:", font=("Arial", 12, "bold"),
                                               text_color="cyan")
        self.sense_label_header.grid(row=0, column=0, padx=10, pady=(10, 0),
                                     sticky="nw")  # sticky="nw" coloca no topo-oeste (norte-oeste)

        self.sense_display = ctk.CTkLabel(self.log_frame, text="(Aguardando...)", font=("Consolas", 15), justify="left")
        self.sense_display.grid(row=1, column=0, padx=10, pady=(0, 20), sticky="nw")

        # --- Visão (See) ---
        self.see_label_header = ctk.CTkLabel(self.log_frame, text="Última Visão (See):", font=("Arial", 12, "bold"),
                                             text_color="yellow")
        self.see_label_header.grid(row=2, column=0, padx=10, pady=(5, 0), sticky="nw")

        self.see_display = ctk.CTkLabel(self.log_frame, text="(Aguardando...)", font=("Consolas", 15), justify="left")
        self.see_display.grid(row=3, column=0, padx=10, pady=(0, 10), sticky="nw")

        # Ativamos o evento de redimensionamento para o frame
        self.log_frame.bind("<Configure>", self._on_resize)

        # Definição do Frame de Mensagens
        self.cmd_frame = ctk.CTkFrame(self)
        self.cmd_frame.grid(row=1, column=0, padx=20, pady=(10, 20), sticky="ew")
        self.cmd_frame.grid_columnconfigure(0, weight=1)
        self.cmd_entry = ctk.CTkEntry(
            self.cmd_frame,
            placeholder_text="Digite o comando aqui... (ex: (init...))"
        )
        self.cmd_entry.grid(row=0, column=0, padx=10, pady=20, sticky="ew")
        # Atalho: pressionar Enter para enviar
        self.cmd_entry.bind("<Return>", lambda e: self.send_command())
        self.send_button = ctk.CTkButton(self.cmd_frame, text="Enviar", command=self.send_command)
        self.send_button.grid(row=0, column=1, padx=10, pady=20)

        # Iniciar o loop de escuta
        self.update_hears()

    def _on_resize(self, event):
        """Atualiza a largura de quebra de linha com base na largura atual do frame."""
        # Subtraímos um pouco (ex: 40px) para dar margem nas laterais
        new_width = event.width - 40
        if new_width > 0:
            self.sense_display.configure(wraplength=new_width)
            self.see_display.configure(wraplength=new_width)

    def send_command(self):
        msg = self.cmd_entry.get()
        if msg:
            self.network.send(msg + '\0')
            self.cmd_entry.delete(0, 'end')

    def update_hears(self):
        """Checa o socket periodicamente sem travar a interface."""

        msg = self.network.receive()

        if msg:
            # Filtro para Sense Body
            if msg.startswith("(sense_body"):
                # Atualiza apenas a label específica
                self.sense_display.configure(text=msg)

            # Filtro para Visão
            elif msg.startswith("(see"):
                # Atualiza apenas a label específica
                self.see_display.configure(text=msg)

            # Mensagens que não são como essas
            else:
                print(msg)

        # Executa esta função novamente após 50ms (Polling)
        self.after(50, self.update_hears)


if __name__ == "__main__":
    network = NetworkManager("127.0.0.1", 6000)
    app = App(network)
    app.mainloop()
