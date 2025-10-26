# 🍌 Analisador de Maturação de Banana


Notas: 


-- O Windows pode reconhecer o executável como um arquivo malicioso, mas basta clicar em executar mesmo assim e tá tudo certo. Vai ser executado<br>

-- O programa não acerta 100% das vezes, alguns equívocos acontecem quando uma imagem tem muito verde por exemplo, já que a imagem é analisada como um todo, o modelo prediz uma banana verde o que pode não ser uma verdade. <br>
-- Tivemos problema de compatibilidade com bibliotecas, o principal foi tentar utilizar o pyintaller com o python 3.13, tivemos que voltar para o 3.10.
-- O programa não valida se uma imagem contém ou não uma banana. O modelo foi treinado com imagens de bananas e digamos que para o "mundo" dele só existe bananas. <br>
-- O programa abrirá uma tela de terminal antes de ser executado.<br>
-- O programa demora um pouco para ser executado devido às biblitecas utilizadas como o tensorflow.<br>
-- Colocamos uma limitação de 20MB para o tamanho da imagem adicionada.<br>
-- Colocamos uma verificação de confiança da predição que requer 60% para que seja considerada uma predição válida.<br>


Este é um projeto de Deep Learning e Visão Computacional que utiliza uma **Rede Neural Convolucional (CNN)** para classificar o estágio de maturação de bananas.

O aplicativo de desktop (GUI) permite que um usuário faça o upload de uma imagem e receba uma classificação em tempo real, informando um dos quatro estágio de maturação:

* **Class A** - Verde
* **Class B** - Parcialmente Madura
* **Class C** - Madura
* **Class D** - Passada
<br>
LINK DE VÍDEO TESTE SEM AUDIO

https://drive.google.com/file/d/18caospEZf7bDh_pC_T_jo-B8VEgjV8I-/view?usp=sharing

## Conceitos de Inteligência Artificial utilizados

Este projeto é um exemplo clássico de um *pipeline* completo de IA: **treinamento** (no `train_model.py`) e **inferência** (no `app.py`).

### 1. Rede Neural Convolucional (CNN)
O coração deste projeto é uma CNN, um tipo de IA especializado em entender imagens. No `train_model.py`, nós construímos essa rede "do zero". Ela aprende a identificar padrões visuais (como cor, textura e manchas) associados a cada estágio de maturação.

### 2. Classificação de Imagem
A tarefa de IA sendo executada é a **classificação de imagem**. O modelo é treinado em milhares de imagens para "forçar" uma nova imagem em uma das 4 categorias (classes) que ele conhece.

### 3. Logits e Softmax
O modelo é treinado com a perda `loss=...SparseCategoricalCrossentropy(from_logits=True)`. Isso significa que a saída bruta do modelo (`predictions`) não são probabilidades, mas sim números brutos chamados **logits**.
No `app.py`, aplicamos `tf.nn.softmax(predictions[0])` para converter esses logits em um vetor de probabilidades (por exemplo: `[0.1, 0.05, 0.8, 0.05]`), onde cada número representa a "confiança" do modelo para uma classe, e a soma de todos é 1.0 (ou 100%).

### 4. Inferência e Predição
O processo no `app.py` é a **inferência**. Nós pegamos a probabilidade mais alta usando `np.argmax(score)` para determinar a classe vencedora e `np.max(score)` para obter o nível de confiança dessa previsão.

---

## Conceitos de Computação Visual utilizados

Antes que a IA possa "ver" a imagem, ela precisa ser formatada corretamente. Este é o trabalho da Computação Visual.

### No Treinamento (`train_model.py`)

1.  **Carregamento de Dados (`image_dataset_from_directory`)**: Esta função varre as subpastas (`Class A`, `Class B`, etc.) e automaticamente rotula as imagens com base no nome da pasta em que estão.
2.  **Redimensionamento (`image_size=(180, 180)`)**: Padroniza todas as imagens de treino para `180x180` pixels.
3.  **Normalização (`Rescaling(1./255)`)**: Converte os valores de pixel de [0-255] para [0.0-1.0]. Isso é crucial para que a rede neural aprenda de forma eficiente.
4.  **Aumento de Dados (Data Augmentation)**:
    * `RandomFlip`, `RandomRotation`, `RandomZoom`: Esta é uma técnica poderosa. A cada época de treinamento, o modelo "vê" versões ligeiramente diferentes das mesmas imagens (giradas, com zoom). Isso o ensina a ser mais robusto e evita que ele "decore" as imagens de treino (**overfitting**).

