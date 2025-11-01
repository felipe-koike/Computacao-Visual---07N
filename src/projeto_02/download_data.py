import kagglehub
import os
import shutil

dataset_name = "luischuquimarca/banana-ripeness"
extract_path = r"D:\\ComputacaoVisual\\projeto02"

print(f"Baixando o dataset '{dataset_name}'...")
path = kagglehub.dataset_download(dataset_name)
print(f"Dataset baixado para: {path}")

if not os.path.exists(extract_path):
    os.makedirs(extract_path)

print(f"Copiando arquivos para '{extract_path}'...")
shutil.copytree(path, extract_path, dirs_exist_ok=True)

print("Download e cópia concluídos!")
print(f"Seus dados estão em: {os.path.abspath(extract_path)}")
