# Projeto 02 - Computação Visual

<br>

| Nome      | RA |
| ----------- | ----------- |
| Bruno Viana Tripoli Barbosa      | 10409547       |
| Felipe Gyotoku Koike   | 10409640        |
| Jonatas de Brito Silva   | 10403674        |
| Vitor Machado   | 10409358        |

<br>

# Analisador de Maturação de Bananas

Este projeto é uma aplicação de **Visão Computacional** que utiliza uma **Rede Neural Convolucional (CNN)** treinada com PyTorch para classificar o estágio de maturação de bananas. A aplicação possui uma interface gráfica (GUI) onde o usuário pode carregar uma imagem e receber a classificação do modelo em tempo real.

## 🌟 Funcionalidades

* **Interface Gráfica Simples:** Interface amigável construída com CustomTkinter para fácil upload de imagens.
* **Classificação em 4 Estágios:** O modelo classifica as bananas em quatro categorias:
    * `Class A`: Verde
    * `Class B`: Parcialmente Madura
    * `Class C`: Madura
    * `Class D`: Passada
* **Exibição de Confiança:** Mostra a porcentagem de confiança do modelo na classificação.
* **Validação de Arquivo:** Verifica o tamanho do arquivo (limite de 20MB) para evitar sobrecarga.
* **Modelo de Deep Learning:** Utiliza uma arquitetura ResNet18 treinada especificamente para esta tarefa.

## 🧠 Como Funciona: IA e Visão Computacional

Este projeto é um exemplo clássico de como a Inteligência Artificial e a Visão Computacional trabalham juntas.

1.  **Visão Computacional (CV):** É o campo que permite ao computador "enxergar" e interpretar imagens digitais. Neste projeto, a CV é usada para:
    * Ler o arquivo de imagem (JPG, PNG, etc.).
    * Pré-processar a imagem: redimensioná-la para o tamanho que o modelo espera (224x224 pixels) e convertê-la em um formato numérico (Tensor) que a IA possa entender.

2.  **Inteligência Artificial (IA):** É o "cérebro" que toma a decisão. Em vez de programar regras manualmente (ex: "se 50% da banana for amarela, está madura"), nós usamos **Deep Learning (Aprendizado Profundo)** para *treinar* um modelo a *aprender* esses padrões sozinho.
    * **Modelo:** Utilizamos uma **Rede Neural Convolucional (CNN)**, especificamente a arquitetura **ResNet18**.
    * **O que é uma CNN?** Uma CNN é um tipo de IA ideal para imagens. Ela funciona em camadas, aprendendo características de forma hierárquica:
        * *Camadas iniciais* aprendem características simples (bordas, cantos, cores).
        * *Camadas intermediárias* aprendem texturas e formas simples (manchas, curvas).
        * *Camadas finais* aprendem padrões complexos (a forma geral de uma banana, a distribuição das manchas, etc.) para tomar a decisão final.

3.  **Aprendizado por Transferência (Transfer Learning):** Este é o conceito de IA mais importante usado aqui.
    * Treinar uma CNN do zero exige milhões de imagens e um poder computacional imenso.
    * Para evitar isso, usamos **Aprendizado por Transferência**: pegamos o modelo ResNet18 que já foi **pré-treinado** pela Google no dataset "ImageNet" (um banco de dados gigante com 14 milhões de imagens de objetos comuns).
    * Esse modelo pré-treinado já é um "especialista" em identificar bordas, texturas e formas.
    * O notebook `train_model.ipynb` apenas **adapta** esse conhecimento prévio para a nossa tarefa específica (maturação de bananas), o que é muito mais rápido e eficiente.

## 📄 Análise dos Arquivos de Código

O projeto é dividido em duas partes principais: o treinamento do modelo (o "cérebro") e a aplicação gráfica (o "rosto").

### 1. `train_model.ipynb` (O Cérebro - Treinamento)

Este Jupyter Notebook é responsável por treinar o modelo de IA.

* **Objetivo:** Ensinar a arquitetura ResNet18 a diferenciar os 4 estágios de maturação.
* **Passo 1: Carga e Preparação dos Dados**
    * Ele usa `torchvision.datasets.ImageFolder` para carregar as imagens das pastas `dataset/train`, `dataset/validation` e `dataset/test`.
    * **Data Augmentation (Aumento de Dados):** Nas imagens de treino (`train_transform`), ele aplica transformações aleatórias como `RandomHorizontalFlip` (virar a imagem) e `RandomRotation` (girar um pouco). Isso cria "novas" imagens de treino e ajuda o modelo a generalizar melhor, evitando que ele decore apenas as imagens originais (overfitting).
    * Todas as imagens são redimensionadas para `(224, 224)` e convertidas para `Tensor`.

