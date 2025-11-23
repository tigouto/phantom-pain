#include "player.h"

void initPlayer(Player* p, SDL_Renderer* ren, int x, int y, int w, int h) {
    p->pos = (SDL_Rect){x,y,w,h};
	p->framePernasRect = (SDL_Rect){0,0,230,210};
	p->frameTroncoRect = (SDL_Rect){0,0,230,210};
	p->dir = DIREITA;
    p->vely = 0;
    p->gravidade = 1;
    p->noChao = 1;
    p->movendo = 0;
	p->virando = 0;
    p->frameVirada = 0;
    p->frameID = 0;
	p->frameIE = 0;
    p->ultimoFrameTroca = 0;
    p->intervaloFrame = 120;

    p->atacando = 0;
	p->hitboxPlayer = (SDL_Rect){0,0,0,0};
    p->tempoAtaque = 0;
    p->intervaloEntreAtaques = 800;
    p->frameAtaquePernas = 0;
    p->frameAtaqueTronco = 0;
    p->ultimoFrameAtaque = 0;
    p->intervaloFrameAtaque = 100;
    p->totalFramesAtaquePernas = 4;
    p->totalFramesAtaqueTronco = 6;
    p->dano = 1;

    p->pulando = 0;
	p->puloInicial = -18;
	p->tempoPulo = 0.0f;
    p->framePulo = 0;
    p->ultimoFramePulo = 0;
    p->intervaloFramePulo = 200;
    p->totalFramesPulo = 4;
}

