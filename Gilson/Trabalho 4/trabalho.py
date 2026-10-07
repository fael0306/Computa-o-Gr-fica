import numpy as np
import matplotlib.pyplot as plt
from skimage import io


def reconstruir_placa(altura, largura, caminho_imagem):
    imagem = io.imread(caminho_imagem)

    if imagem.ndim == 2:
        imagem = np.dstack([imagem, imagem, imagem])

    altura_orig = imagem.shape[0]
    largura_orig = imagem.shape[1]

    plt.figure()
    plt.imshow(imagem)
    plt.title("Clique: inf-esq, inf-dir, sup-dir, sup-esq")
    plt.draw()

    pontos = plt.ginput(4, timeout=0)
    plt.close()

    p_ie = pontos[0]
    p_id = pontos[1]
    p_sd = pontos[2]
    p_se = pontos[3]

    novo_se = [0.0, 0.0]
    novo_sd = [largura - 1.0, 0.0]
    novo_id = [largura - 1.0, altura - 1.0]
    novo_ie = [0.0, altura - 1.0]

    pares = [
        (novo_se, p_se),
        (novo_sd, p_sd),
        (novo_id, p_id),
        (novo_ie, p_ie),
    ]

    A = np.zeros((8, 8))
    b = np.zeros(8)

    for i in range(4):
        u = pares[i][0][0]
        v = pares[i][0][1]
        x = pares[i][1][0]
        y = pares[i][1][1]

        A[2 * i, 0] = u
        A[2 * i, 1] = v
        A[2 * i, 2] = 1
        A[2 * i, 6] = -u * x
        A[2 * i, 7] = -v * x
        b[2 * i] = x

        A[2 * i + 1, 3] = u
        A[2 * i + 1, 4] = v
        A[2 * i + 1, 5] = 1
        A[2 * i + 1, 6] = -u * y
        A[2 * i + 1, 7] = -v * y
        b[2 * i + 1] = y

    h = np.linalg.solve(A, b)

    H = np.array([
        [h[0], h[1], h[2]],
        [h[3], h[4], h[5]],
        [h[6], h[7], 1.0],
    ])

    nova_imagem = np.zeros((altura, largura, imagem.shape[2]),
                           dtype=imagem.dtype)

    for j in range(altura):
        for i in range(largura):
            ponto = np.array([i, j, 1.0])
            ponto_transformado = H.dot(ponto)

            x_orig = ponto_transformado[0] / ponto_transformado[2]
            y_orig = ponto_transformado[1] / ponto_transformado[2]

            if (x_orig < 0 or x_orig > largura_orig - 1 or
                    y_orig < 0 or y_orig > altura_orig - 1):
                continue

            x0 = int(np.floor(x_orig))
            y0 = int(np.floor(y_orig))
            x1 = x0 + 1
            y1 = y0 + 1

            if x1 > largura_orig - 1:
                x1 = largura_orig - 1
            if y1 > altura_orig - 1:
                y1 = altura_orig - 1

            dx = x_orig - x0
            dy = y_orig - y0

            c00 = imagem[y0, x0].astype(float)
            c10 = imagem[y0, x1].astype(float)
            c01 = imagem[y1, x0].astype(float)
            c11 = imagem[y1, x1].astype(float)

            cima = c00 * (1 - dx) + c10 * dx
            baixo = c01 * (1 - dx) + c11 * dx
            cor = cima * (1 - dy) + baixo * dy

            if np.issubdtype(imagem.dtype, np.integer):
                info = np.iinfo(imagem.dtype)
                cor = np.clip(cor, info.min, info.max)

            nova_imagem[j, i] = cor.astype(imagem.dtype)

    plt.figure()
    plt.imshow(nova_imagem)
    plt.title("Placa reconstruida")
    plt.show()

    return nova_imagem


if __name__ == "__main__":
    reconstruir_placa(600, 600, "Placa_Pare_Marcas.png")
    # reconstruir_placa(800, 400, "Placa_Estacionamento_Marcas.png")