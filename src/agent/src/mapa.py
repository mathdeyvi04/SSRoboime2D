import tkinter as tk

import customtkinter as ctk
from PIL import Image, ImageTk


class FieldMap(ctk.CTkFrame):
    def __init__(self, master, imagem_fundo, largura=900, altura=600):
        super().__init__(master)

        self.largura = largura
        self.altura = altura
        self.imagem_fundo_path = imagem_fundo

        self.canvas = tk.Canvas(
            self,
            width=self.largura,
            height=self.altura,
            highlightthickness=0,
            bg="black",
        )
        self.canvas.pack(fill="both", expand=True)

        self.carregar_fundo()
        self.jogadores = []
        self.jogador_selecionado = None
        self.offset_x = 0
        self.offset_y = 0

    def carregar_fundo(self):
        imagem = Image.open(self.imagem_fundo_path)
        imagem = imagem.resize((self.largura, self.altura))
        self.fundo_pil = imagem
        self.fundo_tk = ImageTk.PhotoImage(imagem)

        self.canvas.create_image(0, 0, image=self.fundo_tk, anchor="nw")

    def add_player(self, x, y, nome="Jogador", cor="red"):
        raio = 5

        circulo = self.canvas.create_oval(
            x - raio,
            y - raio,
            x + raio,
            y + raio,
            fill=cor,
            outline="white",
            width=2,
        )

        texto = self.canvas.create_text(
            x,
            y - 18,
            text=nome,
            fill="white",
            font=("Arial", 10, "bold"),
        )

        jogador = {
            "nome": nome,
            "x": x,
            "y": y,
            "cor": cor,
            "circulo": circulo,
            "texto": texto,
        }

        self.jogadores.append(jogador)

        self.canvas.tag_bind(
            circulo,
            "<Button-1>",
            lambda event, j=jogador: self.selecionar_jogador(event, j),
        )
        self.canvas.tag_bind(
            texto,
            "<Button-1>",
            lambda event, j=jogador: self.selecionar_jogador(event, j),
        )

        self.canvas.tag_bind(
            circulo, "<Button-3>", lambda event, j=jogador: self.remover_jogador(j)
        )
        self.canvas.tag_bind(
            texto, "<Button-3>", lambda event, j=jogador: self.remover_jogador(j)
        )

        self.canvas.tag_bind(
            circulo, "<B1-Motion>", lambda event, j=jogador: self.move_player(event, j)
        )
        self.canvas.tag_bind(
            texto, "<B1-Motion>", lambda event, j=jogador: self.move_player(event, j)
        )
        self.canvas.tag_bind(
            circulo,
            "<ButtonRelease-1>",
            lambda event, j=jogador: self.release_player(event, j),
        )
        self.canvas.tag_bind(
            texto,
            "<ButtonRelease-1>",
            lambda event, j=jogador: self.release_player(event, j),
        )

    def adicionar_jogador_click(self, event):
        nome = f"J{len(self.jogadores) + 1}"
        self.add_player(event.x, event.y, nome=nome, cor="blue")

    def remover_jogador(self, jogador):
        self.canvas.delete(jogador["circulo"])
        self.canvas.delete(jogador["texto"])

        if jogador in self.jogadores:
            self.jogadores.remove(jogador)

    def selecionar_jogador(self, event, jogador):
        self.jogador_selecionado = jogador

        self.offset_x = event.x - jogador["x"]
        self.offset_y = event.y - jogador["y"]

    def release_player(self, event, jogador):
        self.jogador_selecionado = None

    def move_player(self, event, jogador):
        if not hasattr(self, "jogador_selecionado") or self.jogador_selecionado is None:
            return

        jogador = self.jogador_selecionado
        raio = 5

        novo_x = event.x - self.offset_x
        novo_y = event.y - self.offset_y

        jogador["x"] = novo_x
        jogador["y"] = novo_y

        self.canvas.coords(
            jogador["circulo"],
            novo_x - raio,
            novo_y - raio,
            novo_x + raio,
            novo_y + raio,
        )

        self.canvas.coords(
            jogador["texto"],
            novo_x,
            novo_y - 18,
        )

    def limpar_jogadores(self):
        for jogador in self.jogadores:
            self.canvas.delete(jogador["circulo"])
            self.canvas.delete(jogador["texto"])
        self.jogadores.clear()
