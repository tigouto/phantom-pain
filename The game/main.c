#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include "cenario.h"
#include "player.h"
#include "pedinte.h"

/* ----------------- FUNÇÃO PRINCIPAL DO JOGO (runGame) ----------------- */

typedef enum{
	ESTADO_JOGO,
	ESTADO_MORTE,
	ESTADO_PAUSA
} EstadoGame;

EstadoGame estadoAtual = ESTADO_JOGO;

void renderMenuPause(SDL_Renderer* ren, int w, int h, int opcaoPause, SDL_Texture* menuPauseC, SDL_Texture* contPause, SDL_Texture* sairPause, SDL_Texture* menuPauseB){
    // Fundo escuro
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 160);
    SDL_Rect tela = {0, 0, w, h};
    SDL_RenderFillRect(ren, &tela);

    // ----- FLOR DE CIMA -----
    SDL_Rect topo = { (w-670*0.8)/2, 50, 670*0.8, 170*0.8 };
    SDL_RenderCopy(ren, menuPauseC, NULL, &topo);

    // ----- BOTÕES -----
    int btnW = 350, btnH = 80;
    int startY = topo.y + topo.h + 60;

    SDL_Rect btnContinuar = { (w - 660*0.8)/2, startY, 660*0.8, 70*0.8 };
    SDL_Rect btnSair      = { (w - 920*0.8)/2, startY + btnH + 45, 920*0.8, 70*0.8 };

    SDL_Rect srcSelC = { 660,0,660,70 }; // highlight
    SDL_Rect srcNC   = {   0,0,660,70 }; // normal
    SDL_Rect srcSelS = { 920,0,920,70 }; // highlight
    SDL_Rect srcNS   = {   0,0,920,70 }; // normal

    // Continuar
    if (opcaoPause == 0)
        SDL_RenderCopy(ren, contPause, &srcSelC, &btnContinuar);
    else
        SDL_RenderCopy(ren, contPause, &srcNC, &btnContinuar);

    // Sair
    if (opcaoPause == 1)
        SDL_RenderCopy(ren, sairPause, &srcSelS, &btnSair);
    else
        SDL_RenderCopy(ren, sairPause, &srcNS, &btnSair);

    // ----- FLOR DE BAIXO -----
    SDL_Rect baixo = { (w-680*0.8)/2, btnSair.y + btnH + 80, 680*0.8, 170*0.8 };
    SDL_RenderCopy(ren, menuPauseB, NULL, &baixo);
}

void renderMenuMorte(SDL_Renderer* ren, int w, int h, int opcaoPause, SDL_Texture* acordar, SDL_Texture* lembrar, SDL_Texture* fundoMorte, SDL_Texture* tituloMorte){
    // Fundo escuro
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 160);
    SDL_Rect tela = {0, 0, w, h};
    SDL_RenderFillRect(ren, &tela);

    // ----- FLOR DE CIMA -----
    SDL_Rect topo = { (w-1060*0.8)/2, h/4, 1060*0.8, 250*0.8 };
    SDL_RenderCopy(ren, tituloMorte, NULL, &topo);

    // ----- BOTÕES -----
    int btnW = 350, btnH = 80;
    int startY = topo.y + topo.h + 60;

    SDL_Rect btnLembrar = { (w - 570*0.8)/2, startY, 570*0.8, 80*0.8 };
    SDL_Rect btnAcordar     = { (w - 540*0.8)/2, startY + btnH + 45, 540*0.8, 80*0.8 };

    SDL_Rect srcSelL = { 570,0,570,80 }; // highlight
    SDL_Rect srcNL   = {   0,0,570,80 }; // normal
    SDL_Rect srcSelA = { 540,0,540,80 }; // highlight
    SDL_Rect srcNA   = {   0,0,540,80 }; // normal

    // Continuar
    if (opcaoPause == 0)
        SDL_RenderCopy(ren, lembrar, &srcSelL, &btnLembrar);
    else
        SDL_RenderCopy(ren, lembrar, &srcNL, &btnLembrar);

    // Sair
    if (opcaoPause == 1)
        SDL_RenderCopy(ren, acordar, &srcSelA, &btnAcordar);
    else
        SDL_RenderCopy(ren, acordar, &srcNA, &btnAcordar);

    // ----- FLOR DE BAIXO -----
    //SDL_Rect baixo = { (w-680*0.8)/2, btnSair.y + btnH + 80, 680*0.8, 170*0.8 };
    //SDL_RenderCopy(ren, acordar, NULL, &baixo);
}

