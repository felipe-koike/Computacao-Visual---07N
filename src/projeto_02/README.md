# 🍌 Analisador de Maturação de Banana

Este é um projeto de Deep Learning e Visão Computacional que utiliza uma **Rede Neural Convolucional (CNN)** para classificar o estágio de maturação de bananas.

O aplicativo de desktop (GUI) permite que um usuário faça o upload de uma imagem e receba uma classificação em tempo real, informando um dos quatro estágios de maturação:

* **Class A** - Verde
* **Class B** - Parcialmente Madura
* **Class C** - Madura
* **Class D** - Passada

![Imagem do aplicativo em funcionamento](https://i.imgur.com/SEU_LINK_DE_IMAGEM_AQUI.png) ---

## 🧠 Conceitos de Inteligência Artificial (IA)

Este aplicativo é um exemplo clássico de **inferência de modelo**, que é o processo de usar um modelo de IA já treinado para fazer previsões sobre dados novos.

### 1. Rede Neural Convolucional (CNN)
O coração deste projeto é o arquivo `banana_classifier_model.h5`. Este é um modelo de Deep Learning, especificamente uma CNN, que foi treinado em milhares de imagens para *aprender* a identificar padrões visuais (como cor, textura e manchas) associados a cada estágio de maturação da banana.

### 2. Classificação de Imagem
A tarefa de IA sendo executada é a **classificação de imagem**. O modelo recebe uma imagem como entrada e a "força" em uma das 4 categorias (classes) que ele conhece.

### 3. Logits e Softmax
O modelo que usamos foi treinado com a perda `from_logits=True`. Isso significa que a saída bruta do modelo (o `predictions`) não são probabilidades, mas sim números brutos chamados **logits**.
Na função `predict_image`, aplicamos `tf.nn.softmax(predictions[0])` para converter esses logits em um vetor de probabilidades (por exemplo: `[0.1, 0.05, 0.8, 0.05]`), onde cada número representa a "confiança" do modelo para uma classe, e a soma de todos é 1.0 (ou 100%).

### 4. Inferência e Predição
O processo completo de `predict_image` é a **inferência**. Nós pegamos a probabilidade mais alta usando `np.argmax(score)` para determinar a classe vencedora e `np.max(score)` para obter o nível de confiança dessa previsão.

---

## 👁️ Conceitos de Computação Visual (CV)

Antes que a IA possa "ver" a imagem, ela precisa ser formatada corretamente. Este é o trabalho da Computação Visual, e acontece principalmente na função `predict_image`. Uma imagem bruta do usuário não pode ser usada diretamente pelo modelo.

O *pipeline* de pré-processamento de CV neste código é:

1.  **Conversão de Espaço de Cor (`.convert('RGB')`)**
    * **Por quê?** O modelo foi treinado em imagens coloridas de 3 canais (Red, Green, Blue). Se um usuário enviar uma imagem em tons de cinza (1 canal) ou um PNG com transparência (4 canais, RGBA), o modelo falharia. Esta linha garante que *todas* as imagens de entrada sejam padronizadas para RGB.

2.  **Redimensionamento (`.resize(MODEL_IMG_SIZE)`)**
    * **Por quê?** Redes Neurais são rígidas quanto ao tamanho da entrada. Nosso modelo foi treinado especificamente com imagens de **180x180 pixels**. Esta linha redimensiona a imagem do usuário (que pode ser 4000x3000 ou 100x100) para exatamente `(180, 180)`, para que ela "caiba" na entrada do modelo.

3.  **Conversão para Array (`img_to_array`)**
    * **Por quê?** O TensorFlow não entende "imagens"; ele entende "tensores" (basicamente, matrizes de números). Esta função converte a imagem PIL de `(180, 180)` em um array NumPy com formato `(180, 180, 3)`, onde os números representam os valores de pixel (de 0 a 255) para cada canal de cor.

4.  **Expansão de Lote (Batch) (`expand_dims`)**
    * **Por quê?** O modelo foi projetado para prever em *lotes* de imagens, não apenas uma de cada vez. Sua entrada espera um formato de 4D: `(batch_size, height, width, channels)`. Esta função adiciona uma dimensão "falsa" no início, transformando nosso array `(180, 180, 3)` em `(1, 180, 180, 3)`, que o modelo entende como "um lote de 1 imagem".

### Validação de Imagem
O aplicativo também usa CV para validação de entrada na função `open_image_file`:

* **Correção de Orientação (`ImageOps.exif_transpose(img)`)**: Corrige fotos de celular que podem estar "deitadas" devido a dados EXIF.
* **Criação de Miniatura (`thumb.thumbnail(...)`)**: Cria uma miniatura de `(100, 100)` para a UI. Isso é crucial para a performance, pois evita tentar exibir uma imagem de 5000x5000 pixels na janela.
* **Tratamento de Erros (`UnidentifiedImageError`, `DecompressionBombError`)**: Protege o app contra arquivos corrompidos ou "pixel bombs" (imagens maliciosas projetadas para consumir toda a memória).

---

## 🚀 Como o Aplicativo Funciona (`app.py`)

O código do `app.py` integra a IA e a CV em uma aplicação de software robusta.

1.  **Constantes e Carregamento (Início)**
    * `MODEL_IMG_SIZE = (180, 180)`: A constante mais importante. Define o tamanho que o modelo espera.
    * `resource_path`: Uma função vital para o **PyInstaller**. Quando o app é compilado em um `.exe`, os arquivos (`.h5`, `.txt`) são extraídos para uma pasta temporária (`_MEIPASS`). Esta função garante que o app encontre seus arquivos, esteja ele rodando como `.py` ou `.exe`.
    * `tf.keras.models.load_model(MODEL_PATH)`: Carrega o "cérebro" (o modelo treinado) na memória.

2.  **Interface Gráfica (`App` class)**
    * Usa `customtkinter` para criar uma janela moderna.
    * O botão "Carregar Imagem" chama a função `open_image_file`.

3.  **Fluxo de Execução (`open_image_file`)**
    * O usuário seleciona um arquivo.
    * **Validação 1:** O tamanho do arquivo (`os.path.getsize`) é verificado contra `MAX_FILE_SIZE`.
    * **Validação 2:** O app tenta abrir a imagem (`Image.open`). Se falhar (ex: é um `.txt`), o `UnidentifiedImageError` é capturado.
    * A miniatura é criada e exibida.
    * A imagem completa é enviada para `predict_image`.

4.  **Fluxo de Predição (`predict_image`)**
    * A imagem passa pelo *pipeline* de pré-processamento de CV (RGB -> Resize -> Array -> Batch).
    * `model.predict()` é chamado.
    * Os *logits* são convertidos em probabilidades (`softmax`).
    * O resultado final (descrição e confiança) é formatado e retornado para a UI.

---

## 🛠️ Como Executar

### Pré-requisitos
Você precisará ter o Python 3.10+ e as seguintes bibliotecas instaladas:

```bash
pip install tensorflow customtkinter Pillow
