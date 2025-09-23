# Projeto 01 - Computação Visual

### Nome - RA

Bruno Viana Tripoli Barbosa   - 10409547<br>
Felipe Gyotoku Koike          - 10409640<br>
Jonatas de Brito Silva        - 10409637<br> 
Vitor Machado                 - 10409358<br> 


Olá professor.



### Como rodar o nosso projeto?

  O projeto segue a mesma linha dos exercícios feitos no EAD, com diferença da pasta onde as bibliotecas SDL foram instaladas, basta clonar o projeto na máquina e substituir os caminhos das mesmas.

  Para maior praticidade segue o comando abaixo:

  ```git
  git clone https://github.com/felipe-koike/Computacao-Visual---07N.git
  ```


###  1. Carregamento de imagem
  Utilizamos na linha 165 a função ```void load_rgba32()``` para o carregamento da imagem "g_image", dentro dessa função, na linha 172, a função ```IMG_Load()``` realiza a verificação de tipos de arquivos que o programa pode realizar a leitura e a existência do arquivo.<br>
  A função ```load_rgba32()``` também cria uma cópia da imagem "g_image_two" para ser poder reverter a equalização da imagem (item 5).

###  2. Análise e conversão para escala de cinza

  Uma verificação prévia é feita para saber se a imagem está em escala de cinza, caso não esteja, a função ```to_gray_scale()``` é chamada.

  ```c
  if(verify_gray_scale(g_window.renderer, &g_image) == 0) to_gray_scale(g_window.renderer, &g_image);
  ```
    
  A função ```to_gray_scale()``` faz a verificação da imagem e a converte para tons de cinza caso seja colorida.<br>
  <br><img width="377" height="84" alt="image" src="https://github.com/user-attachments/assets/a2217f71-e7b1-4db5-9dce-5ac42dd3bba7" /><br><br>
  A função ```to_gray_scale()``` usa um laço for para transformar os valores de R, G e B em Y, seguindo a fórmula Y = 0.2125 ∗ 𝑅 + 0.7154 ∗ 𝐺 + 0.0721 ∗ 𝐵.<br>

###  3. Interface gráfica de usuário (GUI) com duas janelas
  Dentro da main chamamos a função ```initialize()``` na linha 259, que executa a função ```MyWindow_initialize()``` e devolve se a janela principal ou secundária foi criada. A função ```SDL_CreateWindowAndRenderer()``` na linha 143 é a que cria as janelas.<br>
  Para deixar a janela do tamanho da imagem, passamos como parâmetros a altura e largura da imagem, na função SDL_SetWindowSize() da linha 647.<br>
  E para posicionar no centro, utilizamos a função SDL_SetWindowPosition() com as tags ```SDL_WINDOWPOS_CENTERED``` nos vetores X e Y.<br>
  Para a janela secundária, criamos as constantes: ```DEFAULT_H_WINDOW_WIDTH``` e ```DEFAULT_H_WINDOW_HEIGHT``` para definir o tamanho.<br>
  Definimos ```h_win_x``` e ```h_win_y``` para a janela secundária ficar ao lado direito da janela principal com uma margem da tela.

###  4. Análise e exibição do histograma
  Para calcular o histograma utilizamos a função calculate_histogram() na linha 526, onde temos o vetor histograma de tamanho 256, onde cada índice corresponde a uma intensidade, utilizamos um ciclo for para calcular a intensidade de cada
  pixel da imagem e incrementar o valor da sua respectiva posição do vetor. Após isso chamamos a função render() na linha 357 que renderiza a imagem na janela principal, o histograma, as informações juntamente com o botão na janela secundária.<br>
  Dentro da render() também temos as funções ```calculate_average_intensity()```, ```calculate_standard_deviation()```, ```classify_intensity_string()``` e ```classify_deviation_string()``` que realizam os cálculos da média da intensidade, do desvio padrão,
  classicam a intensidade e classicam o desvio padrão, respectivamente.

###  5. Equalização do histograma
  Para a equalização das imagens criamos a função ```calculate_equilize_vector()``` na linha 543 que realiza o cálculo da equalização e salva os novos valores de intensidade no vetor ```histogram_equalized[]```.<br>
  Ao apertar o botão, o programa verifica se a imagem está equalizada ou não. Se ela não estiver, o programa chama a função ```apply_equalization()``` que utiliza um ciclo for que verifica a intensidade
  de cada pixel e subtsitui pelo seu valor de intensidade pelo valor no índice correspondente do vetor ```histogram_equalized[]```.<br>
  Se estiver equalizado, o programa chama a função restore_original_image() que transforma a imagem equalizada "g_image" pelo backup "g_image_two".<br>
  Dentro da função ```loop()``` na linha 414, o programa realiza a verificação que chama a função render() para a atualização dos valores das janelas (imagem, histograma, botão, textos e cor do botão).

###  6. Salvar imagem
  Para salvar a imagem criamos a função ```save_image_as_png()``` na linha 573, quando o usuário aperta a tecla S utiliza a função ```IMG_SavePNG()``` para realizar o salvamento. Sendo que a verificação se o botão S foi apertado está dentro da função ```loop()```.<br>


-------------------------------------------------------------
OBS: Professor Kishimoto, todos os integrantes do grupo participaram, tivemos reuniões no Discord nos dias 14/09/2025, 18/09/2025, 19/09/2025 e 21/09/2025 para discutir e realizar o projeto juntos.<br>
Acabamos subindo uma único commit com o código praticamente feito, seguido de algumas atualizações, sem se atentar ao ponto "7. Repositório e documentação" da lista dos critérios de avaliação. Pedimos perdão pelo descuido e agradeceriamos se essa falta de atenção não impactasse na nossa nota.<br> 
Podemos mostrar nossas conversas do WhatsApp e Discord para evidênciar toda nossa colaboração e versões do projeto.