void runGame(SDL_Window* win, SDL_Renderer* ren){
	int dentro = 0;
	int opcaoPause = 0;
	int podeUsarEsc = 1;
	
	int w, h;
	SDL_GetWindowSize(win, &w, &h);
	
	SDL_Rect camera = {0,0,w,h};
	int atual = 0;
    // --- Texturas básicas (sprites, HUD, pedinte, flor) ---
    SDL_Texture* sprites = IMG_LoadTexture(ren, "./src/entidades/ss reaper.png");
    SDL_Texture* spritesCorpo = IMG_LoadTexture(ren,"./src/entidades/ss reaper ataque.png");
    SDL_Texture* spritesFoice = IMG_LoadTexture(ren,"./src/entidades/ss lamina.png");
    SDL_Texture* hud = IMG_LoadTexture(ren, "./src/mapa/hud.png");
    SDL_Texture* texPedinte = IMG_LoadTexture(ren, "./src/entidades/ss pedinte.png");
    SDL_Texture* texFlor = IMG_LoadTexture(ren, "./src/mapa/ss flor.png"); // novo

	// TEXTURAS MENU PAUSE
	SDL_Texture* menuPauseC = IMG_LoadTexture(ren, "./src/menu/pause/flor-cima-pause.png");
    SDL_Texture* contPause = IMG_LoadTexture(ren, "./src/menu/pause/continuar-pause.png");
    SDL_Texture* sairPause = IMG_LoadTexture(ren, "./src/menu/pause/sair-pause.png");
    SDL_Texture* menuPauseB = IMG_LoadTexture(ren, "./src/menu/pause/flor-baixo-pause.png");
    
    // TEXTURAS MENU MORTE
	SDL_Texture* acordar = IMG_LoadTexture(ren, "./src/morte/acordar morte.png");
    SDL_Texture* lembrar = IMG_LoadTexture(ren, "./src/morte/lembrar morte.png");
    SDL_Texture* fundoMorte = IMG_LoadTexture(ren, "./src/morte/ss fundo morte.png");
    SDL_Texture* tituloMorte = IMG_LoadTexture(ren, "./src/morte/titulo morte.png");

    // Texturas de cenário
    SDL_Texture* fundo_tex  = IMG_LoadTexture(ren, "./src/mapa/ponte-f/bg+lua.png");
    SDL_Texture* parafu_tex = IMG_LoadTexture(ren, "./src/mapa/ponte-f/paralax fundo.png");
    SDL_Texture* parafr_tex = IMG_LoadTexture(ren, "./src/mapa/ponte-f/paralax frente.png");
    SDL_Texture* ponte_tex  = IMG_LoadTexture(ren, "./src/mapa/ponte-f/ponte.png");
    SDL_Texture* portao_tex = IMG_LoadTexture(ren, "./src/mapa/ponte-f/portão.png");
    SDL_Texture* ponte_prox = IMG_LoadTexture(ren, "./src/mapa/ponte-f/sala-port.png");
	SDL_Texture* texCheckpoint = IMG_LoadTexture(ren, "./src/mapa/ponte-f/checkpoint.png");
	SDL_Texture* texAcampamento = IMG_LoadTexture(ren, "./src/mapa/ponte-f/acampamento.png");
    // sala textures (exemplo)
    SDL_Texture* fundo_sala = IMG_LoadTexture(ren, "./src/mapa/sala-p/background.png");
    SDL_Texture* porta_sala = IMG_LoadTexture(ren, "./src/mapa/sala-p/porta.png");
    SDL_Texture* borda_sala = IMG_LoadTexture(ren, "./src/mapa/sala-p/borda.png");
    SDL_Texture* atras_sala = IMG_LoadTexture(ren, "./src/mapa/sala-p/fundo-atras.png");
    SDL_Texture* frente_sala = IMG_LoadTexture(ren, "./src/mapa/sala-p/fundo-frente.png");
    // texturas acampamento
	SDL_Texture* fundo_acampamento = IMG_LoadTexture(ren, "./src/mapa/acampamento/fundo-acampamento.png");
	SDL_Texture* frente_acampamento = IMG_LoadTexture(ren, "./src/mapa/acampamento/frente-acampamento.png");
	SDL_Texture* texMesa = IMG_LoadTexture(ren, "./src/mapa/acampamento/mesa-acampamento.png");
	SDL_Texture* texDialogo = IMG_LoadTexture(ren, "./src/mapa/ss caixa-diálogo.png");

    // --- Preparar cenários (igual ao seu original) ---
    Cenario cenarios[MAX_CENARIOS];
    int totalCenarios = 0;

    initCenario(&cenarios[0]);
    cenarios[0].fundo = fundo_tex;
    addParalax(&cenarios[0], parafu_tex, 3.0f, 20);
    addParalax(&cenarios[0], parafr_tex, 1.5f, 20);
    
    SDL_Rect ponteR = { 0, h - ((100 * h) / 100), 4000, (100 * h) / 100 };
    SDL_Rect portaR = { 0, h - 399 - ((h * 7) / 100), 115, 400 };
    SDL_Rect portaProxR = { 4000, 0, 70, h };
    SDL_Rect hitboxGradeLocal = {w-(w*6.5)/100, 0, (w*6.5)/100, h };
    SDL_Rect hitboxBarreiraC1 = {w/3, 0, w/3, h };
    SDL_Rect hitboxBarreiraC2 = {w-(w/3)/100, 0, (w/3)/100, h };

    
    addAtras(&cenarios[0], portao_tex, portaR);
    addAtras(&cenarios[0], ponte_prox, portaProxR);
    addFrente(&cenarios[0], ponte_tex, ponteR);
    
	cenarios[0].posX = 0;
    cenarios[0].largura = 4070;
    cenarios[0].altura = h;
    
	totalCenarios++;

	initCenario(&cenarios[1]);
	cenarios[1].fundo = fundo_sala;
	
	// começa logo após o primeiro cenário
	cenarios[1].posX = cenarios[0].posX + cenarios[0].largura;
	
	// largura igual à tela
	cenarios[1].largura = w;
	cenarios[1].altura = h;
	
	SDL_Rect portaSalaRect = {0, 0, (w*6.5)/100, h*70/100};
	
	addAtras(&cenarios[1], fundo_sala, (SDL_Rect){0, 0, w, h});
	addAtras(&cenarios[1], atras_sala, (SDL_Rect){0, 0, w, h});
	addAtras(&cenarios[1], frente_sala, (SDL_Rect){0, 0, w, h});
	addFrente(&cenarios[1], porta_sala, portaSalaRect);
	addFrente(&cenarios[1], borda_sala, (SDL_Rect){0, 0, w, h});
	
	// --- NOVO --- variáveis da porta
	int portaDescendo = 0;
	int portaAlturaFinal = h - portaSalaRect.h;   // encosta no chão
	float portaVel = 600.0f; // pixels por segundo
	float portaY = 0;        // posição atual em float
	int portaFechada = 0;


	totalCenarios++;
	
	initCenario(&cenarios[2]);
	cenarios[2].fundo = fundo_acampamento;
	
	// posição: começa logo após o cenário 1
	cenarios[2].posX = cenarios[1].posX + cenarios[1].largura;
	
	// largura: igual à tela (pode alterar se o acampamento for maior)
	cenarios[2].largura = w;
	cenarios[2].altura = h;
	
	// camadas "atrás" do player
	addAtras(&cenarios[2], fundo_acampamento, (SDL_Rect){0, 0, w, h});
	addFrente(&cenarios[2], frente_acampamento, (SDL_Rect){0, 0, w, h});
	totalCenarios++;



    // --- Jogador / HUD / Física / Ataque (integração do teste.c) ---
    SDL_ShowCursor(SDL_DISABLE);
    
	Player player;
    initPlayer(&player,ren,w/5, (h - ((15*h)/100)/3) - 105, 110, 100);
    
    SDL_Rect chaoR = { 0, h-(ponteR.y + ponteR.h)/15 + 2, w, (ponteR.y + ponteR.h)/15};

    // Plataformas (do teste.c)
    /*int numPlataformas = 0;
    SDL_Rect plataformas[2] = {
        { 200, h - 200, 150, 20 },
        { 400, h - 300, 150, 20 },
    };*/

    // HUD flores
    hudVida vida = {
    	.vidaRect = {0, 0, w/5, h/10},
	    .vidas = 3,
	    .florAnimando = -1,
	    .frame = 0,
	    .ultimoFrame = 0,
	    .totalFrames = 13,
	    .intervalo = 120,
	    .invulneravel = 0,
	    .tempoInvulneravel = 0
	};

	// Checkpoints
	addCheckpointElemento(
	    &cenarios[0],
	    texCheckpoint,
	    (SDL_Rect){ 120, h-(ponteR.y+ponteR.h)/15-163, 390, 160 }, // posição e tamanho do primeiro frame
	    6,    // número de frames
	    390,  // largura de cada frame
	    160,  // altura de cada frame
	    100   // tempo entre frames em ms (~10 FPS)
	);
	
	addAcampamentoElemento(
		&cenarios[0], 
		texAcampamento, 
		(SDL_Rect){ 2500, h-(ponteR.y+ponteR.h)/15-273, 520, 270 },
		2,   // numero de frames
		520, // largura
		270  // altura
	);
	
	addMesaElemento(&cenarios[2],texMesa,(SDL_Rect){cenarios[2].posX + w/2.7, h-(ponteR.y+ponteR.h)/15-203, 200, 200 },2,270,280);
	
	addDialogoElemento(&cenarios[2],texDialogo,(SDL_Rect){cenarios[2].posX + w/5,h/30,3*w/5,h/2});
	
	// NPCs
    addPedinte(&cenarios[0], 3*w/5, chaoR.y-100, 110, 100);
    addPedinte(&cenarios[0], 9*w/5, chaoR.y-100, 110, 100);

    // render inicial + fade in
    SDL_RenderClear(ren);
    desenharCenario(ren, &cenarios[atual], camera, w, h,w,h);
    SDL_RenderCopy(ren, sprites, &player.framePernasRect, &(SDL_Rect){player.pos.x - camera.x, player.pos.y, player.pos.w, player.pos.h});
    SDL_RenderPresent(ren);
    fade_out_in(ren, w, h, 0);

    int espera = 16;
    while (!SDL_QuitRequested()) {
        SDL_Event evt;
        int isevt = SDL_WaitEventTimeout(&evt, espera);
        if (isevt && evt.type == SDL_QUIT) break;

        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        Uint32 agora = SDL_GetTicks();

		if (!keys[SDL_SCANCODE_ESCAPE]) {
		    podeUsarEsc = 1;
		}

        if (estadoAtual == ESTADO_JOGO){
        	
        	if (isevt && evt.type == SDL_KEYDOWN && evt.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
	            if(estadoAtual == ESTADO_JOGO && podeUsarEsc){
	            	estadoAtual = ESTADO_PAUSA;
	                opcaoPause = 0;
	                podeUsarEsc = 0;
	            }
	        }
        	
            updatePlayer(&player, keys, agora, chaoR, &vida, cenarios, atual, &camera, ren,w);
        	ImpedirPassar(&player, portaR);

			if (atual == 1) {
			    SDL_Rect hitboxGradeGlobal = hitboxGradeLocal;
			    hitboxGradeGlobal.x += cenarios[1].posX; // <-- deslocamento correto
			
			    ImpedirPassar(&player, hitboxGradeGlobal);
			    
			    int meioCenario = cenarios[1].posX + cenarios[1].largura / 2;

			    if (!portaFechada && !portaDescendo && player.pos.x > meioCenario) {
			        portaDescendo = 1;   // inicia animação
			        portaY = portaSalaRect.y;
			    }
			    
			    if(portaFechada){
			    	SDL_Rect portaGlobal = cenarios[1].frente[0].pos;
				    portaGlobal.x += cenarios[1].posX;  // converter para coordenada global
				    ImpedirPassar(&player, portaGlobal);
				}
			}
			
			if (atual == 2) {
			    // largura das barreiras (ajuste se quiser)
			    int larguraBarreira = w / 20; // ~5% da largura da tela (ex.: 1920 -> 96px)
			    int alturaBarreira  = h;      // ocupa a altura inteira
			
			    // barreira que já existe (impede ir para a esquerda) - mantemos na mesma posição usada antes
			    SDL_Rect barreiraEsq = {
			        cenarios[2].posX + w/3.5, // x global (mesma lógica que você usava)
			        0,
			        larguraBarreira,
			        alturaBarreira
			    };
			
			    // barreira direita (impede ir para a direita)
			    // colocamos a barreira perto da borda direita do cenário 2
			    SDL_Rect barreiraDir = {
			        // posX do cenário + largura do cenário - offset - largura da barreira
			        cenarios[2].posX + cenarios[2].largura - (w/3.5) - 0.5*larguraBarreira,
			        0,
			        larguraBarreira,
			        alturaBarreira
			    };
			
			    // aplica as duas barreiras
			    ImpedirPassar(&player, barreiraEsq); // bloqueia movimento para a esquerda quando colide por esse lado
			    //ImpedirPassar(&player, barreiraDir);
			    if (player.pos.x + player.pos.w > barreiraDir.x) {
				
				    // Volta para o cenário 0
				    fade_out_in(ren,w,h,1);
				    atual = 0;
				    player.pos.x = cenarios[atual].posX + 2700, h-(ponteR.y+ponteR.h)/15-273 ;
					fade_out_in(ren,w,h,0);
					dentro = 0;
				}
			}

            // Atualiza pedintes
            for (int i = 0; i < cenarios[atual].numPedintes; i++){
            	updatePedinte(&cenarios[atual].pedintes[i], player.pos, agora);
		    }
		    
		    // Atualiza checkpoints
		    float deltaTime = 16.0f; // o tempo decorrido entre frames

	    	updateElementos(&cenarios[atual], player.pos, keys, deltaTime, &vida.vidas, &atual, &dentro, w);
	    	
            if(atual == 2 && !dentro){
                fade_out_in(ren,w,h,1);
                player.pos.x = cenarios[atual].posX + w*3/5;
                fade_out_in(ren,w,h,0);
                dentro = 1;
            }
            
            
			if(!dentro){
            // transições de cenario
	            if (player.pos.x > cenarios[atual].posX + cenarios[atual].largura && (atual + 1) < totalCenarios) {
	                fade_out_in(ren, w, h, 1);
	                atual++;
	                player.pos.x = cenarios[atual].posX;
	                camera.x = 0;
	                camera.y = 0;
	                if (camera.x < cenarios[atual].posX) camera.x = cenarios[atual].posX;
	                if (camera.x > cenarios[atual].posX + cenarios[atual].largura - w)
	                    camera.x = cenarios[atual].posX + cenarios[atual].largura - w;
	                fade_out_in(ren, w, h, 0);
	            }
	            if (player.pos.x + player.pos.w < cenarios[atual].posX) {
	                fade_out_in(ren, w, h, 1);
	                atual--;
	                player.pos.x = cenarios[atual].posX + cenarios[atual].largura - 120;
	                camera.x = player.pos.x + player.pos.w / 2 - w / 2;
	                if (camera.x < cenarios[atual].posX) camera.x = cenarios[atual].posX;
	                if (camera.x > cenarios[atual].posX + cenarios[atual].largura - w)
	                    camera.x = cenarios[atual].posX + cenarios[atual].largura - w;
	                fade_out_in(ren, w, h, 0);
	            }
            }

            // Invulnerabilidade timeout
            if (vida.invulneravel && agora - vida.tempoInvulneravel > TEMPO_INVULNERAVEL) {
                vida.invulneravel = 0;
            }

            // câmera
            camera.x = player.pos.x + player.pos.w/2 - w/2;
            if (camera.x < cenarios[atual].posX) camera.x = cenarios[atual].posX;
            if (camera.x > cenarios[atual].posX + cenarios[atual].largura - w)
                camera.x = cenarios[atual].posX + cenarios[atual].largura - w;
        }
        
        if (portaDescendo) {
		    float dt = espera / 1000.0f; // 16 ms → 0.016 s
		
		    portaY += portaVel * dt;
		
		    if (portaY >= portaAlturaFinal) {
		        portaY = portaAlturaFinal;
		        portaDescendo = 0; // terminou
		        portaFechada = 1;
		    }
		    // aplica ao rect da camada
		    cenarios[1].frente[0].pos.y = (int)portaY;  
		}
		
        // --- RENDER ---
        SDL_RenderClear(ren);
        desenharCenario(ren, &cenarios[atual], camera, w, h,w,h);

        // plataformas (debug fill)
        /*for (int i = 0; i < numPlataformas; i++) {
            SDL_SetRenderDrawColor(ren, 0x80, 0x80, 0x80, 0xFF);
            SDL_RenderFillRect(ren, &plataformas[i]);
        }*/
		
		renderElementos(ren, &cenarios[atual], camera);

		// pedintes
        for (int i = 0; i < cenarios[atual].numPedintes; i++){
        	renderPedinte(ren, texPedinte, &cenarios[atual].pedintes[i], camera);
		}

        // jogador
        renderPlayer(&player, ren, sprites, spritesCorpo, spritesFoice, camera);

        // desenha frente cenário
        desenharFrente(ren, &cenarios[atual], camera);
        
        renderDialogosAcima(ren, &cenarios[atual], camera);

        // HUD: desenha hud base
        SDL_RenderCopy(ren, hud, NULL, &vida.vidaRect);

		// Atualiza animação da perda da flor
		updateFlor(&vida, agora);
		
		if(vida.vidas == 0){
			estadoAtual = ESTADO_MORTE;
		}
		
		renderHudFlores(ren, texFlor, vida, w, vida.vidaRect);

		if(estadoAtual == ESTADO_MORTE){
            renderMenuMorte(
			    ren, w, h,
			    opcaoPause,
			    acordar,
			    lembrar,
			    fundoMorte,
			    tituloMorte
			);
        	
            if (isevt && evt.type == SDL_KEYDOWN) {	
            	if (podeUsarEsc && evt.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
	            	podeUsarEsc = 0;
	        	}
            	
                switch(evt.key.keysym.scancode) {
                    case SDL_SCANCODE_DOWN:
                    case SDL_SCANCODE_UP:
                        if (opcaoPause == 0) {
                            opcaoPause = 1;
                        } else if (opcaoPause == 1) {
                            opcaoPause = 0;
                        }
                        break;
                    
                    case SDL_SCANCODE_RETURN:
                    case SDL_SCANCODE_Z:
                        if(opcaoPause == 1){
                            return;
                        }
                        break;
            	}
	    	}
        }

        if(estadoAtual == ESTADO_PAUSA){
            renderMenuPause(
			    ren, w, h,
			    opcaoPause,
			    menuPauseC,
			    contPause,
			    sairPause,
			    menuPauseB
			);

        
            if (isevt && evt.type == SDL_KEYDOWN) {	
            	if (podeUsarEsc && evt.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
	            	estadoAtual = ESTADO_JOGO;
	            	podeUsarEsc = 0;
	        	}
            	
                switch(evt.key.keysym.scancode) {
                    case SDL_SCANCODE_DOWN:
                    case SDL_SCANCODE_UP:
                        if (opcaoPause == 0) {
                            opcaoPause = 1;
                        } else if (opcaoPause == 1) {
                            opcaoPause = 0;
                        }
                        break;
                    
                    case SDL_SCANCODE_RETURN:
                    case SDL_SCANCODE_Z:
                        if(opcaoPause == 0){
                            estadoAtual = ESTADO_JOGO;
                        } else if(opcaoPause == 1){
                            return;
                        }
                        break;
            	}
	    	}
        }
        SDL_RenderPresent(ren);
    }

    // --- LIMPEZA ---
    if (fundo_tex) SDL_DestroyTexture(fundo_tex);
    if (parafu_tex) SDL_DestroyTexture(parafu_tex);
    if (parafr_tex) SDL_DestroyTexture(parafr_tex);
    if (ponte_tex) SDL_DestroyTexture(ponte_tex);
    if (portao_tex) SDL_DestroyTexture(portao_tex);
    if (ponte_prox) SDL_DestroyTexture(ponte_prox);
    if (fundo_sala) SDL_DestroyTexture(fundo_sala);
    if (borda_sala) SDL_DestroyTexture(borda_sala);
    if (atras_sala) SDL_DestroyTexture(atras_sala);
    if (frente_sala) SDL_DestroyTexture(frente_sala);

    SDL_DestroyTexture(hud);
    SDL_DestroyTexture(spritesFoice);
    SDL_DestroyTexture(spritesCorpo);
    SDL_DestroyTexture(sprites);
    SDL_DestroyTexture(texPedinte);
    SDL_DestroyTexture(texFlor);
    SDL_DestroyTexture(texCheckpoint);
    SDL_DestroyTexture(texAcampamento);
    SDL_DestroyTexture(texMesa);
    SDL_DestroyTexture(texDialogo);
    SDL_DestroyTexture(acordar);
    SDL_DestroyTexture(lembrar);
    SDL_DestroyTexture(fundoMorte);
    SDL_DestroyTexture(tituloMorte);
    


    SDL_ShowCursor(SDL_ENABLE);
}