### Na Inferência (`app.py`)

Quando o usuário faz o upload de uma imagem, um *pipeline* de CV semelhante é aplicado para garantir que a imagem seja idêntica ao que o modelo espera:

1.  **Conversão de Espaço de Cor (`.convert('RGB')`)**: Garante que imagens em tons de cinza ou com transparência (PNG) sejam convertidas para o formato RGB de 3 canais que o modelo espera.
2.  **Redimensionamento (`.resize(MODEL_IMG_SIZE)`)**: Redimensiona a imagem do usuário (que pode ter qualquer tamanho) para exatamente `(180, 180)`.
3.  **Conversão para Array (`img_to_array`)**: Converte a imagem de um objeto PIL para uma matriz NumPy (`(180, 180, 3)`) que o TensorFlow entende.
4.  **Expansão de Lote (`expand_dims`)**: Transforma a matriz em `(1, 180, 180, 3)`, criando um "lote de 1 imagem", que é o formato de entrada que o modelo espera.

---

## Como o Projeto Funciona

O projeto é dividido em dois scripts principais: a "fábrica" (`train_model.py`) e o "produto final" (`app.py`).

### 1. `train_model.py` (A "Fábrica de Cérebros")

Este script é executado uma vez para criar o arquivo do modelo.

1.  **Carregamento:** Carrega as imagens das pastas `dataset/Real Dataset/train` e `dataset/Real Dataset/validation`.
2.  **Arquitetura (A Receita da CNN):**
    * **Blocos Convolucionais (Conv2D + MaxPooling2D):** O "sistema visual" do modelo. As camadas `Conv2D` atuam como filtros que aprendem a detectar características (bordas, curvas, cores, texturas). As camadas `MaxPooling2D` reduzem o tamanho da imagem, focando nas características mais importantes. A profundidade dos filtros aumenta (32 -> 64 -> 128 -> 256) para aprender padrões cada vez mais complexos.
    * **Flatten:** "Achata" os mapas de características 2D em um único vetor longo 1D.
    * **Camadas Densas (Dense):** O "cérebro" de classificação.
        * `Dense(512)`: Uma camada oculta que aprende a combinar as características visuais para tomar uma decisão.
        * `Dropout(0.5)`: Uma técnica de regularização que "desliga" aleatoriamente 50% dos neurônios durante o treino para evitar overfitting.
        * `Dense(num_classes)`: A camada de saída final, com um neurônio para cada classe.
3.  **Compilação:** Prepara o modelo para o treino, definindo:
    * `optimizer='adam'`: O algoritmo que ajusta os pesos do modelo.
    * `loss='SparseCategoricalCrossentropy(from_logits=True)'`: A função matemática que mede o quão "errada" está a previsão do modelo.
4.  **Treinamento (`model.fit`):**
    * O modelo "olha" para os lotes de imagens de `train_ds` repetidamente (10 `epochs`).
    * A cada época, ele se valida contra o `val_ds` (dados que nunca viu).
    * `EarlyStopping`: Um "vigia" inteligente. Ele monitora a perda de validação (`val_loss`). Se a performance parar de melhorar por 5 épocas, o treino é interrompido automaticamente, e os melhores pesos são restaurados (`restore_best_weights=True`).
5.  **Salvamento:**
    * `model.save('banana_classifier_model.h5')`: Salva o "cérebro" treinado (arquitetura + pesos) em um único arquivo.
    * `class_names.txt`: Salva a lista de nomes das classes na ordem correta (ex: `['Class A', 'Class B', ...]`).

### 2. `app.py` (O Aplicativo para o Usuário)

Este script usa os arquivos gerados pelo `train_model.py`.

1.  **Constantes e Carregamento (Início)**
    * `MODEL_IMG_SIZE = (180, 180)`: A constante mais importante. Garante que o app use o mesmo tamanho de imagem do treino.
    * `resource_path`: Uma função vital para o **PyInstaller**. Garante que o app encontre seus arquivos (`.h5`, `.txt`), esteja ele rodando como `.py` ou `.exe`.
    * `tf.keras.models.load_model(MODEL_PATH)`: Carrega o "cérebro" (`.h5`) na memória.
    * O app também carrega o `class_names.txt` para traduzir o índice da previsão (ex: `2`) para um nome legível (ex: `Class C`).

