# Formiga Robô — OpenGL

Projeto desenvolvido em C++ utilizando OpenGL para a disciplina de Computação Gráfica.

A aplicação apresenta uma formiga robótica tridimensional articulada, com animação de caminhada, movimentação independente de componentes, iluminação, texturas e controle de câmera.

## Download

A versão pronta para Windows x64 está disponível na seção **Releases** do repositório.

Baixe:

`FormigaRobo_Entrega.zip`

Extraia todos os arquivos e execute:

`FormigaRobo.exe`

> É necessário manter `FormigaRobo.exe`, `freeglut.dll` e as texturas BMP na mesma pasta.

## Funcionalidades

- Cabeça articulada
- Duas antenas articuladas
- Seis pernas articuladas
- Caminhada automática
- Controle individual das pernas
- Controle independente da cabeça e antenas
- Rotação da formiga
- Zoom e pan da câmera
- Órbita da câmera
- Iluminação OpenGL
- Texturas BMP
- Projeção perspectiva

## Controles

| Tecla | Ação |
|---|---|
| W / Z | Órbita vertical da câmera |
| A / S | Órbita horizontal da câmera |
| + / - | Zoom |
| Setas | Pan da câmera |
| X | Reset da câmera |
| G / D | Girar a formiga |
| P | Iniciar/pausar caminhada |
| T | Ligar/desligar texturas |
| L | Ligar/desligar iluminação |
| Q / E | Movimentar cabeça |
| 1 / 2 | Antena esquerda — articulação base |
| 3 / 4 | Antena esquerda — articulação distal |
| 5 / 6 | Antena direita — articulação base |
| 7 / 8 | Antena direita — articulação distal |
| C | Selecionar perna |
| R / F | Articulação superior da perna |
| Y / H | Articulação intermediária |
| U / J | Articulação inferior |
| 0 | Reset das pernas |

## Compilação

### Ambiente utilizado

- Windows x64
- Visual Studio 2022
- C++
- OpenGL
- GLU
- FreeGLUT
- RgbImage

### FreeGLUT com vcpkg

```powershell
vcpkg install freeglut:x64-windows
vcpkg integrate install
