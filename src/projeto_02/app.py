import customtkinter as ctk
from tkinter import filedialog
from PIL import Image, UnidentifiedImageError, ImageOps
import tensorflow as tf
import numpy as np
import os
import sys



# Limite de 20MB para o tamanho do arquivo
MAX_FILE_SIZE = 20 * 1024 * 1024  # 20 Megabytes
# Confiança mínima para considerar uma predição válida
CONFIDENCE_THRESHOLD = 60.0  # 60%
# Tamanho da miniatura na UI
THUMBNAIL_SIZE = (100, 100)


MODEL_IMG_SIZE = (180, 180)



def resource_path(relative_path):
    """ 
    Gera o caminho absoluto para o recurso. 
    Funciona tanto no modo de desenvolvimento quanto no executável do PyInstaller.
    """
    try:
        base_path = sys._MEIPASS
    except Exception:
        base_path = os.path.abspath(".")
    return os.path.join(base_path, relative_path)



MODEL_PATH = resource_path("banana_classifier_model.h5") 
CLASSES_PATH = resource_path("class_names.txt")

try:
    model = tf.keras.models.load_model(MODEL_PATH, compile=False)
except Exception as e:
    print(f"Erro ao carregar modelo: {e}")
    exit()

with open(CLASSES_PATH, "r") as f:
    CLASS_NAMES = [line.strip() for line in f.readlines()]

CLASS_DESCRIPTIONS = {
    "Class A": "Verde",
    "Class B": "Parcialmente Madura",
    "Class C": "Madura",
    "Class D": "Passada"
}


def predict_image(pil_image):
    """
    Faz a predição de um objeto de imagem PIL.
    Retorna um tuple (sucesso, mensagem).
    """
    try:
        # Converte para RGB
        img_rgb = pil_image.convert('RGB')
        
        # Redimensiona a imagem para o tamanho que o modelo espera (180x180)
        img_resized = img_rgb.resize(MODEL_IMG_SIZE)
        
        img_array = tf.keras.preprocessing.image.img_to_array(img_resized)
        img_array = tf.expand_dims(img_array, 0)

        predictions = model.predict(img_array)
        

        score = tf.nn.softmax(predictions[0])

        predicted_class_index = np.argmax(score)
        predicted_class_name = CLASS_NAMES[predicted_class_index]
        confidence = 100 * np.max(score)


        description = CLASS_DESCRIPTIONS.get(predicted_class_name, "Desconhecida")
        msg = f"Resultado: [{description}] ({predicted_class_name})\nConfiança: {confidence:.2f}%"
        return (True, msg)  # Retorna Sucesso

    except Exception as e:
        return (False, f"Erro ao processar imagem: {e}")


class App(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.title("Analisador de Maturação de Banana")
        self.geometry("500x400") 
        
        ctk.set_appearance_mode("System")
        ctk.set_default_color_theme("blue")

        self.main_frame = ctk.CTkFrame(self, corner_radius=10)
        self.main_frame.pack(padx=20, pady=20, fill="both", expand=True)

        self.title_label = ctk.CTkLabel(self.main_frame, text="Analisador de Maturação de Bananas", 
                                        font=ctk.CTkFont(size=20, weight="bold"))
        self.title_label.pack(pady=(20, 10))

        self.upload_button = ctk.CTkButton(self.main_frame, text="Carregar Imagem da Banana",
                                           command=self.open_image_file, 
                                           font=ctk.CTkFont(size=14))
        self.upload_button.pack(pady=10, padx=40, fill="x")

        # Label da imagem (miniatura)
        self.image_label = ctk.CTkLabel(self.main_frame, text="", 
                                        width=THUMBNAIL_SIZE[0], height=THUMBNAIL_SIZE[1])
        self.image_label.pack(pady=10)
        
        # Label de resultado
        self.result_label = ctk.CTkLabel(self.main_frame, text="Aguardando imagem...",
                                         font=ctk.CTkFont(size=16),
                                         wraplength=400)
        self.result_label.pack(pady=(10, 20), fill="x", expand=True)

        # Cores padrão
        self.success_color = ctk.ThemeManager.theme["CTkLabel"]["text_color"]
        self.warning_color = "orange"
        self.error_color = "red"


    def open_image_file(self):
        """Abre a caixa de diálogo e lida com a validação do arquivo."""
        file_path = filedialog.askopenfilename(
            title="Selecione uma imagem",
            filetypes=[("Arquivos de Imagem", "*.png *.jpg *.jpeg *.bmp *.webp")]
        )
        
        if not file_path:
            return # Usuário cancelou

        try:
            file_size = os.path.getsize(file_path)
            if file_size > MAX_FILE_SIZE:
                mb_size = MAX_FILE_SIZE // (1024 * 1024)
                self.show_message(f"Erro: Imagem muito grande.\n(Máximo: {mb_size} MB)", "error")
                return
        except OSError as e:
            self.show_message(f"Erro ao ler arquivo: {e}", "error")
            return

        try:
            self.result_label.configure(text="Analisando...", text_color="gray")
            self.update_idletasks()
            
            img = Image.open(file_path)
            img = ImageOps.exif_transpose(img)

            thumb = img.copy()
            thumb.thumbnail(THUMBNAIL_SIZE, Image.LANCZOS)
            ctk_img = ctk.CTkImage(light_image=thumb, dark_image=thumb, size=THUMBNAIL_SIZE)
            self.image_label.configure(image=ctk_img, text="")
            
            success, message = predict_image(img)
            
            if success:
                self.show_message(message, "success")
            else:
                self.show_message(message, "warning")

        except UnidentifiedImageError:
            self.show_message("Arquivo inválido.\nPor favor, selecione uma imagem (PNG, JPG, etc.).", "error")
        except Image.DecompressionBombError:
            self.show_message("Erro: Imagem muito grande (muitos pixels).\nTente uma imagem com resolução menor.", "error")
        except Exception as e:
            self.show_message(f"Erro inesperado: {e}", "error")


    def show_message(self, message, msg_type="success"):
        """Atualiza o label de resultado com a cor correta."""
        if msg_type == "success":
            color = self.success_color
        elif msg_type == "warning":
            color = self.warning_color
        elif msg_type == "error":
            color = self.error_color
            self.image_label.configure(image=None, text="")
        else:
            color = self.success_color
            
        self.result_label.configure(text=message, text_color=color)

if __name__ == "__main__":
    app = App()
    app.mainloop()