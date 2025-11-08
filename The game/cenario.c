#include "cenario.h"

void initCenario(Cenario* c) {
    c->fundo = NULL;
    c->numParalax = 0;
    c->numAtras = 0;
    c->numFrente = 0;
    c->posX = 0;
    c->largura = 4000;
    c->altura = 1080;
    c->numPedintes = 0;
}

void addParalax(Cenario* c, SDL_Texture* tex, float fator, int posY) {
    if (c->numParalax < MAX_PARALAX) c->paralax[c->numParalax++] = (Paralax){tex, fator, posY};
}

void addAtras(Cenario* c, SDL_Texture* tex, SDL_Rect pos) {
    if (c->numAtras < MAX_ELEMENTOS) c->atras[c->numAtras++] = (Elemento){tex, pos};
}

void addFrente(Cenario* c, SDL_Texture* tex, SDL_Rect pos) {
    if (c->numFrente < MAX_ELEMENTOS) c->frente[c->numFrente++] = (Elemento){tex, pos};
}

void addPedinte(Cenario* c, int x, int y, int w, int h) {
    if (c->numPedintes < MAX_PEDINTES) {
        initPedinte(&c->pedintes[c->numPedintes++], x, y, w, h);
    }
}

void addCheckpointElemento(Cenario* c, SDL_Texture* tex, SDL_Rect pos, int numFrames, int larguraFrame, int alturaFrame, float tempoFrame) {
    if (c->numElementos < MAX_ELEMENTOS) {
        ElementoCenario e;
        e.tipo = TIPO_ELEMENTO_CHECKPOINT;
        e.checkpoint = (Checkpoint){
            .tex = tex,
            .pos = pos,
            .ativado = 0,
            .frameAtual = 0,
            .numFrames = numFrames,
            .larguraFrame = larguraFrame,
            .alturaFrame = alturaFrame,
            .tempoFrame = tempoFrame,
            .timer = 0.0f
        };
        c->elementos[c->numElementos++] = e;
    }
}

void addAcampamentoElemento(Cenario* c, SDL_Texture* tex, SDL_Rect pos, int numFrames, int larguraFrame, int alturaFrame) {
    if (c->numElementos < MAX_ELEMENTOS) {
        ElementoCenario e;
        e.tipo = TIPO_ELEMENTO_ACAMPAMENTO;
        e.acampamento = (Acampamento){
            .tex = tex,
            .pos = pos,
            .ativado = 0,
            .frameAtual = 0,
            .numFrames = numFrames,
            .larguraFrame = larguraFrame,
            .alturaFrame = alturaFrame,
            .interagindo = 0
        };
        c->elementos[c->numElementos++] = e;
    }
}

void desenharCenario(SDL_Renderer* ren, Cenario* c, SDL_Rect camera, int screenW, int screenH,int w,int h) {
    if (!c) return;

    // Fundo (posicionado da mesma forma do seu código original)
    if (c->fundo) {
        SDL_Rect fundoR = { (screenW - 2990) / 2, ((screenH - 1495) / 2) + (5 * screenH) / 100, 2990, 1495 };
        SDL_RenderCopy(ren, c->fundo, NULL, &fundoR);
    }

    // Paralaxes
    for (int i = 0; i < c->numParalax; ++i) {
        desenharParalax(ren, c->paralax[i].tex, c->paralax[i].fator, camera.x - c->posX, c->paralax[i].posY, screenW,w,h);
    }

    // Elementos atrás
    for (int i = 0; i < c->numAtras; ++i) {
        SDL_Rect dest = c->atras[i].pos;
        dest.x -= (camera.x - c->posX);
        SDL_RenderCopy(ren, c->atras[i].tex, NULL, &dest);
    }
}

void desenharParalax(SDL_Renderer* ren, SDL_Texture* tex, float fatorParalax, int cameraX, int posY, int screenW,int w,int h) {
    if (!tex) return;
    int texW, texH;
    SDL_QueryTexture(tex, NULL, NULL, &texW, &texH);
    if (texW <= 0) return;

  
    int dstW = w;
    int dstH = h;
    if (dstW <= 0) dstW = texW;

    // offset calculado pelo fator
    int offsetX = -(int)(cameraX / fatorParalax) % dstW;
    if (offsetX > 0) offsetX -= dstW;

    for (int x = offsetX; x < screenW; x += dstW) {
        SDL_Rect dest = { x, posY, dstW, dstH };
        SDL_RenderCopy(ren, tex, NULL, &dest);
    }
}

