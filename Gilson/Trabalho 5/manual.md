# Manual de Instruções — Formiga Robô (OpenGL)

## 1. Descrição

Este programa implementa uma formiga robô em OpenGL, com cabeça (olhos e antenas), tórax, abdômen e seis pernas. As antenas, a cabeça e as pernas se movimentam de forma independente. As pernas simulam uma caminhada (marcha de tripé). O usuário interage por teclado e mouse, podendo também controlar a câmera (zoom e pan). As partes do robô possuem textura aplicada a partir de uma imagem BMP.

## 2. Como compilar e executar

**Dependências:**

- OpenGL, GLUT (ou freeglut) e GLU
- Biblioteca `RgbImage` (fornecida)
- Arquivo de textura `.bmp` no mesmo diretório do executável

**Compilação (exemplo em Windows / MinGW):**

```bash
g++ formiga.cpp RgbImage.cpp -o formiga.exe -lfreeglut -lopengl32 -lglu32
```

Execução:

```bash
formiga.exe
```

3. Controles

3.1 Teclado

Tecla Ação
W Inclina a câmera para cima
S Inclina a câmera para baixo
A Gira a câmera para a esquerda
D Gira a câmera para a direita
+ Zoom in (aproxima a câmera)
- Zoom out (afasta a câmera)
1 Gira a cabeça para um lado
2 Gira a cabeça para o outro lado
3 Move a antena esquerda para um lado
4 Move a antena esquerda para o outro lado
5 Move a antena direita para um lado
6 Move a antena direita para o outro lado
T Liga / desliga a textura
ESC Encerra o programa

3.2 Mouse

Ação Efeito
Arrastar com botão esquerdo Orbita a câmera em torno do robô
Arrastar com botão direito Pan (desloca o alvo da câmera)

3.3 Teclas especiais

Tecla Efeito
← / → Pan horizontal
↑ / ↓ Pan vertical
Page Up / Page Down Pan vertical (aproxima/afasta alvo)

4. Movimentos automáticos

· Pernas: caminhada simulada com marcha de tripé (pernas 0‑2‑4 em uma fase; 1‑3‑5 na fase oposta), incluindo balanço e leve elevação no avanço.
· Antenas: oscilação senoidal contínua, de forma independente entre si.
· Cabeça: leve rotação automática contínua, somada ao controle manual do usuário.

5. Observações

· Caso o arquivo de textura não seja encontrado, o programa exibe um aviso e desenha o robô sem textura, sem interromper a execução.
· A cena é renderizada com projeção perspectiva e Viewport ajustada ao tamanho da janela.