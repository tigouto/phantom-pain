#include "pedinte.h"

void initPedinte(Pedinte* p, int x, int y, int w, int h) {
    p->pos = (SDL_Rect){x, y, w, h};
    p->frameRect = (SDL_Rect){0, 0, 210, 170};
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
}

void updatePedinte(Pedinte* p, SDL_Rect playerRect, Uint32 agora) {
    float distancia = fabs((playerRect.x + playerRect.w/2) - (p->pos.x + p->pos.w/2));
    switch (p->estado) {
        case INIMIGO_PARADO:
            if (distancia < p->distanciaVisao) p->estado = INIMIGO_LEVANTANDO;
            break;
        case INIMIGO_LEVANTANDO:
            break;
        case INIMIGO_ANDANDO:
            if (distancia > p->distanciaVisao + 100) p->estado = INIMIGO_PATRULHANDO;
            break;
        case INIMIGO_PATRULHANDO:
            if (distancia < p->distanciaVisao) p->estado = INIMIGO_ANDANDO;
            break;
    }

    if (p->estado == INIMIGO_LEVANTANDO) {
        if (agora - p->ultimoFrame > p->intervaloFrame) {
            p->ultimoFrame = agora;
            if (p->frameAtual < 5) p->frameAtual++;
            else p->estado = INIMIGO_ANDANDO;
        }
        p->frameRect = (SDL_Rect){210 * p->frameAtual, 0, 210, 170};
    } if (p->estado == INIMIGO_ANDANDO) {
        p->dir = (playerRect.x < p->pos.x) ? ESQUERDA : DIREITA;
        p->pos.x += (p->dir == DIREITA) ? 2 : -2;
        if(p->ativo){
        	p->pos.y = p->yBase;
		}

        if (agora - p->ultimoFrame > p->intervaloFrame) {
            p->ultimoFrame = agora;
            p->frameAtual++;
            if (p->frameAtual > 6) p->frameAtual = 0;
        }
        int linha = (p->dir == DIREITA) ? 2 : 1;
        p->frameRect = (SDL_Rect){210 * p->frameAtual, 175 * linha, 210, 172};
    } if (p->estado == INIMIGO_PATRULHANDO) {
        p->pos.x += (p->dir == DIREITA) ? 2 : -2;
        if (p->pos.x < p->limiteEsq) p->dir = DIREITA;
        if (p->pos.x + p->pos.w > p->limiteDir) p->dir = ESQUERDA;

        if (agora - p->ultimoFrame > p->intervaloFrame) {
            p->ultimoFrame = agora;
            p->frameAtual++;
            if (p->frameAtual > 6) p->frameAtual = 0;
        }
        int linha = (p->dir == DIREITA) ? 2 : 1;
        p->frameRect = (SDL_Rect){210 * p->frameAtual, 170 * linha, 210, 170};
    } if (p->estado == INIMIGO_PARADO) {
        p->frameAtual = 0;
        p->frameRect = (SDL_Rect){0,0,210,170};
    }
}

void renderPedinte(SDL_Renderer* ren, SDL_Texture* tex, Pedinte* p, SDL_Rect camera) {
    SDL_Rect dest = p->pos;
    if(p->ativo){
        	p->pos.y = p->yBase;
		}
    dest.x -= camera.x;
    SDL_RenderCopy(ren, tex, &p->frameRect, &dest);
}
