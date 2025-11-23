# Relatório do Projeto – Renderizador OBJ em OpenGL

**Aluno:** Rafael Manteiga Balbino<br>
**Matrícula:** 201920649111

## 1. Orientações para construir o executável e carregar os modelos

Para construir o executável:

1. Instalar dependências:
pip install -r requirements.txt

2. Gerar o executável com PyInstaller:
pyinstaller --onefile CG.py

3. O executável será criado na pasta dist/.

Para executar diretamente o código-fonte:
python3 CG.py

O carregamento do arquivo .obj ocorre automaticamente ao iniciar o programa, bastando ajustar o caminho no código se necessário.

## 2. Modelos utilizados com sucesso

Foram carregados corretamente três modelos 3D no formato .obj:

- Stanford Bunny  

Quando necessário, a conversão foi feita no Blender:
File → Export → Wavefront (.obj)

## 3. Recursos necessários

Bibliotecas utilizadas:
- numpy
- glfw
- PyOpenGL
- PyOpenGL_accelerate

Arquivos necessários:
- Arquivo .obj
- Drivers compatíveis com OpenGL 3.3+

## 4. Outras unformações

- Transformações 3D e câmera orbital utilizam matrizes NumPy.  
- GLFW gerencia janela, contexto OpenGL e eventos de entrada.  
- Shaders são compilados dinamicamente com PyOpenGL.  
- Requer hardware com suporte a OpenGL moderno.