void updatePlayer(Player* p, const Uint8* keys, Uint32 agora, SDL_Rect chaoR, hudVida* vida, Cenario* cenarios, int atual, SDL_Rect* camera, SDL_Renderer* ren){
	int atacandoAgora = p->atacando;
	
	if (keys[SDL_SCANCODE_X]) {
            if (!p->virando && !p->atacando && agora - p->tempoAtaque > p->intervaloEntreAtaques && vida->vidas > 0) {
                p->atacando = 1;
                p->tempoAtaque = agora;
                p->ultimoFrameAtaque = p->tempoAtaque;
                
                int linhaAtaquePernas = (p->dir == DIREITA) ? 6 : 8;
				int linhaAtaqueTronco = (p->dir == DIREITA) ? 0 : 1;
				
				p->framePernasRect = (SDL_Rect){0, 210 * linhaAtaquePernas, 230, 210};
				p->frameTroncoRect = (SDL_Rect){0, 210 * linhaAtaqueTronco, 230, 210};

            }
        }

    if (p->atacando) {
        if (agora - p->ultimoFrameAtaque > p->intervaloFrameAtaque) {
            p->ultimoFrameAtaque = agora;
            p->frameAtaquePernas++;
            p->frameAtaqueTronco++;
            
			if (p->frameAtaqueTronco >= p->totalFramesAtaqueTronco) {
			
			    // Reset das pernas
			    if (p->dir == DIREITA){
			    	p->framePernasRect = (SDL_Rect){0, 0, 230, 210};
				}
			        
			    else
			        p->framePernasRect = (SDL_Rect){0, 210 * 2, 230, 210};
			
			    // Reset do tronco
			    p->frameTroncoRect = (SDL_Rect){0, 0, 230, 210};
			
			    // IMPORTANTE: zerar contadores
			    p->frameAtaquePernas = 0;
			    p->frameAtaqueTronco = 0;
				p->atacando = 0;
			} else {
			    // continua animando ataque
			    int linhaAtaquePernas = (p->dir == DIREITA) ? 6 : 8;
			    int linhaAtaqueTronco = (p->dir == DIREITA) ? 0 : 1;
			
			    p->framePernasRect = (SDL_Rect){
			        230 * (p->frameAtaquePernas % p->totalFramesAtaquePernas),
			        210 * linhaAtaquePernas,
			        230, 210
			    };
			
			    p->frameTroncoRect = (SDL_Rect){
			        230 * (p->frameAtaqueTronco % p->totalFramesAtaqueTronco),
			        210 * linhaAtaqueTronco,
			        230, 210
			    };
			}
    	}

        for (int i = 0; i < cenarios[atual].numPedintes; i++) {
    		Pedinte* pe = &cenarios[atual].pedintes[i];
    		if(pe->morto) continue;
    		
    		if(SDL_HasIntersection(&p->hitboxPlayer,&pe->pos)){
				pe->pos.x += (pe->dir == DIREITA) ? -RECUO_PEDINTE : RECUO_PEDINTE;

	        	// aplica o dano do player
		        pe->vida -= p->dano;
		
		        // se a vida chegou a zero → morreu
		        if (pe->vida <= 0) {
		            pe->morto = 1;
		        }
    		}
		}
		
		SDL_Rect hitScreen = p->hitboxPlayer;
        hitScreen.x -= camera->x;
        SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ren, 255, 0, 0, 128);
        SDL_RenderFillRect(ren, &hitScreen);
    }
    
    if(p->atacando){
	    // largura e altura do golpe (ajuste conforme sua sprite)
	    int hitW = 80;
	    int hitH = 80;
	
	    if (p->dir == DIREITA) {
	        p->hitboxPlayer.x = p->pos.x + p->pos.w - 20; // pequeno offset
	        p->hitboxPlayer.y = p->pos.y + 40;            // alinhamento vertical
	    } else {
	        p->hitboxPlayer.x = p->pos.x - hitW + 20;
	        p->hitboxPlayer.y = p->pos.y + 40;
	    }
	
	    p->hitboxPlayer.w = hitW;
	    p->hitboxPlayer.h = hitH;
	}
	else {
        p->hitboxPlayer = (SDL_Rect){0,0,0,0};
    }
    
    if (keys[SDL_SCANCODE_LEFT]) {
        p->movendo = 1;
        
        if (p->dir == DIREITA && p->virando != 1 ) {
            p->virando = 1; p->frameVirada = 0; p->ultimoFrameTroca = agora;
        }
        if (atual != 0 || p->pos.x > 55) p->pos.x -= 13;
        p->dir = ESQUERDA;
        if (p->virando == 0) {
            if (agora - p->ultimoFrameTroca > p->intervaloFrame) {
                p->ultimoFrameTroca = agora;
                p->frameIE++;
                if (p->frameIE > 3) p->frameIE = 0;
            }
            p->framePernasRect = (SDL_Rect){230 * p->frameIE, 210 * 2, 230, 210};
        }
    }
    // direita
    if (keys[SDL_SCANCODE_RIGHT]) {
        p->movendo = 1;
        if (p->dir == ESQUERDA && p->virando != 2) {
            p->virando = 2; p->frameVirada = 0; p->ultimoFrameTroca = agora;
        }
        p->pos.x += 13;
        p->dir = DIREITA;
        if (p->virando == 0) {
            if (agora - p->ultimoFrameTroca > p->intervaloFrame) {
                p->ultimoFrameTroca = agora;
                p->frameID++;
                if (p->frameID > 3) p->frameID = 0;
            }
            p->framePernasRect = (SDL_Rect){230 * p->frameID, 0, 230, 210};
        }
    }
    
    if (keys[SDL_SCANCODE_Z] && p->noChao) {
    	if (!p->pulando){
    		p->vely = p->puloInicial;
            p->noChao = 0;
            p->pulando = 1;
            p->tempoPulo = agora;
            p->framePulo = 0;
            p->ultimoFramePulo = p->tempoPulo;
		}
	}
	
	if (p->pulando && !p->virando) {
        if (agora - p->ultimoFramePulo > p->intervaloFramePulo) {
        	p->ultimoFramePulo = agora;
    		p->framePulo++;
        	if (p->framePulo >= p->totalFramesPulo){
        		p->pulando = 0;
        		if (p->dir == DIREITA) p->framePernasRect = (SDL_Rect){0,0,230,210};
        		else p->framePernasRect = (SDL_Rect){0, 210 * 2, 230, 210};
        		return;
			}
		}
        int linhaPulo = (p->dir == DIREITA) ? 4 : 5; // supondo linhas 4/5
        p->framePernasRect = (SDL_Rect){230 * (p->framePulo % p->totalFramesPulo), 210 * linhaPulo, 230, 210};
	}
	
	if (p->virando == 1) {
        if (agora - p->ultimoFrameTroca > p->intervaloFrame) {
            p->ultimoFrameTroca = agora;
            p->frameVirada++;
            if (p->frameVirada > 2) { 
				p->virando = 0; 
				p->frameIE = 0; 
				p->dir = ESQUERDA;
				p->framePernasRect = (SDL_Rect){0, 210 * 2, 230, 210};
				return; 
			}
            p->framePernasRect = (SDL_Rect){230 * p->frameVirada, 210 * 1, 230, 210};
        }
    } 
	if (p->virando == 2) {
        if (agora - p->ultimoFrameTroca > p->intervaloFrame) {
            p->ultimoFrameTroca = agora;
            p->frameVirada++;
            if (p->frameVirada > 2) {
				p->virando = 0; 
				p->frameID = 0; 
				p->dir = DIREITA; 
				p->framePernasRect = (SDL_Rect){0, 0, 230, 210};
				return;
			}
            p->framePernasRect = (SDL_Rect){230 * p->frameVirada, 210 * 3, 230, 210};
        }
    }

    if (!p->pulando && !p->movendo && !p->virando && !p->atacando && vida->vidas > 0) {
        if (p->dir == DIREITA) p->framePernasRect = (SDL_Rect){0,0,230,210};
        else p->framePernasRect = (SDL_Rect){0, 210 * 2, 230, 210};
    }
    
    p->pos.y += p->vely;
    p->vely += p->gravidade;
    
    if (p->pos.y + p->pos.h >= chaoR.y) {
        p->pos.y = chaoR.y - p->pos.h; 
		p->vely = 0; 
		p->noChao = 1;
	}
	
	/*for (int i = 0; i < numPlataformas; i++) {
        SDL_Rect plat = plataformas[i];
        if (vely >= 0 && player.y + player.h > plat.y && player.y + player.h - vely <= plat.y &&
            player.x + player.w > plat.x && player.x < plat.x + plat.w) {
            player.y = plat.y - player.h;
            vely = 0;
            noChao = 1;
        }
    }*/
    
    for (int i = 0; i < cenarios[atual].numPedintes; i++) {
	    Pedinte* pe = &cenarios[atual].pedintes[i];
	    
	    if(pe->morto) continue;
	    
	    if (SDL_HasIntersection(&p->pos, &pe->pos) ) {
		    pe->sofrendoDano = 1;
		    if (pe->dir == DIREITA) pe->velDano = -12;   // empurra para esquerda
		    else pe->velDano = 12;    // empurra para direita
	        if (!vida->invulneravel) {
			    perderFlor(vida, agora);
			    vida->invulneravel = 1;
			    vida->tempoInvulneravel = agora;
			}
	    }
	}
}

void renderPlayer(Player* p, SDL_Renderer* ren, SDL_Texture* sprites, SDL_Texture* spritesCorpo,SDL_Rect camera){
	SDL_Rect playerScreen = p->pos;
    playerScreen.x -= camera.x;
    SDL_RenderCopy(ren, sprites, &p->framePernasRect, &playerScreen);
    if (p->atacando){
    	SDL_RenderCopy(ren, spritesCorpo, &p->frameTroncoRect, &playerScreen);
	}
}
