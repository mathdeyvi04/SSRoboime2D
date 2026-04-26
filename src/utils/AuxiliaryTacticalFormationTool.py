import customtkinter as ctk
from tkinter import Canvas, Listbox
from tkinter import messagebox as msb
from PIL import Image, ImageTk

class AuxiliaryTacticalFormationTool(ctk.CTk):

    IMG_PATH = "./src/utils/FieldImageforTacticalFormationTool.png"

    def __init__(self):

        # Lógica de Posicionamento
        self.rows = 52
        self.columns = 34 * 2
        self.active_positions = {}
        self.total_formations = self.get_the_formations()

        # ----- Variáveis de Manipulação
        self.changed = False
        self.xfieldtocanvas = lambda x_on_the_field: 5.76471 * x_on_the_field + 299.76471
        self.yfieldtocanvas = lambda y_on_the_field: -7.51953 * y_on_the_field + 248.625

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
        self.btn1 = ctk.CTkButton(self.frame_superior, text="Ação 1", command=print)
        self.btn1.place(relx=0.65, rely=0.2, relwidth=0.32)

        self.btn2 = ctk.CTkButton(self.frame_superior, text="Ação 2", command=print)
        self.btn2.place(relx=0.65, rely=0.4, relwidth=0.32)

        self.btn3 = ctk.CTkButton(self.frame_superior, text="Ação 3", command=print)
        self.btn3.place(relx=0.65, rely=0.6, relwidth=0.32)

    def resizable_img(self, new_width: int = 600, new_height: int = 400) -> None:
        img_resizable = self.img.copy()
        img_resizable.thumbnail((new_width, new_height), Image.Resampling.LANCZOS)
        self.imgtk = ImageTk.PhotoImage(img_resizable)
        self.canvas.create_image(
            0, 0,
            image=self.imgtk,
            anchor="nw"
        )

    @staticmethod
    def get_the_formations() -> dict:
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
                            list(map(int, position_tuple))
                        )

        except FileNotFoundError:
            return {}

    def show_on_the_field(self):

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
        name_of_formation = self.listbox.get(selection[0])

        self.canvas.delete("all")
        self.resizable_img()

        w, h = self.canvas.winfo_width(), self.canvas.winfo_height()
        dx, dy = (w / 2) / self.columns, h / self.rows

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
        while number_rows < 14:
            # The function that describes the y of lines verticals: 8 + 5 * number_cols * dy
            self.canvas.create_line(
                13, 8 + 5 * number_rows * dy,
                w / 2, 8 + 5 * number_rows * dy,
                fill="blue",
                stipple="gray50"
            )
            number_rows += 1

        # Por enquanto, vamos apenas colocar todos os jogadores
        for i, player in enumerate(self.total_formations[name_of_formation]):
            x, y = self.xfieldtocanvas(player[0]), self.yfieldtocanvas(player[1])
            r = 10
            self.canvas.create_oval(
                x - r, y - r,
                x + r, y + r,
                fill="black"
            )
            self.canvas.create_text(
                x, y,
                text=str(i),
                fill="white",
                font=("Arial", 8, "bold")
            )


if __name__ == '__main__':
    AuxiliaryTacticalFormationTool().mainloop()