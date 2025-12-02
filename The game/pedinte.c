#include "pedinte.h"

// ============================================
// Função de dano + knockback + animação
// ============================================
void pedinteTomarDano(Pedinte* p, int direcaoHit) {

    if (p->morto) return;

    p->estado = INIMIGO_DANO;
    p->sofrendoDano = 1;    
    p->frameAtual = 0;
    p->ultimoFrame = 0;
    p->intervaloFrame = 80;

    // Direção da animação
    p->dir = (direcaoHit > 0) ? DIREITA : ESQUERDA;

    // Knockback
    p->velDano = (direcaoHit > 0) ? 10 : -10;

    // Vida
    p->vida--;
    if (p->vida <= 0)
        p->morto = 1;
}


// ============================================
// INIT
// ============================================
void initPedinte(Pedinte* p, int x, int y, int w, int h) {
    p->pos = (SDL_Rect){x, y, w, h};
    p->frameRect = (SDL_Rect){0, 0, 210, 180};
    p->dir = ESQUERDA;
    p->estado = INIMIGO_PARADO;

    p->ultimoFrame = 0;
    p->frameAtual = 0;
    p->intervaloFrame = 300;

    p->distanciaVisao = 400;
    p->limiteEsq = x - 150;
    p->limiteDir = x + 150;

    p->ativo = 1;
    p->yBase = y;

    p->atrito = 0.8f;
    p->velDano = 0;

    p->sofrendoDano = 0;   
    p->vida = 10;
    p->morto = 0;
}


// ============================================
// UPDATE
// ============================================
void updatePedinte(Pedinte* p, SDL_Rect playerRect, Uint32 agora) {

    // ===============================
    // ANIMAÇÃO DE DANO TEM PRIORIDADE
    // ===============================
    if (p->estado == INIMIGO_DANO) {

        // knockback
        p->pos.x += p->velDano;

        // atrito
        if (p->velDano > 0) {
            p->velDano -= p->atrito;
            if (p->velDano < 0) p->velDano = 0;
        } else {
            p->velDano += p->atrito;
            if (p->velDano > 0) p->velDano = 0;
        }

        
        if (agora - p->ultimoFrame > p->intervaloFrame) {
            p->ultimoFrame = agora;
            p->frameAtual++;

            if (p->frameAtual >= 6) {
                p->sofrendoDano = 0;   
                p->estado = INIMIGO_ANDANDO;
                p->frameAtual = 0;
            }
        }

        int linha = (p->dir == DIREITA) ? 4 : 3;

        p->frameRect = (SDL_Rect){
            210 * p->frameAtual,
            180 * linha,
            210, 180
        };

        return; // <-- impede qualquer outra ação
    }

    // ==========================================================
    // Lógica normal
    // ==========================================================

    float distancia = fabs((playerRect.x + playerRect.w/2) - (p->pos.x + p->pos.w/2));

    switch (p->estado) {
        case INIMIGO_PARADO:
            if (distancia < p->distanciaVisao) p->estado = INIMIGO_LEVANTANDO;
            break;

        case INIMIGO_LEVANTANDO:
            break;

        case INIMIGO_ANDANDO:
            if (distancia > p->distanciaVisao + 100)
                p->estado = INIMIGO_PATRULHANDO;
            break;

        case INIMIGO_PATRULHANDO:
            if (distancia < p->distanciaVisao)
                p->estado = INIMIGO_ANDANDO;
            break;
    }

    // ======================
    // LEVANTAR (linha 0)
    // ======================
    if (p->estado == INIMIGO_LEVANTANDO) {

        if (agora - p->ultimoFrame > p->intervaloFrame) {
            p->ultimoFrame = agora;

            if (p->frameAtual < 5)
                p->frameAtual++;
            else {
                p->estado = INIMIGO_ANDANDO;
                p->frameAtual = 0;
            }
        }

        p->frameRect = (SDL_Rect){
            210 * p->frameAtual,
            0,
            210, 180
        };
    }

    // ======================
    // ANDANDO (linha 1 ou 2)
    // ======================
    if (p->estado == INIMIGO_ANDANDO) {

        p->dir = (playerRect.x < p->pos.x) ? ESQUERDA : DIREITA;

        p->pos.x += (p->dir == DIREITA) ? 2 : -2;

        if (agora - p->ultimoFrame > p->intervaloFrame) {
            p->ultimoFrame = agora;
            p->frameAtual++;
            if (p->frameAtual > 6) p->frameAtual = 0;
        }

        int linha = (p->dir == DIREITA) ? 2 : 1;

        p->frameRect = (SDL_Rect){
            210 * p->frameAtual,
            175 * linha,
            210, 172
        };
    }

    // ========================
    // PATRULHA
    // ========================
    if (p->estado == INIMIGO_PATRULHANDO) {

        p->pos.x += (p->dir == DIREITA) ? 2 : -2;

        if (p->pos.x < p->limiteEsq) p->dir = DIREITA;
        if (p->pos.x + p->pos.w > p->limiteDir) p->dir = ESQUERDA;

        if (agora - p->ultimoFrame > p->intervaloFrame) {
            p->ultimoFrame = agora;
            p->frameAtual++;
            if (p->frameAtual > 6) p->frameAtual = 0;
        }

        int linha = (p->dir == DIREITA) ? 2 : 1;

        p->frameRect = (SDL_Rect){
            210 * p->frameAtual,
            175 * linha,
            210, 180
        };
    }

    // ======================
    // PARADO
    // ======================
    if (p->estado == INIMIGO_PARADO) {
        p->frameAtual = 0;
        p->frameRect = (SDL_Rect){0,0,210,180};
    }
}


// ============================================
// RENDER
// ============================================
void renderPedinte(SDL_Renderer* ren, SDL_Texture* tex, Pedinte* p, SDL_Rect camera) {

    if (p->morto) return;

    SDL_Rect dest = p->pos;
    dest.x -= camera.x;

    SDL_RenderCopy(ren, tex, &p->frameRect, &dest);
}