* **Passo 2: Treinamento (Aprendizado por Transferência)**
    * O notebook carrega a `resnet18` com os pesos pré-treinados do ImageNet (`ResNet18_Weights.DEFAULT`).
    * O treinamento ocorre em duas fases:
        1.  **Feature Extraction (Extração de Características):** Inicialmente, ele "congela" todas as camadas do ResNet18, exceto a última (`for param in model.parameters(): param.requires_grad = False`). Ele substitui a última camada (`model.fc`) por uma nova camada com 4 saídas (nossas 4 classes). Ele então treina *apenas* essa nova camada. Isso é rápido e ensina o modelo a mapear as características que ele já sabe (texturas, formas) para as nossas classes.
        2.  **Fine-Tuning (Ajuste Fino):** Após a primeira fase, o notebook "descongela" o modelo inteiro (`for param in model.parameters(): param.requires_grad = True`). Ele então continua o treinamento com uma taxa de aprendizado (`lr`) muito baixa. Isso permite que o modelo ajuste *levemente* todas as suas camadas para se especializar ainda mais na tarefa de identificar bananas.

* **Passo 3: Avaliação e Exportação**
    * Após o treinamento, o modelo é avaliado no conjunto de `test_loader` para verificar sua acurácia em dados nunca vistos.
    * Ele gera gráficos de perda (Loss) e acurácia (Accuracy) e uma **Matriz de Confusão** (que mostra quais classes o modelo mais confunde).
    * Finalmente, ele salva os pesos aprendidos (o `state_dict`) no arquivo `resnet18_model.pth`.

### 2. `app.py` (O Rosto - Aplicação Gráfica)

Este script Python é a aplicação final que o usuário executa.

* **Objetivo:** Fornecer uma interface gráfica para usar o modelo treinado.
* **Frameworks:** `CustomTkinter` (para a GUI), `PIL` (para manipulação de imagens) e `PyTorch` (para executar a inferência).

* **Passo 1: Carregamento do Modelo**
    * O script **recria** a arquitetura ResNet18 (`models.resnet18(weights=None)`). É crucial usar `weights=None` (sem pesos pré-treinados) porque vamos carregar *nossos próprios pesos* treinados.
    * Ele ajusta a camada final (`model.fc`) para ter 4 saídas (o `num_classes` baseado no `CLASS_NAMES`).
    * Ele então carrega os pesos salvos pelo notebook (o `state_dict`) usando `model.load_state_dict()`.
    * **IMPORTANTE:** O script está configurado para carregar `banana_classifier_model.pth`. Você deve renomear o arquivo `resnet18_model.pth` (saída do notebook) para este nome.
    * Ele coloca o modelo em **modo de avaliação** (`model.eval()`). Isso é essencial para desativar camadas como Dropout e Batch Normalization, garantindo resultados consistentes na inferência.

* **Passo 2: Pipeline de Inferência (`predict_image`)**
    * Esta função é o coração da aplicação.
    * Ela recebe a imagem carregada pelo usuário (objeto PIL).
    * Converte a imagem para 'RGB'.
    * Aplica as transformações de pré-processamento de *inferência* (apenas `Resize` e `ToTensor`, sem Data Augmentation).
    * Adiciona uma "dimensão de batch" (`unsqueeze(0)`) pois o PyTorch espera receber um lote de imagens, mesmo que seja só uma.
    * Executa a predição dentro de um bloco `with torch.no_grad()`, que desativa o cálculo de gradientes e torna a inferência muito mais rápida.
    * Aplica `softmax` na saída do modelo para convertê-la em probabilidades (%).
    * Encontra a classe com maior probabilidade (`torch.max`) e formata a mensagem de resultado.

* **Passo 3: Interface Gráfica (Classe `App`)**
    * A classe `App` (que herda de `ctk.CTk`) constrói a janela.
    * `open_image_file`: É chamada quando o botão "Carregar Imagem" é clicado. Ela abre a caixa de diálogo, valida o tamanho do arquivo, exibe um thumbnail (miniatura) da imagem na tela e chama `predict_image`.
    * `show_message`: Exibe o resultado da predição (ou mensagens de erro) no label `result_label`, usando cores diferentes para sucesso, aviso ou erro.

## 🚀 Estrutura do Projeto e Dependências

Para que o projeto funcione, a estrutura de pastas deve ser:<br>

<img width="293" height="545" alt="image" src="https://github.com/user-attachments/assets/2b55c5e4-50bd-4ead-ac1b-1ca7b4d1f1ab" />


### Dependências Principais

Você pode instalar as bibliotecas necessárias com o pip:

```bash
pip install torch torchvision
pip install customtkinter
pip install pillow
pip install numpy
pip install scikit-learn  # (Necessário para a matriz de confusão no notebook)

