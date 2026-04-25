import customtkinter as ctk
from tkinter import Canvas
from PIL import Image, ImageTk

class AuxiliaryTacticalFormationTool(ctk.CTk):

    IMG_PATH = "./FieldImageforTacticalFormationTool.png"

    def __init__(self):

        # Lógica de Posicionamento
        self.rows = 52
        self.columns = 34 * 2
        self.active_positions = {}


        # Lógica de Interface
        super().__init__()
        # Configurar tema
        ctk.set_appearance_mode("dark")
        ctk.set_default_color_theme("green")

        # Janela principal
        self.title("TacticalFormationTool")
        self.width, self.height = 1000, 800
        self.geometry(f"{self.width}x{self.height}")
        self.resizable(False, False)

        self.img = Image.open(AuxiliaryTacticalFormationTool.IMG_PATH)
        self.imgtk = None

        # Frame superior
        self.frame_superior = ctk.CTkFrame(self, height=self.height//2)
        self.frame_superior.pack(fill="both", padx=10, pady=(10, 5))

        # Frame inferior
        self.frame_inferior = ctk.CTkFrame(self, height=self.height//2)
        self.frame_inferior.pack(fill="both", padx=10, pady=(5, 10))

        # Canvas
        self.canvas = Canvas(
            self.frame_inferior
        )
        self.canvas.pack(fill="both", expand=True, padx=10, pady=10)
        self.resizable_img(new_width=self.width, new_height=self.height // 2)
        self.canvas.create_image(
            0, 0,
            image=self.imgtk,
            anchor="nw"
        )

    def resizable_img(self, new_width: int, new_height: int) -> None:
        img_resizable = self.img.copy()
        img_resizable.thumbnail((new_width, new_height), Image.Resampling.LANCZOS)
        self.imgtk = ImageTk.PhotoImage(img_resizable)


if __name__ == '__main__':
    AuxiliaryTacticalFormationTool().mainloop()