2.  **Interface Gráfica (`App` class)**
    * Usa `customtkinter` para criar uma janela moderna.
    * O botão "Carregar Imagem" chama a função `open_image_file`.

3.  **Fluxo de Execução (`open_image_file`)**
    * O usuário seleciona um arquivo.
    * **Validação 1 (Tamanho):** O tamanho do arquivo (`os.path.getsize`) é verificado contra `MAX_FILE_SIZE`.
    * **Validação 2 (Tipo):** O app tenta abrir a imagem (`Image.open`). Se falhar (ex: é um `.txt`), o `UnidentifiedImageError` é capturado.
    * A miniatura é criada e exibida.
    * A imagem completa é enviada para `predict_image`.

4.  **Fluxo de Predição (`predict_image`)**
    * A imagem passa pelo *pipeline* de pré-processamento de CV (RGB -> Resize -> Array -> Batch).
    * `model.predict()` é chamado.
    * Os *logits* são convertidos em probabilidades (`softmax`).
    * O resultado final (descrição e confiança) é formatado e retornado para a UI.

---

## Como fazer todo o processo de treinamento e geração do executável

### Passo 1: Configurar o Ambiente

1.  **Instale o Python**: Baixe e instale o **Python 3.10** ou **3.11** (versões estáveis para este projeto). Marque **"Add Python to PATH"** durante a instalação.
2.  **Crie a Pasta do Projeto**: Crie uma pasta (ex: `C:\Projetos\analyser`).
3.  **Abra o Prompt de Comando (CMD)** e navegue até sua pasta:
    ```bash
    cd C:\Projetos\analyser
    ```
4.  **Crie e Ative um Ambiente Virtual**:
    ```bash
    python -m venv venv
    venv\Scripts\activate
    ```
5.  **Instale as Bibliotecas**:
    ```bash
    pip install tensorflow customtkinter Pillow pyinstaller kagglehub
    ```

### Passo 2: Baixar o Dataset

1.  Na sua pasta (`analyser`), crie um arquivo chamado `download_data.py`.
2.  Cole o seguinte código nele:
    ```python
    import kagglehub
    import os
    import zipfile
    
    dataset_name = "luischuquimarca/banana-ripeness"
    extract_path = "dataset" # Irá criar a pasta 'dataset'
    
    print(f"Baixando o dataset '{dataset_name}'...")
    path = kagglehub.dataset_download(dataset_name)
    print(f"Dataset baixado para: {path}")
    
    if not os.path.exists(extract_path):
        os.makedirs(extract_path)
    
    print(f"Extraindo arquivos para a pasta '{extract_path}'...")
    with zipfile.ZipFile(path, 'r') as zip_ref:
        zip_ref.extractall(extract_path)
    
    print("Download e extração concluídos.")
    ```
3.  Execute o script **uma vez** (com o `(venv)` ativo):
    ```bash
    python download_data.py
    ```
    Ao final, você terá uma pasta `dataset` pronta para o treinamento.

### Passo 3: Treinar o Modelo

1.  Certifique-se de que os arquivos `train_model.py` e `app.py` disponibilizados neste repositório estão na sua pasta `analyser`.
2.  Execute o script de treinamento:
    ```bash
    python train_model.py
    ```
3.  Este script irá demorar alguns minutos. Ao final, você terá dois novos arquivos essenciais na sua pasta:
    * `banana_classifier_model.h5` (O "cérebro" treinado)
    * `class_names.txt` (As "etiquetas" que o cérebro aprendeu)

### Passo 4: Gerar o Executável (`.exe`)

1.  Execute o comando do **PyInstaller** no seu terminal (ainda com o `(venv)` ativo):

    ```bash
    pyinstaller --onefile --windowed --add-data "banana_classifier_model.h5;." --add-data "class_names.txt;." app.py
    ```

3.  O processo pode levar alguns minutos.

### Passo 5: Testar

1.  Após a conclusão, uma nova pasta `dist` será criada.
2.  Dentro dela, você encontrará o `app.exe`.
3.  Este é o programa final, só clicar e fazer os uploads das imagens de bananas.
