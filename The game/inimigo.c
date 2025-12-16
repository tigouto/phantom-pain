#include "inimigo.h"

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

    p->sofrendoDano = 0;   // <-- corrigido
    p->vida = 10;
    p->morto = 0;
}

// ============================================
// UPDATE
// ============================================
void updatePedinte(Pedinte* p, SDL_Rect playerRect, Uint32 agora) {

	if(p->morto) return;
	
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

        // animação 6 frames
        if (agora - p->ultimoFrame > p->intervaloFrame) {
            p->ultimoFrame = agora;
            p->frameAtual++;

            if (p->frameAtual >= 6) {
                p->sofrendoDano = 0;      // <-- corrigido
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
            if (distancia < p->distanciaVisao) p->estado = INIMIGO_INICIANDO;
            break;

        case INIMIGO_INICIANDO:
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
    if (p->estado == INIMIGO_INICIANDO) {

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
    
    // ============================================
	// ANIMAÇÃO REVERSA (levantar de trás pra frente)
	// ============================================
	
	if (p->estado == INIMIGO_MORRENDO) {
		if (agora - p->ultimoFrame > p->intervaloFrame) {
	        p->ultimoFrame = agora;
	
	        if (p->frameAtual > 0) {
	            p->frameAtual--;       // toca ao contrário
	        } 
			else {
	            p->ativo = 0;          // ele nunca mais levanta
	            p->morto = 1;
	            
	             // mantém frame 0 morto
	            p->frameRect = (SDL_Rect){
	                0,      // frame 0
	                0,      // linha 0
	                210,180
	            };
	            return;
	        }
	    }
	
	    // linha 0 da spritesheet = levantar (normal)
	    p->frameRect = (SDL_Rect){
	        210 * p->frameAtual,
	        0,              // linha 0
	        210, 180
	    };
	
	    return;
	}
}

// Função de dano + knockback + animação

void bossTomarDano(Boss* p, int direcaoHit) {
	
	if(p->morto) return;

    // Vida
    p->vida--;
    if (p->vida <= 0) {
	    p->morto = 1;
	    return; // sai da função porque já está morto
	}
}

void pedinteTomarDano(Pedinte* p, int direcaoHit) {
	
	if(p->morto) return;
	if (p->estado == INIMIGO_MORRENDO) return;  // <-- BLOQUEIA DANO DURANTE MORTE

    p->estado = INIMIGO_DANO;
    p->sofrendoDano = 1;      // <-- corrigido
    p->frameAtual = 0;
    p->ultimoFrame = 0;
    p->intervaloFrame = 80;

    // Direção da animação
    p->dir = (direcaoHit > 0) ? DIREITA : ESQUERDA;

    // Knockback
    p->velDano = (direcaoHit > 0) ? 10 : -10;

    // Vida
    p->vida--;
    if (p->vida <= 0) {
	    p->estado = INIMIGO_MORRENDO;
	    p->frameAtual = 5;        // levantar reverso começa no último frame
	    p->ultimoFrame = SDL_GetTicks();
	    p->intervaloFrame = 150;  // ajuste se quiser mais lento
	    p->sofrendoDano = 0;
	    p->velDano = 0;
	    return; // sai da função porque já está morto
	}
}

// RENDER

void renderPedinte(SDL_Renderer* ren, SDL_Texture* tex, Pedinte* p, SDL_Rect camera) {
	
	int esquerdaTela = camera.x;
	int direitaTela  = camera.x + camera.w;
	
	if (p->pos.x + p->pos.w < esquerdaTela || p->pos.x > direitaTela) {
	    p->ativo = 0;
	    return;
	}
	
    SDL_Rect dest = p->pos;
    dest.x -= camera.x;
	

    SDL_RenderCopy(ren, tex, &p->frameRect, &dest);
}


// BOSS

//  INIT

void initBoss(Boss* b,int x, int y, int w, int h, int yFinal, int vida){
    b->pos = (SDL_Rect){x,y,w,h};
    
    b->vida = vida;
    b->morto = 0;
    
    b->frameRect = (SDL_Rect){0,0,200,260};
	b->distanciaVisao = 600;
	
    b->dir = ESQUERDA;
    b->velQueda = 12;
	b->yFinal = yFinal;

    b->ultimoFrame = 0;
    b->frameAtual = 0;
    b->intervaloFrame = 200;

    b->ativo = 1;
    // ⭐ ESSENCIAL — estava faltando e causava todo o bug
    b->estado = INIMIGO_CAINDO;
    
    b->podeDash = 1;
	b->ultimoDash = 0;
	b->duracaoDash = 300;   // 300 ms de dash
	b->velocidadeDash = 12; // ajustável
	b->tempoCarregamento = 800;
	b->inicioCarregamento = 0;
	b->distanciaDash = 600;
}

void updateBoss(Boss* p, SDL_Rect playerRect, Uint32 agora, int h) {

    float distancia = fabs((playerRect.x + playerRect.w/2) - (p->pos.x + p->pos.w/2));

    switch (p->estado) {

        // --------------------------------------------------
        // ESTADO: INICIANDO (queda inicial)
        // --------------------------------------------------
		case INIMIGO_CAINDO:
		
		    p->pos.y += p->velQueda;
		
		    // ANIMAÇÃO DE QUEDA (0–3 sem loop)
		    if (agora - p->ultimoFrame > p->intervaloFrame) {
		        p->ultimoFrame = agora;
		
		        if (p->frameAtual < 3) {
		            p->frameAtual++;  // avança até o 3
		        }
		
		        p->frameRect.x = p->frameAtual * p->frameRect.w;
		    }
			
			p->frameRect = (SDL_Rect){
		        200 * p->frameAtual,
		        260 * 0,
		        200, 260
		    };
		    
		    // CHEGOU NO CHÃO → mudar para ANDANDO
		    if (p->pos.y >= p->yFinal) {
		        p->pos.y = p->yFinal;
		
		        p->estado = INIMIGO_PARADO;
		
		        // começa no frame 4 (primeiro dos 2 finais)
		        p->frameAtual = 4;
		        p->ultimoFrame = agora;
		    }
		
		    break;

		case INIMIGO_PARADO:
			// ANIMAÇÃO 4→5 sem loop
		    if (agora - p->ultimoFrame > p->intervaloFrame) {
		        p->ultimoFrame = agora;
		
		        if (p->frameAtual == 4) {
		            p->frameAtual = 5;   // vai para o último frame
		        }
		        // se já está no 5 → não muda mais (sem loop)
		    }
		
		    p->frameRect = (SDL_Rect){
		        200 * p->frameAtual,
		        260 * 0,
		        200, 260
		    };
		    if(distancia < p->distanciaVisao){
		    	p->estado = INIMIGO_ANDANDO;
			}
		    break;
        // --------------------------------------------------
        // ESTADO: ANDANDO (persegue o jogador)
        // --------------------------------------------------
        case INIMIGO_ANDANDO:
		
		    // Define direção
		    p->dir = (playerRect.x < p->pos.x) ? ESQUERDA : DIREITA;
		
		    // Move em direção ao player
		    p->pos.x += (p->dir == DIREITA) ? 4 : -4;
		
		    // -------------------------------
		    // ANIMAÇÃO (6 frames — loop)
		    // -------------------------------
		    if (agora - p->ultimoFrame > p->intervaloFrame) {
		        p->ultimoFrame = agora;
		
		        p->frameAtual++;
		        if (p->frameAtual >= 6)   // passou dos 6 frames?
		            p->frameAtual = 0;   // volta para o início
		    }
		
		    // Seleciona linha da spritesheet
		    int linha = (p->dir == DIREITA) ? 2 : 1;
		
		    p->frameRect = (SDL_Rect){
		        200 * p->frameAtual,   // coluna da frame
		        260 * linha,           // linha 2 ou 3
		        200, 260
		    };
			
			if(distancia > p->distanciaVisao){
				p->estado = INIMIGO_TELEPORTANDO;
			}
			break;
			
		case INIMIGO_TELEPORTANDO:

		    // Alinha horizontalmente no centro do player
		    p->pos.x = playerRect.x + playerRect.w/2 - p->pos.w/2;
		
		    // Vai para o topo da tela (10% para não surgir colado na borda)
		    p->pos.y = h * 0.10;
		
		    // Reseta animação para começar a queda
		    p->frameAtual = 0;
		    p->ultimoFrame = agora;
		
		    // Próximo estado: queda
		    p->inicioCarregamento = agora;
			p->estado = INIMIGO_CARREGANDO;
	    	break;
	    	
	    case INIMIGO_CARREGANDO: 
	    
		    float distanciaX = fabs(
		        (playerRect.x + playerRect.w/2) -
		        (p->pos.x + p->pos.w/2)
		    );
		
		    // -----------------------------
		    // DASH AÉREO SE PLAYER FUGIR
		    // -----------------------------
		    if (distanciaX > p->distanciaDash && p->podeDash) {
		
		        p->dir = (playerRect.x < p->pos.x) ? ESQUERDA : DIREITA;
		
		        p->ultimoDash = agora;
		        p->podeDash = 0;
		    }
		
		    // Executa dash
		    if (!p->podeDash) {
		
		        p->pos.x += (p->dir == DIREITA)
		                    ? p->velocidadeDash
		                    : -p->velocidadeDash;
		
		        // Dash terminou?
		        if (agora - p->ultimoDash >= p->duracaoDash) {
		            p->podeDash = 1;
		        }
		    }
		
		    // -----------------------------
		    // ANIMAÇÃO DE CARREGAMENTO
		    // -----------------------------
		    p->frameRect = (SDL_Rect){
		        200 * 0,      // frame de carregamento
		        260 * 3,      // linha de carregamento
		        200, 260
		    };
		
		    // -----------------------------
		    // TERMINOU O CARREGAMENTO?
		    // -----------------------------
		    if (agora - p->inicioCarregamento >= p->tempoCarregamento) {
		        p->frameAtual = 0;
		        p->ultimoFrame = agora;
		        p->estado = INIMIGO_CAINDO;
		    }
		
		    break;
    }
}

void renderBoss(SDL_Renderer* ren, SDL_Texture* tex, Boss* b, SDL_Rect camera) {	
    SDL_Rect dest = b->pos;
    dest.x -= camera.x;
	

    SDL_RenderCopy(ren, tex, &b->frameRect, &dest);
}
