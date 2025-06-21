
# 🎨 Editor de Voxel em C++ e OpenGL

Bem-vindo ao repositório do nosso Editor de Voxel 3D\! Este projeto foi desenvolvido como trabalho final para a disciplina de Computação Gráfica e consiste em uma aplicação interativa onde o usuário pode criar, destruir e modificar um mundo composto por blocos, ou voxels. A aplicação foi construída do zero utilizando C++ e a API gráfica moderna OpenGL.

## ✨ Funcionalidades

O editor conta com um conjunto robusto de funcionalidades, divididas em diferentes modos de interação:

#### **Gerais**

  * **Navegação 3D:** Câmera livre no estilo "fly-through", controlada por mouse e teclado (WASD).
  * **Menu de Pausa:** Um menu acessível pela tecla `ESC` com opções de **Salvar**, **Carregar** e **Sair**, com suporte para navegação por mouse e teclado.
  * **Gerenciamento de Arquivos:** Capacidade de salvar o estado atual do mundo voxel em um arquivo de texto e carregá-lo posteriormente.
  * **Grade Dinâmica:** O tamanho do mundo pode ser alternado dinamicamente entre 101x101, 32x32 e 16x16 com a tecla `F3`.

#### **Modo de Edição por Teclado (Padrão)**

  * **Seleção de Bloco:** Um seletor visual em forma de "brilho" pode ser movido pela grade usando as **Setas Direcionais** e **Page Up/Down**.
  * **Criação e Deleção:** Teclas `V` e `Delete` tornam o bloco selecionado visível ou invisível.
  * **HUD de Cores:** Uma interface gráfica simples (HUD) exibe a paleta de cores e permite a troca de cor do bloco selecionado com a tecla `C`.

#### **Modo de Desenho por Mouse**

  * **Alternância de Modo:** Pressione a tecla `M` para entrar e sair do "Modo de Desenho".
  * **Seleção por Raycasting:** O seletor de bloco segue o cursor do mouse, calculando a posição no mundo 3D através de um raio disparado da câmera.
  * **Desenho e Deleção Intuitivos:**
      * **Clique Esquerdo:** "Pinta" um voxel com a cor de desenho atual.
      * **Clique Direito:** Apaga o voxel que está sendo mirado.
  * **Seleção de Cor para Desenho:** As teclas numéricas de **1 a 6** definem a cor a ser utilizada para pintar.

## 🚀 Tecnologias Utilizadas

  * **Linguagem:** C++
  * **API Gráfica:** OpenGL 4.5
  * **Bibliotecas:**
      * **GLFW:** Para gerenciamento de janelas, contextos OpenGL e inputs (teclado/mouse).
      * **GLAD:** Para carregar os ponteiros de funções do OpenGL.
      * **GLM (OpenGL Mathematics):** Para todas as operações matemáticas com vetores e matrizes (transformações, projeção, etc.).

## ⚙️ Como Compilar e Executar

Para compilar este projeto, você precisará de um compilador C++ moderno e do CMake.

### Pré-requisitos

  * CMake (versão 3.10 ou superior)
  * Um compilador C++ (GCC/g++, Clang, MSVC)
  * As bibliotecas de desenvolvimento do **GLFW**.

### Passos para Compilação

Recomendamos o uso do CMake para gerar os arquivos de build para a sua plataforma.


1.  **Configure as dependências:**
    Certifique-se de que o CMake consiga encontrar as bibliotecas `GLAD`, `GLFW` e `GLM`. A forma mais fácil é colocá-las em uma pasta `libs` dentro do projeto ou usar um gerenciador de pacotes como o `vcpkg`.

2.  **Execute o CMake para gerar o build:**

    ```bash
    mkdir build
    cd build
    cmake ..
    ```

3.  **Compile o projeto:**

    ```bash
    cmake --build .
    ```

    Ou, em sistemas baseados em Make:

    ```bash
    make
    ```

4.  **Execute:**
    O arquivo executável será gerado dentro da pasta `build`.

## 🎮 Como Usar

| Ação                      | Controle                       | Modo                   |
| ------------------------- | ------------------------------ | ---------------------- |
| **Movimentar Câmera** | `W`, `A`, `S`, `D`             | Padrão                 |
| **Olhar ao Redor** | `Mouse`                        | Padrão                 |
| **Zoom** | `Scroll do Mouse`              | Padrão                 |
| **Abrir/Fechar Menu** | `ESC`                          | Todos                  |
| **Salvar Mundo** | `F1` ou Menu                   | Padrão                 |
| **Carregar Mundo** | `F2` ou Menu                   | Padrão                 |
| **Alternar Tamanho da Grade** | `F3`                         | Padrão / Desenho       |
| **Mover Seletor (Teclado)** | `Setas` + `Page Up/Down`     | Padrão                 |
| **Ativar Voxel (Teclado)** | `V`                            | Padrão                 |
| **Apagar Voxel (Teclado)** | `Delete`                       | Padrão                 |
| **Trocar Cor (Teclado)** | `C`                            | Padrão                 |
| **Entrar/Sair do Modo Desenho** | `M`                        | Padrão / Desenho       |
| **Escolher Cor de Desenho** | Teclas `1` a `6`             | Desenho                |
| **Desenhar Voxel (Mouse)** | `Clique Esquerdo`              | Desenho                |
| **Apagar Voxel (Mouse)** | `Clique Direito`               | Desenho                |

## 🏗️ Estrutura do Código

O projeto foi desenvolvido com uma abordagem procedural, focando na clareza e na separação de responsabilidades entre as funções.

  * **Gerenciamento de Voxels (`Voxel*** grid`):** O mundo é representado por uma matriz 3D alocada dinamicamente. Cada elemento é uma `struct Voxel` que armazena sua posição, visibilidade, cor e estado de seleção. A inserção e deleção são feitas apenas alterando a flag `visivel`, o que é extremamente performático.
  * **Renderização Eficiente:** Em vez de armazenar a geometria de milhões de cubos, definimos um **único modelo de cubo** em um VBO/VAO. A cada quadro, percorremos nossa grade 3D e, para cada voxel visível, enviamos suas informações de transformação (`model`) e cor (`uColor`) como *uniforms* para os shaders, desenhando o modelo repetidamente.
  * **Gerenciamento de Estado:** Variáveis globais como `menuAberto` e `modoDesenho` controlam o estado atual da aplicação, garantindo que os inputs de teclado e mouse se comportem de maneira diferente em cada contexto (navegação, menu, desenho).
  * **Raycasting:** A seleção por mouse no modo de desenho é feita com uma função de *raycasting* que converte as coordenadas 2D do mouse para um raio 3D no espaço do mundo. Uma simples "marcha de raio" (Ray Marching) avança ao longo desse raio para detectar qual voxel é interceptado.

## 👥 Autores

  * **[Gabriel da Silva Pereira]**
  * **[Raphael Frey Machado]**

