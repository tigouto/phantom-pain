#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include "cenario.h"
#include "player.h"
#include "pedinte.h"

/* ----------------- FUNÇÃO PRINCIPAL DO JOGO (runGame) ----------------- */

int jogoPausado = 0;
int opcaoPause = 0;
int podeUsarEsc = 1;

void desenharTexto(SDL_Renderer* renderer, const char* texto, int x, int y, SDL_Color cor, TTF_Font* fonte){
    // Renderiza o texto em surface
    SDL_Surface* surface = TTF_RenderUTF8_Blended(fonte, texto, cor);
    if (!surface) {
        printf("Erro na TTF_RenderUTF8_Blended: %s\n", TTF_GetError());
        return;
    }

    // Transforma em textura
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("Erro na SDL_CreateTextureFromSurface: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }

    // Pega tamanho do texto
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    dst.w = surface->w;
    dst.h = surface->h;

    // Renderiza
    SDL_RenderCopy(renderer, texture, NULL, &dst);

    // Libera
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void renderMenuPause(SDL_Renderer* ren, int w, int h, TTF_Font* fontePadrao){
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren,0,0,0,150);
    SDL_Rect tela = {0,0,w,h};
    SDL_RenderFillRect(ren, &tela);
	SDL_Color branco = {255, 255, 255, 255};
	SDL_Color cinza = {180, 180, 180, 255};
	SDL_Color amarelo = {255, 255, 0, 255};
	
	desenharTexto(ren,"PAUSADO", 500, 200, branco, fontePadrao);

	if (opcaoPause == 0)
	    desenharTexto(ren,"X Retornar", 500, 400, amarelo, fontePadrao);
	else
	    desenharTexto(ren,"Retornar", 500, 400, cinza, fontePadrao);

    if(opcaoPause == 1){
    	desenharTexto(ren,"X Sair para o menu",500,500,amarelo,fontePadrao);
	}else{
		desenharTexto(ren,"Sair para o menu",500,500,cinza,fontePadrao);
	}
}

void runGame(SDL_Window* win, SDL_Renderer* ren, TTF_Font* fontePadrao){
	int w, h;
	SDL_GetWindowSize(win, &w, &h);
	
	SDL_Rect camera = {0,0,w,h};
	int atual = 0;
    // --- Texturas básicas (sprites, HUD, pedinte, flor) ---
    SDL_Texture* sprites = IMG_LoadTexture(ren, "./src/entidades/ss reaper.png");
    SDL_Texture* hud = IMG_LoadTexture(ren, "./src/mapa/hud.png");
    SDL_Texture* texPedinte = IMG_LoadTexture(ren, "./src/entidades/ss pedinte.png");
    SDL_Texture* texFlor = IMG_LoadTexture(ren, "./src/mapa/ss flor.png"); // novo

    assert(sprites && hud && texPedinte && texFlor);

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
    SDL_Texture* borda_sala = IMG_LoadTexture(ren, "./src/mapa/sala-p/borda.png");
    SDL_Texture* atras_sala = IMG_LoadTexture(ren, "./src/mapa/sala-p/fundo-atras.png");
    SDL_Texture* frente_sala = IMG_LoadTexture(ren, "./src/mapa/sala-p/fundo-frente.png");

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
	
	addAtras(&cenarios[1], fundo_sala, (SDL_Rect){0, 0, w, h});
	addAtras(&cenarios[1], atras_sala, (SDL_Rect){0, 0, w, h});
	addAtras(&cenarios[1], frente_sala, (SDL_Rect){0, 0, w, h});
	addFrente(&cenarios[1], borda_sala, (SDL_Rect){0, 0, w, h});

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

	// NPCs
    addPedinte(&cenarios[0], 3*w/5, chaoR.y-100, 110, 100);

    // render inicial + fade in
    SDL_RenderClear(ren);
    desenharCenario(ren, &cenarios[atual], camera, w, h,w,h);
    SDL_RenderCopy(ren, sprites, &player.frameRect, &(SDL_Rect){player.pos.x - camera.x, player.pos.y, player.pos.w, player.pos.h});
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

        if (!jogoPausado){
        	
        	if (isevt && evt.type == SDL_KEYDOWN && evt.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
	            if(!jogoPausado && podeUsarEsc){
	                jogoPausado = 1;
	                opcaoPause = 0;
	                podeUsarEsc = 0;
	            }
	        }
        	
            updatePlayer(&player, keys, agora, chaoR, &vida, cenarios, atual, &camera, ren);

            // Atualiza pedintes
            for (int i = 0; i < cenarios[atual].numPedintes; i++){
            	updatePedinte(&cenarios[atual].pedintes[i], player.pos, agora);
		    }
		    
		    // Atualiza checkpoints
		    float deltaTime = 16.0f; // o tempo decorrido entre frames

		    updateElementos(&cenarios[atual], player.pos, keys, deltaTime, &vida.vidas);

            // Invulnerabilidade timeout
            if (vida.invulneravel && agora - vida.tempoInvulneravel > TEMPO_INVULNERAVEL) {
                vida.invulneravel = 0;
            }

            // câmera
            camera.x = player.pos.x + player.pos.w/2 - w/2;
            if (camera.x < cenarios[atual].posX) camera.x = cenarios[atual].posX;
            if (camera.x > cenarios[atual].posX + cenarios[atual].largura - w)
                camera.x = cenarios[atual].posX + cenarios[atual].largura - w;

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
            if (player.pos.x + player.pos.w < cenarios[atual].posX && atual > 0) {
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
        renderPlayer(&player, ren, sprites, camera);

        // desenha frente cenário
        desenharFrente(ren, &cenarios[atual], camera);

        // HUD: desenha hud base
        SDL_RenderCopy(ren, hud, NULL, &vida.vidaRect);

		// Atualiza animação da perda da flor
		updateFlor(&vida, agora);
		
		renderHudFlores(ren, texFlor, vida, w, vida.vidaRect);

        if(jogoPausado){
            renderMenuPause(ren,w,h,fontePadrao);
        
            if (isevt && evt.type == SDL_KEYDOWN) {	
            	if (podeUsarEsc && evt.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
	            	jogoPausado = 0;
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
                            jogoPausado = 0;
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
    SDL_DestroyTexture(sprites);
    SDL_DestroyTexture(texPedinte);
    SDL_DestroyTexture(texFlor);
    SDL_DestroyTexture(texCheckpoint);
    SDL_DestroyTexture(texAcampamento);


    SDL_ShowCursor(SDL_ENABLE);
}

/* ----------------- FUNÇÃO main (menu) ----------------- */
int main(int argc, char* args[]) {
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
	TTF_Font* fontePadrao = TTF_OpenFont("./src/fonte/minhaFonte.TTF", 48);
	if (!fontePadrao) printf("Erro ao carregar fonte: %s\n", TTF_GetError());
	
    SDL_Window* win = SDL_CreateWindow("Phantom Pain v0.1", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                       0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, 0);

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

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
                                runGame(win, ren, fontePadrao);
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
                            runGame(win, ren, fontePadrao);
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
    SDL_Quit();
}
