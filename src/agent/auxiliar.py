import tkinter as tk

import customtkinter as ctk
from src.mapa import FieldMap
from src.search import SearchPath

ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("blue")


class App(ctk.CTk):
    def __init__(self):
        super().__init__()

        self.title("Mapa do Campo de Futebol")
        self.geometry("1000x700")

        self.search = SearchPath(self, on_positions_loaded=self.receber_posicoes)
        self.search.pack(fill="both", expand=True, padx=20, pady=20)

        self.mapa = FieldMap(
            self,
            imagem_fundo="src/images/campo.png",  # troque pelo nome da sua imagem
            largura=108,
            altura=64,
        )
        self.mapa.pack(pady=20)

        self.frame_botoes = ctk.CTkFrame(self)
        self.frame_botoes.pack(pady=10)

        self.botao_exemplo = ctk.CTkButton(
            self.frame_botoes,
            text="Adicionar jogadores exemplo",
            command=self.carregar_exemplo,
        )
        self.botao_exemplo.pack(side="left", padx=10)

        self.botao_limpar = ctk.CTkButton(
            self.frame_botoes,
            text="Limpar mapa",
            command=self.mapa.limpar_jogadores,
        )
        self.botao_limpar.pack(side="left", padx=10)
        self.btn_save_position = ctk.CTkButton(
            self.frame_botoes,
            text="Salvar posição",
            command=self.save_position,
        )
        self.btn_save_position.pack(side="left", padx=10)

    def receber_posicoes(self, positions):

        self.mapa.limpar_jogadores()
        for index, pos in enumerate(positions):
            if index <= 10:
                self.mapa.add_player(pos[0], pos[1], str(index), "blue")
            else:
                self.mapa.add_player(pos[0], pos[1], str(index), "red")
        print("Posições carregadas:", positions)

    def carregar_exemplo(self):
        self.mapa.limpar_jogadores()

        # --- Configuração do Time Azul (AZ) ---
        # Defesa e Goleiro
        self.mapa.add_player(-40.0, 0.0, "AZ1", "blue")  # Goleiro
        self.mapa.add_player(-25.0, 22.0, "AZ2", "blue")  # Lateral Esq
        self.mapa.add_player(-25.0, 7.0, "AZ3", "blue")  # Zagueiro
        self.mapa.add_player(-25.0, -7.0, "AZ4", "blue")  # Zagueiro
        self.mapa.add_player(-25.0, -22.0, "AZ5", "blue")  # Lateral Dir

        # Meio-Campo
        self.mapa.add_player(5.0, 15.0, "AZ6", "blue")
        self.mapa.add_player(5.0, 5.0, "AZ7", "blue")
        self.mapa.add_player(5.0, -5.0, "AZ8", "blue")
        self.mapa.add_player(5.0, -15.0, "AZ9", "blue")

        # Ataque
        self.mapa.add_player(35.0, 8.0, "AZ10", "blue")
        self.mapa.add_player(35.0, -8.0, "AZ11", "blue")

        # --- Configuração do Time Vermelho (VM) ---
        # Defesa e Goleiro (Lado oposto, X positivo)
        self.mapa.add_player(40.0, 0.0, "VM1", "red")  # Goleiro
        self.mapa.add_player(25.0, 22.0, "VM2", "red")  # Lateral Esq
        self.mapa.add_player(25.0, 7.0, "VM3", "red")  # Zagueiro
        self.mapa.add_player(25.0, -7.0, "VM4", "red")  # Zagueiro
        self.mapa.add_player(25.0, -22.0, "VM5", "red")  # Lateral Dir

        # Meio-Campo / Volantes
        self.mapa.add_player(10.0, 18.0, "VM6", "red")
        self.mapa.add_player(10.0, 6.0, "VM7", "red")
        self.mapa.add_player(10.0, -6.0, "VM8", "red")
        self.mapa.add_player(10.0, -18.0, "VM9", "red")

        # Ataque (Avançados no campo adversário, X negativo)
        self.mapa.add_player(-20.0, 10.0, "VM10", "red")
        self.mapa.add_player(-20.0, -10.0, "VM11", "red")

    def save_position(self):
        players = self.mapa.jogadores
        with open("src/formations/posicoes_jogadores.txt", "w", encoding="utf-8") as f:
            f.write("positions={\n")
            for p in players:
                print(p)
                f.write(f"{{{p['x']},{p['y']}}},\n")
            f.write("};")

        print("Arquivo salvo com sucesso!")


app = App()
app.mainloop()
