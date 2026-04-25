import re
from pathlib import Path
from tkinter import filedialog

import customtkinter as ctk


class SearchPath(ctk.CTkFrame):
    def __init__(self, master, on_positions_loaded=None, **kwargs):
        super().__init__(master, **kwargs)

        self.on_positions_loaded = on_positions_loaded

        self.grid_columnconfigure(0, weight=1)
        self.grid_columnconfigure(1, weight=0)

        self.label_titulo = ctk.CTkLabel(
            self,
            text="Selecione um arquivos",
            font=("Arial", 20),
        )
        self.label_titulo.grid(
            row=0,
            column=0,
            columnspan=2,
            padx=10,
            pady=20,
        )

        self.entry_caminho = ctk.CTkEntry(
            self,
            placeholder_text="Diretório do arquivo",
        )
        self.entry_caminho.grid(
            row=1,
            column=0,
            padx=(20, 5),
            pady=10,
            sticky="ew",
        )

        self.btn_buscar = ctk.CTkButton(
            self,
            text="Buscar",
            width=100,
            command=self.buscar_arquivo,
        )
        self.btn_buscar.grid(
            row=1,
            column=1,
            padx=(5, 20),
            pady=10,
        )

        self.label_resultado = ctk.CTkLabel(
            self,
            text="Nenhum arquivo selecionado",
            wraplength=500,
        )
        self.label_resultado.grid(
            row=2,
            column=0,
            columnspan=2,
            padx=10,
            pady=10,
        )

        self.entry_position = ctk.CTkEntry(
            self,
            placeholder_text="Posições encontradas no arquivo",
        )
        self.entry_position.grid(
            row=3,
            column=0,
            padx=(20, 5),
            pady=10,
            sticky="ew",
        )

        self.btn_set_position = ctk.CTkButton(
            self,
            text="Ler posições",
            width=100,
            command=self.carregar_posicoes_do_entry,
        )
        self.btn_set_position.grid(
            row=3,
            column=1,
            padx=(5, 20),
            pady=10,
        )

    def buscar_arquivo(self):
        caminho = filedialog.askopenfilename(
            title="Escolha um arquivo",
            filetypes=[
                ("Todos os arquivos", "*.*"),
                ("Arquivos C/C++ Header", "*.hpp"),
                ("Arquivos de texto", "*.txt"),
                ("Arquivos Python", "*.py"),
            ],
        )

        if not caminho:
            self.label_resultado.configure(text="Nenhum arquivo foi selecionado")
            return

        self.entry_caminho.delete(0, "end")
        self.entry_caminho.insert(0, caminho)
        self.label_resultado.configure(text=f"Arquivo selecionado:\n{caminho}")

        self.carregar_posicoes(caminho)

    def carregar_posicoes_do_entry(self):
        caminho = self.entry_caminho.get().strip()

        if not caminho:
            self.label_resultado.configure(text="Informe ou selecione um arquivo")
            return

        self.carregar_posicoes(caminho)

    def carregar_posicoes(self, caminho):
        try:
            posicoes = self.buscar_vetores(caminho)

            if not posicoes:
                self.entry_position.delete(0, "end")
                self.label_resultado.configure(
                    text="Nenhum vetor no formato esperado foi encontrado"
                )
                return

            texto_posicoes = self.formatar_posicoes(posicoes)

            self.entry_position.delete(0, "end")
            self.entry_position.insert(0, texto_posicoes)
            self.label_resultado.configure(
                text=f"{len(posicoes)} posições carregadas com sucesso"
            )

            if callable(self.on_positions_loaded):
                self.on_positions_loaded(posicoes)

        except FileNotFoundError:
            self.label_resultado.configure(text="Arquivo não encontrado")
        except UnicodeDecodeError:
            self.label_resultado.configure(text="Erro de leitura: arquivo não é UTF-8")
        except Exception as e:
            self.label_resultado.configure(text=f"Erro ao processar arquivo: {e}")

    def buscar_vetores(self, caminho):
        caminho = Path(caminho)

        with caminho.open("r", encoding="utf-8") as arquivo:
            conteudo = arquivo.read()

        bloco_vetor = re.search(r"=\s*\{(.*)\};", conteudo, re.DOTALL)

        if not bloco_vetor:
            return []

        texto_interno = bloco_vetor.group(1)
        padrao_par = r"\{\s*([-+]?\d*\.?\d+)\s*,\s*([-+]?\d*\.?\d+)\s*\}"
        pares = re.findall(padrao_par, texto_interno, re.DOTALL)

        posicoes = []
        for x, y in pares:
            num_x = float(x)
            num_y = float(y)

            val_x = int(num_x) if num_x.is_integer() else num_x
            val_y = int(num_y) if num_y.is_integer() else num_y

            posicoes.append((val_x, val_y))

        return posicoes

    def formatar_posicoes(self, posicoes):
        return "(" + ",".join(f"({x}, {y})" for x, y in posicoes) + ")"

    def obter_posicoes(self):
        caminho = self.entry_caminho.get().strip()
        if not caminho:
            return []
        return self.buscar_vetores(caminho)
