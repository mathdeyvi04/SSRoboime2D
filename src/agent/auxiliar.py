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
            largura=600,
            altura=400,
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

        self.mapa.add_player(80, 300, "AZ1", "blue")
        self.mapa.add_player(180, 90, "AZ2", "blue")
        self.mapa.add_player(180, 230, "AZ3", "blue")
        self.mapa.add_player(180, 370, "AZ4", "blue")
        self.mapa.add_player(180, 510, "AZ5", "blue")
        self.mapa.add_player(340, 120, "AZ6", "blue")
        self.mapa.add_player(340, 240, "AZ7", "blue")
        self.mapa.add_player(340, 360, "AZ8", "blue")
        self.mapa.add_player(340, 480, "AZ9", "blue")
        self.mapa.add_player(500, 220, "AZ10", "blue")
        self.mapa.add_player(500, 380, "AZ11", "blue")

        self.mapa.add_player(520, 300, "VM1", "red")
        self.mapa.add_player(520, 90, "VM2", "red")
        self.mapa.add_player(520, 230, "VM3", "red")
        self.mapa.add_player(520, 370, "VM4", "red")
        self.mapa.add_player(520, 510, "VM5", "red")
        self.mapa.add_player(560, 120, "VM6", "red")
        self.mapa.add_player(560, 240, "VM7", "red")
        self.mapa.add_player(560, 360, "VM8", "red")
        self.mapa.add_player(560, 380, "VM9", "red")
        self.mapa.add_player(400, 220, "VM10", "red")
        self.mapa.add_player(400, 380, "VM11", "red")

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
