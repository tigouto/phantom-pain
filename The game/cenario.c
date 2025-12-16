#include "cenario.h"
#include <SDL2/SDL_ttf.h>

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

void addMesaElemento(Cenario* c, SDL_Texture* tex, SDL_Rect pos, int numFrames, int larguraFrame, int alturaFrame){
	if (c->numElementos < MAX_ELEMENTOS) {
        ElementoCenario e;
        e.tipo = TIPO_ELEMENTO_MESA;
        e.mesa = (Mesa){
            .tex = tex,
            .pos = pos,
            .frameAtual = 0,
            .numFrames = numFrames,
            .larguraFrame = larguraFrame,
            .alturaFrame = alturaFrame,
            .interagindo = 0
        };
        c->elementos[c->numElementos++] = e;
    }
}

void addDialogoElemento(Cenario* c, SDL_Texture* tex, SDL_Rect pos, const char* texto) {
    if (c->numElementos < MAX_ELEMENTOS) {
        ElementoCenario e;
        e.tipo = TIPO_ELEMENTO_DIALOGO;
        e.dialogo = (Dialogo){
            .tex = tex,
            .pos = pos,
            .frameAtual = 0,
            .linhaAtual = 0,
            .numFrames = 0,
            .larguraFrame = 785,
            .alturaFrame = 335,
            .ultimoFrame = 0,
            .intervaloFrame = 120,
            .ativado = 0,
            .texto = texto
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

void animarDialogo(Dialogo* dl){
    if (!dl->ativado) return;

    Uint32 agora = SDL_GetTicks();

    // Atualiza src
    dl->src = (SDL_Rect){
        dl->frameAtual * dl->larguraFrame,
        dl->linhaAtual * dl->alturaFrame,
        dl->larguraFrame,
        dl->alturaFrame
    };

    // Controle de tempo
    if (agora - dl->ultimoFrame < dl->intervaloFrame)
        return;

    dl->ultimoFrame = agora;

    // Se já está no último frame da última linha → NÃO anima mais
    if (dl->linhaAtual == 1 && dl->frameAtual == 5) {  
        return; // <-- animação finalizada
    }

    dl->frameAtual++;

    // Linha 0 tem 5 frames
    if (dl->linhaAtual == 0){
        if (dl->frameAtual >= 5){
            dl->linhaAtual = 1;
            dl->frameAtual = 0;
        }
        return;
    }

    // Linha 1 tem 6 frames
    if (dl->linhaAtual == 1){
        if (dl->frameAtual >= 6){
            dl->frameAtual = 5;  // trava no último frame
        }
    }
}


void renderDialogo(SDL_Renderer* ren, Dialogo* dl, SDL_Rect camera, TTF_Font* fonte){
	
    if (!dl->ativado) return;
    
	//printf("[DBG] renderDialogo: ativado=%d texto=%p\n", dl->ativado, (void*)dl->texto);
	//if (dl->texto) printf("[DBG]   texto first 30 chars: %.30s\n", dl->texto);

    SDL_Rect dest = dl->pos;
    dest.x -= camera.x;

    // Desenha a caixa
    SDL_RenderCopy(ren, dl->tex, &dl->src, &dest);

    // Desenha o texto por cima da caixa
    SDL_Rect caixaCorrigida = dl->pos;
	caixaCorrigida.x -= camera.x;  // mesma posição usada para desenhar a caixa

	renderTextoDialogo(ren, fonte, dl->texto, caixaCorrigida, camera);
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

void renderMesa(SDL_Renderer* ren, Mesa* m, SDL_Rect camera) {
    SDL_Rect src = {
        m->frameAtual * m->larguraFrame,
        0,
        m->larguraFrame,
        m->alturaFrame
    };
    
	SDL_Rect dest = m->pos;
	dest.x -= camera.x;
    SDL_RenderCopy(ren, m->tex, &src, &dest);
}

void renderDialogosAcima(SDL_Renderer* ren, Cenario* c, SDL_Rect camera, TTF_Font* fonte) {
    for (int i = 0; i < c->numElementos; i++) {
        if (c->elementos[i].tipo == TIPO_ELEMENTO_DIALOGO) {
            renderDialogo(ren, &c->elementos[i].dialogo, camera, fonte);
        }
    }
}

void updateElementos(Cenario* c, SDL_Rect player, const Uint8* keys, float deltaTime, int* vidas, int* atual, int* dentro, int w) {

	for (int i = 0; i < c->numElementos; i++) {
        ElementoCenario* e = &c->elementos[i];
        switch (e->tipo) {
            case TIPO_ELEMENTO_CHECKPOINT:
                updateCheckpoint(&e->checkpoint, player, deltaTime);
                break;
            case TIPO_ELEMENTO_ACAMPAMENTO:
            	e->acampamento.frameAtual = SDL_HasIntersection(&player, &e->acampamento.pos) ? 1 : 0;
            	
                if (e->acampamento.frameAtual == 1 && keys[SDL_SCANCODE_UP]) {
                    e->acampamento.interagindo = 1;
                    
                    *atual = 3;
                } else {
                    e->acampamento.interagindo = 0;
                }
                break;
            case TIPO_ELEMENTO_MESA:
			    int tocando = SDL_HasIntersection(&player, &e->mesa.pos);
			
			    // controla sprite da mesa
			    e->mesa.frameAtual = tocando ? 1 : 0;
			
			    // procura o diálogo associado
			    Dialogo* dl = NULL;
			    for (int j = 0; j < c->numElementos; j++) {
			        if (c->elementos[j].tipo == TIPO_ELEMENTO_DIALOGO) {
			            dl = &c->elementos[j].dialogo;
			            break;
			        }
			    }
			
			    if (!dl) break;
			
			    // se apertou UP enquanto encostado na mesa
			    static int podePressionar = 1;
			    int up = keys[SDL_SCANCODE_UP];
			
			    if (!up) {
			        podePressionar = 1;  // libera para próxima leitura
			    }
			
			    if (tocando && up && podePressionar) {
			        podePressionar = 0;
			
			        if (!dl->ativado) {
			            // ativa diálogo
			            dl->ativado = 1;
			            dl->linhaAtual = 0;
			            dl->frameAtual = 0;
			            dl->ultimoFrame = SDL_GetTicks();
			        } else {
			            // desativa diálogo
			            dl->ativado = 0;
			        }
			    }
			
			    // se sair da área da mesa → esconde o diálogo
			    if (!tocando)
			        dl->ativado = 0;
			break;

            case TIPO_ELEMENTO_DIALOGO:
            	animarDialogo(&e->dialogo);
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
            case TIPO_ELEMENTO_MESA:
            	renderMesa(ren, &e->mesa, camera);
            	break;
            default: break;
        }
    }
}



void perderFlor(hudVida* hud, Uint32 agora) {
    if (hud->vidas <= 0 || hud->florAnimando != -1) return;
    hud->florAnimando = hud->vidas - 1;
    hud->frame = 0;
    hud->ultimoFrame = agora;
}

void updateFlor(hudVida* vida, Uint32 agora){
	// Atualiza animação da perda da flor
	if (vida->florAnimando != -1) {
	    if (agora - vida->ultimoFrame > vida->intervalo) {
	        vida->ultimoFrame = agora;
	        vida->frame++;
	
	        // quando terminar animação
	        if (vida->frame >= vida->totalFrames) {
	            vida->florAnimando =-1;
	            vida->frame =0;
	            vida->vidas--; // <-- AGORA DECREMENTA VIDA DE VERDADE
	        }
	    }
	}
}

void renderHudFlores(SDL_Renderer* ren, SDL_Texture* texFlor, hudVida hud, int w, SDL_Rect vidaRect) {
    for (int i = 0; i < 3; i++) {
        SDL_Rect florPos = { w/100 + i * w/25, vidaRect.h - 20, vidaRect.h/2, vidaRect.h/2 };
        SDL_Rect frame;

        if (i == hud.florAnimando)
            frame = (SDL_Rect){ 80 * hud.frame, 0, 80, 60 };
        else if (i < hud.vidas)
            frame = (SDL_Rect){ 0, 0, 80, 60 };
        else
            continue;

        SDL_RenderCopy(ren, texFlor, &frame, &florPos);
    }
}

void renderTextoDialogo(SDL_Renderer* ren, TTF_Font* fonte, const char* texto, SDL_Rect caixa, SDL_Rect camera) {
	
    SDL_Color cor = {255, 255, 255, 255};

	SDL_Surface* surf = TTF_RenderUTF8_Blended_Wrapped(fonte, texto, cor, caixa.w - 40);
	if (!surf) { printf("[DBG] TTF_Render error: %s\n", TTF_GetError()); return; }
	//printf("[DBG] surf->w=%d h=%d\n", surf->w, surf->h);

    SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
	if (!fonte) { printf("[DBG] fonte NULL\n"); return; }
	if (!texto) { printf("[DBG] texto NULL\n"); return; }
	//printf("[DBG] renderTextoDialogo: caixa=(%d,%d,%d,%d) camera=(%d,%d)\n", caixa.x, caixa.y, caixa.w, caixa.h, camera.x, camera.y);
	
	SDL_Rect dest = {
	    caixa.x + 20,
	    caixa.y + 40,
	    surf->w,
	    surf->h
	};


    SDL_RenderCopy(ren, tex, NULL, &dest);

    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