/* ----------------- FUNÇÃO main (menu) ----------------- */
int main(int argc, char* args[]) {
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);
	
    SDL_Window* win = SDL_CreateWindow("Phantom Pain v0.1", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                       0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, 0);

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    
    SDL_Surface* cursorSurface = IMG_Load("./src/menu/cursor.png");
    SDL_Cursor* cursorMenu = NULL;

    SDL_Texture* fundo      = IMG_LoadTexture(ren, "./src/menu/bg-menu.png");
    SDL_Texture* logo       = IMG_LoadTexture(ren, "./src/menu/pp-logo.png");
    SDL_Texture* novo       = IMG_LoadTexture(ren, "./src/menu/novo-j.png");
    SDL_Texture* continuar  = IMG_LoadTexture(ren, "./src/menu/continuar.png");
    SDL_Texture* sair       = IMG_LoadTexture(ren, "./src/menu/sair.png");
    assert(fundo && logo && novo && continuar && sair);

    int w, h;
    SDL_GetWindowSize(win, &w, &h);

    int logoW = 640, logoH = 290;
    float escalaLogo = 1.2f;
    int logoFinalW = (int)(logoW * escalaLogo);
    int logoFinalH = (int)(logoH * escalaLogo);
    int logoY = 40;
    SDL_Rect titulo = { (w - logoFinalW)/2, logoY, logoFinalW, logoFinalH };

    int btnSrcW = 315, btnSrcH = 35;
    float escalaBtn = 1.8f;
    int btnFinalW = (int)(btnSrcW * escalaBtn);
    int btnFinalH = (int)(btnSrcH * escalaBtn);
    int espacamento = 40;
    int startY = logoY + logoFinalH + 40;

    SDL_Rect novoJ      = { (w - btnFinalW)/2, startY,                       btnFinalW, btnFinalH };
    SDL_Rect continuarJ = { (w - btnFinalW)/2, startY + btnFinalH + espacamento, btnFinalW, btnFinalH };
    SDL_Rect sairJ      = { (w - btnFinalW)/2, startY + 2*(btnFinalH + espacamento), btnFinalW, btnFinalH };

    SDL_Rect n = {315,0,315,35};
    SDL_Rect c = {0,0,315,35};
    SDL_Rect s = {0,0,315,35};

    int espera = 16;
    int selecionadoN = 1, selecionadoC = 0, selecionadoS = 0;
    int rodando = 1;

    Mix_Music* musica = Mix_LoadMUS("./src/msc/pontef.ogg");
    if (!musica) {
        printf("Erro ao carregar música: %s\n", Mix_GetError());
    }
    
    cursorMenu = SDL_CreateColorCursor(cursorSurface, 0, 0);
    SDL_SetCursor(cursorMenu);
    SDL_FreeSurface(cursorSurface);

	Mix_PlayMusic(musica, -1);
    while (rodando && !SDL_QuitRequested()) {
        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, fundo ,NULL, NULL);
        SDL_RenderCopy(ren, logo ,NULL, &titulo);
        SDL_RenderCopy(ren, novo ,&n, &novoJ);
        SDL_RenderCopy(ren, continuar ,&c, &continuarJ);
        SDL_RenderCopy(ren, sair ,&s, &sairJ);
        SDL_RenderPresent(ren);


        SDL_Event evt;
        int isevt = SDL_WaitEventTimeout(&evt, espera);
        if (isevt && evt.type == SDL_QUIT) break;

        if (isevt) {
            switch (evt.type) {
                case SDL_KEYDOWN:
                    switch(evt.key.keysym.scancode) {
                        case SDL_SCANCODE_DOWN:
                        case SDL_SCANCODE_UP:
                            if (selecionadoN == 1) {
                                n = (SDL_Rect){0,0,315,35};
                                s = (SDL_Rect){315,0,315,35};
                                selecionadoN=0; selecionadoS=1; selecionadoC=0;
                            } else if (selecionadoS == 1) {
                                s = (SDL_Rect){0,0,315,35};
                                n = (SDL_Rect){315,0,315,35};
                                selecionadoN=1; selecionadoS=0; selecionadoC=0;
                            }
                            break;

                        case SDL_SCANCODE_RETURN:
                        case SDL_SCANCODE_Z:
                            if(selecionadoN == 1){
                                runGame(win, ren);
                            } else if(selecionadoS == 1){
                                rodando = 0;
                            }
                            break;
                    }
                    break;

                case SDL_MOUSEMOTION: {
                    int mx = evt.motion.x;
                    int my = evt.motion.y;
                    if (mx >= novoJ.x && mx <= novoJ.x+novoJ.w &&
                        my >= novoJ.y && my <= novoJ.y+novoJ.h) {
                        n = (SDL_Rect){315,0,315,35};
                        c = (SDL_Rect){0,0,315,35};
                        s = (SDL_Rect){0,0,315,35};
                        selecionadoN=1; selecionadoC=0; selecionadoS=0;
                    } else if (mx >= sairJ.x && mx <= sairJ.x+sairJ.w &&
                               my >= sairJ.y && my <= sairJ.y+sairJ.h) {
                        s = (SDL_Rect){315,0,315,35};
                        n = (SDL_Rect){0,0,315,35};
                        c = (SDL_Rect){0,0,315,35};
                        selecionadoN=0; selecionadoC=0; selecionadoS=1;
                    }
                    break;
                }

                case SDL_MOUSEBUTTONDOWN:
                    if (evt.button.button == SDL_BUTTON_LEFT) {
                        if (selecionadoN == 1) {
                        	SDL_ShowCursor(SDL_DISABLE);
                            runGame(win, ren);
                            SDL_ShowCursor(SDL_ENABLE);
                            SDL_SetCursor(cursorMenu);
                        } else if (selecionadoC == 1) {
                            // continuar jogo (não implementado)
                        } else if (selecionadoS == 1) {
                            rodando = 0;
                        }
                    }
                    break;
            }
        }
    }

    Mix_FreeMusic(musica);
    Mix_CloseAudio();
    SDL_DestroyTexture(sair);
    SDL_DestroyTexture(continuar);
    SDL_DestroyTexture(novo);
    SDL_DestroyTexture(logo);
    SDL_DestroyTexture(fundo);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_FreeCursor(cursorMenu);
    SDL_Quit();
}
