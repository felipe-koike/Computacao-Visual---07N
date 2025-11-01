import customtkinter as ctk
from tkinter import filedialog
from PIL import Image, UnidentifiedImageError, ImageOps
import numpy as np
import os
import sys

# --- Importações do PyTorch ---
import torch
import torch.nn as nn
import torchvision.models as models
from torchvision import transforms
# ------------------------------


# Limite de 20MB para o tamanho do arquivo
MAX_FILE_SIZE = 20 * 1024 * 1024  # 20 Megabytes
# Tamanho da miniatura na UI
THUMBNAIL_SIZE = (100, 100)

# --- INFORMAÇÕES DO SEU NOTEBOOK ---
# O seu notebook treinou com imagens de 224x224
MODEL_IMG_SIZE = (224, 224)
# ------------------------------------


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


# --- Carregamento de Classes (Original do seu app.py) ---
print("Carregando classes")
CLASSES_PATH = resource_path("class_names.txt")
try:
    with open(CLASSES_PATH, "r") as f:
        CLASS_NAMES = [line.strip() for line in f.readlines()]
except Exception as e:
    print(f"Erro ao ler class_names.txt: {e}")
    CLASS_NAMES = ["Class A", "Class B", "Class C", "Class D"] # Fallback

CLASS_DESCRIPTIONS = {
    "Class A": "Verde",
    "Class B": "Parcialmente Madura",
    "Class C": "Madura",
    "Class D": "Passada"
}


# --- 1. PRÉ-PROCESSAMENTO (Baseado no seu notebook) ---
#
# Seu notebook usou 'Resize' e 'ToTensor', sem 'Normalize'.
#
try:
    preprocess = transforms.Compose([
        transforms.Resize(MODEL_IMG_SIZE),  # Redimensiona para (224, 224)
        transforms.ToTensor(),              # Converte para Tensor (C, H, W) e escala [0, 1]
    ])
except Exception as e:
    print(f"Erro ao criar pipeline de transformações: {e}")
    preprocess = None

# -----------------------------------------------------------


# --- 2. CARREGAMENTO DO MODELO (Baseado no seu notebook) ---
#
# !! ATENÇÃO: Altere o nome do arquivo .pth se for diferente !!
#
print("Carregando modelo")
MODEL_PATH = resource_path("banana_classifier_model.pth") # <-- MUDE SE NECESSÁRIO
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
print(f"Usando dispositivo: {device}")

try:
    # Recria a arquitetura EXATA do notebook (ResNet18)
    num_classes = len(CLASS_NAMES) # Deve ser 4
    
    # Inicia a ResNet18 sem pesos pré-treinados, pois o .pth já contém todos
    model = models.resnet18(weights=None) 
    
    # Ajusta a camada final para 4 classes (como no notebook)
    model.fc = torch.nn.Linear(model.fc.in_features, num_classes)
    
    # Carrega o state_dict (os pesos) que você salvou
    model.load_state_dict(torch.load(MODEL_PATH, map_location=device, weights_only=True))    
    model = model.to(device)
    model.eval() # IMPORTANTE: Coloca o modelo em modo de avaliação
    print(f"Modelo PyTorch carregado com sucesso de: {MODEL_PATH}")

except Exception as e:
    print(f"Erro ao carregar modelo PyTorch: {e}")
    print("Verifique se o NOME DO ARQUIVO .pth está correto em MODEL_PATH.")
    model = None

# -----------------------------------------------------------


def predict_image(pil_image):
    """
    Faz a predição de um objeto de imagem PIL usando PyTorch.
    Retorna um tuple (sucesso, mensagem).
    """
    if model is None:
        return (False, "Erro: Modelo PyTorch não carregado.\nVerifique o console.")
        
    if preprocess is None:
        return (False, "Erro: Pipeline de transformação não foi criado.")

    try:
        # Converte para RGB
        img_rgb = pil_image.convert('RGB')
        
        # Aplicar transformações de pré-processamento (Resize e ToTensor)
        img_tensor = preprocess(img_rgb)
        
        # Adicionar dimensão do batch (PyTorch espera: B, C, H, W)
        img_tensor = img_tensor.unsqueeze(0) # Shape: [1, 3, 224, 224]
        img_tensor = img_tensor.to(device) # Mover para o dispositivo

        # Fazer predição (desativando cálculo de gradiente)
        with torch.no_grad():
            outputs = model(img_tensor)
        
        # Aplicar Softmax para obter probabilidades
        probabilities = torch.nn.functional.softmax(outputs[0], dim=0)
        
        # Obter a classe e a confiança
        confidence_tensor, predicted_class_index_tensor = torch.max(probabilities, 0)
        
        # Converter tensores para valores Python
        confidence = confidence_tensor.item() * 100
        predicted_class_index = predicted_class_index_tensor.item()

        predicted_class_name = CLASS_NAMES[predicted_class_index]

        description = CLASS_DESCRIPTIONS.get(predicted_class_name, "Desconhecida")
        msg = f"Resultado: [{description}] ({predicted_class_name})\nConfiança: {confidence:.2f}%"
        return (True, msg)  # Retorna Sucesso

    except Exception as e:
        return (False, f"Erro ao processar imagem (PyTorch): {e}")


# --- O resto do código (GUI) permanece o mesmo ---

class App(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.title("Analisador de Maturação")
        self.geometry("500x400") 
        
        ctk.set_appearance_mode("System")
        ctk.set_default_color_theme("blue")

        self.main_frame = ctk.CTkFrame(self, corner_radius=10)
        self.main_frame.pack(padx=20, pady=20, fill="both", expand=True)

        self.title_label = ctk.CTkLabel(self.main_frame, text="Analisador de Maturação", 
                                        font=ctk.CTkFont(size=20, weight="bold"))
        self.title_label.pack(pady=(20, 10))

        self.upload_button = ctk.CTkButton(self.main_frame, text="Carregar Imagem",
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
            
            # Converte para RGB imediatamente após abrir
            img = Image.open(file_path).convert('RGB') 
            img = ImageOps.exif_transpose(img) # Corrige orientação

            thumb = img.copy()
            thumb.thumbnail(THUMBNAIL_SIZE, Image.LANCZOS)
            ctk_img = ctk.CTkImage(light_image=thumb, dark_image=thumb, size=THUMBNAIL_SIZE)
            self.image_label.configure(image=ctk_img, text="")
            
            # --- Chamada para a nova função PyTorch ---
            success, message = predict_image(img)
            # ----------------------------------------
            
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