void desenharFrente(SDL_Renderer* ren, Cenario* c, SDL_Rect camera) {
    for (int i = 0; i < c->numFrente; ++i) {
        SDL_Rect dest = c->frente[i].pos;
        dest.x -= (camera.x - c->posX);
        SDL_RenderCopy(ren, c->frente[i].tex, NULL, &dest);
    }
}

void fade_out_in(SDL_Renderer* ren, int screenW, int screenH, int fadeOut) {
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    SDL_Rect r = {0, 0, screenW, screenH};
    if (fadeOut) {
        for (int a = 0; a <= 255; a += 12) {
            SDL_SetRenderDrawColor(ren, 0,0,0,a);
            SDL_RenderFillRect(ren, &r);
            SDL_RenderPresent(ren);
            SDL_Delay(8);
        }
    } else {
        for (int a = 255; a >= 0; a -= 12) {
            SDL_SetRenderDrawColor(ren, 0,0,0,a);
            SDL_RenderFillRect(ren, &r);
            SDL_RenderPresent(ren);
            SDL_Delay(8);
        }
    }
}

void updateCheckpoint(Checkpoint* cp, SDL_Rect player, float deltaTime) {
    // Atualiza estado de ativação
    if (!cp->ativado && SDL_HasIntersection(&player, &cp->pos)) {
        cp->ativado = 1;
        printf("Checkpoint ativado!\n");
    }

    // Atualiza animação (loop contínuo)
    cp->timer += deltaTime;
    if (cp->timer >= cp->tempoFrame) {
        cp->timer = 0;
        cp->frameAtual++;
        if (cp->frameAtual >= cp->numFrames)
            cp->frameAtual = 0;
    }
}

void renderCheckpoint(SDL_Renderer* ren, Checkpoint* cp, SDL_Rect camera) {
    SDL_Rect src = {
        cp->frameAtual * cp->larguraFrame,
        0,
        cp->larguraFrame,
        cp->alturaFrame
    };

    SDL_Rect dest = cp->pos;
    dest.x -= camera.x;
    
	// Fazer alguma coisa quando ativar o checkpoint
	
    SDL_RenderCopy(ren, cp->tex, &src, &dest);
}

void updateAcampamento(Acampamento* ac, SDL_Rect player){
	if(!ac->ativado && SDL_HasIntersection(&player, &ac->pos)){
		ac->ativado = 1;
		ac->frameAtual++;
	}
	if(ac->ativado && !SDL_HasIntersection(&player, &ac->pos)){
		ac->ativado = 0;
		ac->frameAtual = 0;
	}
}

void renderAcampamento(SDL_Renderer* ren, Acampamento* ac, SDL_Rect camera) {
    SDL_Rect src = {
        ac->frameAtual * ac->larguraFrame,
        0,
        ac->larguraFrame,
        ac->alturaFrame
    };
    
	SDL_Rect dest = ac->pos;
    dest.x -= camera.x;
    SDL_RenderCopy(ren, ac->tex, &src, &dest);
}

void updateElementos(Cenario* c, SDL_Rect player, const Uint8* keys, float deltaTime, int* vidas) {
    for (int i = 0; i < c->numElementos; i++) {
        ElementoCenario* e = &c->elementos[i];
        switch (e->tipo) {
            case TIPO_ELEMENTO_CHECKPOINT:
                updateCheckpoint(&e->checkpoint, player, deltaTime);
                break;
            case TIPO_ELEMENTO_ACAMPAMENTO:
            	if(SDL_HasIntersection(&player, &e->acampamento.pos)){
            		e->acampamento.frameAtual = 1;
				}
				if(!SDL_HasIntersection(&player, &e->acampamento.pos)){
            		e->acampamento.frameAtual = 0;
				}
            	
                if (SDL_HasIntersection(&player, &e->acampamento.pos) && keys[SDL_SCANCODE_C]) {
                    e->acampamento.interagindo = 1;
                    *vidas = 3;
                } else {
                    e->acampamento.interagindo = 0;
                }
                break;
            default:
                break;
        }
    }
}

void renderElementos(SDL_Renderer* ren, Cenario* c, SDL_Rect camera) {
    for (int i = 0; i < c->numElementos; i++) {
        ElementoCenario* e = &c->elementos[i];
        switch (e->tipo) {
            case TIPO_ELEMENTO_CHECKPOINT:
                renderCheckpoint(ren, &e->checkpoint, camera);
                break;
            case TIPO_ELEMENTO_ACAMPAMENTO:
                renderAcampamento(ren, &e->acampamento, camera);
                break;
            default: break;
        }
    }
}
