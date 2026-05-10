import customtkinter as ctk
from tkinter import Canvas, Listbox, Event
from tkinter import messagebox as msb, simpledialog
from PIL import Image, ImageTk

class AuxiliaryTacticalFormationTool(ctk.CTk):
    """
    @brief Ferramenta gráfica para criação e edição de formações táticas.
    """
    IMG_PATH = "./src/utils/FieldImageforTacticalFormationTool.png"

    def __init__(self):
        """
        @brief Inicializa a interface, variáveis e eventos da ferramenta.
        """
        # Lógica de Posicionamento
        self.possible_rows = 52
        self.possible_columns = 34 * 2
        self.active_positions = {
            # Armazenará (n_col, n_row) = unum
        }
        self.total_formations = self.get_the_formations()
        self.name_of_formation = None

        # ----- Variáveis de Manipulação
        self.changed = False
        self.unum_pressed = 0
        self.xfieldtocanvas = lambda x_on_field: 5.29533 * x_on_field + 298.94773
        self.yfieldtocanvas = lambda y_on_field: -5.78125 * y_on_field + 193

        self.xcanvastofield = lambda xcanvas: (xcanvas - 298.94773) * 0.188845643237
        self.ycanvastofield = lambda ycanvas: (193 - ycanvas) * 0.172972972973

        self.xcanvastomesh = lambda xcanvas: round((xcanvas - 13) * 0.0453333333334)
        self.ycanvastomesh = lambda ycanvas: round((ycanvas - 8) * 0.027012987013)

        self.xmeshtocanvas = lambda n_col: 13 + 5 * n_col * 4.411764705882353
        self.ymeshtocanvas = lambda n_row: 8 + 5 * n_row * 7.403846153846154

        # Lógica de Interface
        super().__init__()
        # Configurar tema
        ctk.set_appearance_mode("dark")
        ctk.set_default_color_theme("green")

        # Janela principal
        self.title("TacticalFormationTool")
        self.width, self.height = 620, 800
        self.geometry(f"{self.width}x{self.height}")
        self.resizable(False, False)

        self.img = Image.open(AuxiliaryTacticalFormationTool.IMG_PATH)
        self.imgtk = None

        # Frame superior
        self.frame_superior = ctk.CTkFrame(
            self,
            width=self.width - 20,
            height=(self.height // 2) - 15
        )
        self.frame_superior.place(
            x=10,
            y=10
        )

        # Frame inferior
        self.frame_inferior = ctk.CTkFrame(
            self,
            width=self.width - 20,
            height=(self.height // 2) - 15
        )
        self.frame_inferior.place(
            x=10,
            y=(self.height // 2) + 5
        )

        # Canvas
        self.canvas = Canvas(
            self.frame_inferior
        )
        self.canvas.place(x=0, y=0, relwidth=1, relheight=1)
        self.resizable_img(new_width=600, new_height=400)

        # --- LISTBOX (nomes) ---
        self.listbox = Listbox(
            self.frame_superior,
            bg="#00170b",  # fundo
            fg="white",  # texto
            selectbackground="#1f6aa5",  # fundo do item selecionado
            selectforeground="white",  # texto selecionado
            highlightthickness=1,  # remove borda feia padrão
            bd=0  # remove borda
        )
        self.listbox.config(justify="center")
        self.listbox.place(relx=0.02, rely=0.05, relwidth=0.6, relheight=0.9)
        self.listbox.bind("<<ListboxSelect>>", lambda event: self.show_on_the_field())
        for name in self.total_formations:
            self.listbox.insert("end", name)

        # --- BOTÕES ---
        self.btn1 = ctk.CTkButton(self.frame_superior, text="Salvar", command=self.save)
        self.btn1.place(relx=0.65, rely=0.2, relwidth=0.32)

        self.btn2 = ctk.CTkButton(self.frame_superior, text="Criar Nova Formação", command=self.create_new_formation)
        self.btn2.place(relx=0.65, rely=0.4, relwidth=0.32)

        self.btn3 = ctk.CTkButton(self.frame_superior, text="Deletar", command=self.deletar)
        self.btn3.place(relx=0.65, rely=0.6, relwidth=0.32)

        # Bindings atualizados
        self.canvas.bind("<Button-1>", self._on_press)
        self.protocol("WM_DELETE_WINDOW", self.on_close)

    def resizable_img(self, new_width: int = 600, new_height: int = 400) -> None:
        """
        @brief Redimensiona e renderiza a imagem do campo no canvas.

        @param new_width Nova largura da imagem.
        @param new_height Nova altura da imagem.
        """

        img_resizable = self.img.copy()
        img_resizable.thumbnail((new_width, new_height), Image.Resampling.LANCZOS)
        self.imgtk = ImageTk.PhotoImage(img_resizable)
        self.canvas.create_image(
            0, 0,
            image=self.imgtk,
            anchor="nw"
        )

    @staticmethod
    def get_the_formations() -> dict | None:
        """
        @brief Carrega as formações do arquivo TacticalFormations.hpp.

        @return Dicionário contendo as formações carregadas.
        """
        try:
            with open(
                    "./src/booting/TacticalFormations.hpp",
                    'r'
            ) as f:
                formations = {}
                we_reached_the_declarations = False
                name_of_formation = None
                for linha in f:
                    linha = linha.strip()

                    if linha.startswith("namespace"):
                        we_reached_the_declarations = True
                        continue

                    if we_reached_the_declarations:
                        if name_of_formation is None:
                            if linha.startswith("}"):
                                return formations
                            conj_words = linha.split(' ')
                            name_of_formation = conj_words[3].replace("[]", '')
                            formations[name_of_formation] = []
                            continue

                        if linha.startswith('};'):
                            name_of_formation = None
                            continue

                        position_tuple = linha.replace(',', '').split(' ')
                        formations[name_of_formation].append(
                            list(map(float, position_tuple))
                        )

        except FileNotFoundError:
            return {}

    def render_field(self):
        """
        @brief Renderiza o campo e a malha de posicionamento.
        """

        self.canvas.delete("all")
        self.resizable_img()

        w, h = self.canvas.winfo_width(), self.canvas.winfo_height()
        dx, dy = (w / 2) / self.possible_columns, h / self.possible_rows

        number_cols = 0
        while number_cols < 14:
            # The function that describes the x of lines verticals: 13 + 5 * number_cols * dx
            self.canvas.create_line(
                13 + 5 * number_cols * dx, 8,
                13 + 5 * number_cols * dx, h - 7,
                fill="blue",
                stipple="gray50"
            )
            number_cols += 1

        number_rows = 0
        # Apesar de estar escrito 11, são 10. Há erro de renderização de linhas
        while number_rows < 11:
            # The function that describes the y of lines horizontals: 8 + 5 * number_cols * dy
            self.canvas.create_line(
                13, 8 + 5 * number_rows * dy,
                    w / 2, 8 + 5 * number_rows * dy,
                fill="blue",
                stipple="gray50"
            )
            number_rows += 1

    def show_on_the_field(self):
        """
        @brief Exibe no campo a formação selecionada na listbox.
        """
        if self.changed:
            # Então houve alteração do que estamos nesse momento.
            if not msb.askokcancel(
                "Houve Alteração",
                "Você alterou a última formação, deseja realmente descartar as alterações?"
            ):
                return

            # Como vamos carregar uma do zero, não há alteração nela
            self.changed = False

        selection = self.listbox.curselection()
        self.name_of_formation = self.listbox.get(selection[0])

        self.render_field()

        # Por enquanto, vamos apenas colocar todos os jogadores
        self.active_positions = {}
        for i, player in enumerate(self.total_formations[self.name_of_formation]):
            x, y = self.xfieldtocanvas(player[0]), self.yfieldtocanvas(player[1])
            self.active_positions[(self.xcanvastomesh(x), self.ycanvastomesh(y))] = i + 1
            r = 10
            self.canvas.create_oval(
                x - r, y - r,
                x + r, y + r,
                fill="black"
            )
            self.canvas.create_text(
                x, y,
                text=str(i + 1),
                fill="white",
                font=("Arial", 8, "bold")
            )

    def render_all(self) -> None:
        """
        @brief Redesenha o campo e todos os jogadores ativos.
        """
        self.render_field()

        # Então colocamos as posições ativas
        for n_col_and_n_row in self.active_positions:
            x, y = self.xmeshtocanvas(n_col_and_n_row[0]), self.ymeshtocanvas(n_col_and_n_row[1])
            r = 10
            self.canvas.create_oval(
                x - r, y - r,
                x + r, y + r,
                fill="black"
            )
            self.canvas.create_text(
                x, y,
                text=self.active_positions[n_col_and_n_row],
                fill="white",
                font=("Arial", 8, "bold")
            )

    def on_close(self) -> None:
        """
        @brief Salva as formações no arquivo e encerra a aplicação.
        """

        if self.changed:
            # Então houve alteração do que estamos nesse momento.
            if not msb.askokcancel(
                    "Houve Alteração",
                    "Experimente salvar a formação ou sinta as consequências."
            ):
                return

        with open(
                "./src/booting/TacticalFormations.hpp",
                'w'
        ) as f:
            f.write("#pragma once\n\n// Deve seguir exatamente o padrão que colocamos\nnamespace TacticalFormations {\n")
            for name_of_formation in self.total_formations:
                f.write(f"\tinline constexpr double {name_of_formation}[] = ")
                f.write("{\n")
                for xpos, ypos in self.total_formations[name_of_formation]:
                    f.write(f"\t\t{xpos}, {ypos},\n")
                f.write("\t};\n")

            f.write("}")

        # Vamos salvar o total_formations no arquivo .hpp
        self.destroy()

    def save(self) -> None:
        """
        @brief Salva a formação atualmente editada.
        """
        if not self.changed:
            # Caso não tenha feito nada, não faz sentido salvar
            return

        if self.name_of_formation is not None:
            self.total_formations[self.name_of_formation] = [
                [
                    self.xcanvastofield(self.xmeshtocanvas(n_col_and_n_row[0])),
                    self.ycanvastofield(self.ymeshtocanvas(n_col_and_n_row[1]))
                ] for n_col_and_n_row in self.active_positions
            ]

        self.changed = False

    def create_new_formation(self) -> None:
        """
        @brief Cria uma formação vazia.
        """
        new_formation = simpledialog.askstring(
            "Criando Nova Formação",
            "Insira o nome da formação:"
        )

        if new_formation is None:
            return

        if new_formation in self.total_formations:
            msb.showerror("Error", "Nome de Formação já existe. Escolha outro.")
            return

        self.listbox.insert("end", new_formation)
        self.name_of_formation = new_formation
        self.active_positions = {}
        self.render_all()

    def deletar(self) -> None:
        """
        @brief Remove a formação atualmente selecionada.
        """
        if not msb.askokcancel(
            "Confirmação",
            "Deseja mesmo deletar a formação?"
        ):
            return


        if self.name_of_formation in self.total_formations:
            self.total_formations.pop(self.name_of_formation)
            for i in range(self.listbox.size()):
                if self.listbox.get(i) == self.name_of_formation:
                    self.listbox.delete(i)
                    break

        self.name_of_formation = None
        self.active_positions = {}
        self.render_all()

    def _on_press(self, event: Event) -> None:
        """
        @brief Processa cliques no campo para adicionar ou remover jogadores.

        @param event Evento de clique do mouse.
        """
        self.changed = True
        xcanvas, ycanvas = event.x, event.y
        n_col_and_n_row = (self.xcanvastomesh(xcanvas), self.ycanvastomesh(ycanvas))

        if n_col_and_n_row in self.active_positions:

            # E então guardar informação que ele está selecionado
            self.unum_pressed = self.active_positions[n_col_and_n_row]
            # Precisamos retirá-lo.
            self.active_positions.pop(n_col_and_n_row)
            # E redesenhar tudo
            self.render_all()
        else:

            # Devemos tomar cuidado se chegou no final
            if len(self.active_positions.values()) == 11:
                # Caso isso, nada acontecerá e podemos retornar ao estado anterior
                self.changed = False
                return

            if not self.unum_pressed:
                # Se não há ninguém pressionado, precisamos colocar o com menor número
                unum_in_active_positions = set(self.active_positions.values())
                self.unum_pressed = 1
                while self.unum_pressed in unum_in_active_positions:
                    self.unum_pressed += 1

            # Então vamos colocar algo aqui
            self.active_positions[n_col_and_n_row] = self.unum_pressed

            # E renderizar tudo
            self.render_all()

            # Voltar ao estágio que não havia nada selecionado
            self.unum_pressed = 0

if __name__ == '__main__':
    AuxiliaryTacticalFormationTool().mainloop()