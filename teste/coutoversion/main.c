#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <SDL2/SDL_mixer.h>

void desenharParalax(SDL_Renderer* ren, SDL_Texture* tex, float fatorParalax, int cameraX, int posY) {
    int texW, texH;
    SDL_QueryTexture(tex, NULL, NULL, &texW, &texH);
    
    texW = texW*0.6;
    texH = texH*0.6;

    // Calcula deslocamento com paralax
    int offsetX = -(int)(cameraX / fatorParalax) % texW;
    if (offsetX > 0) offsetX -= texW;

    // Repete a textura horizontalmente no tamanho original
    for (int x = offsetX; x < 1920; x += texW) {  // 1920 = largura mínima da tela. Pode usar w se quiser
        SDL_Rect dest = { x, posY, texW, texH };
        SDL_RenderCopy(ren, tex, NULL, &dest);
    }
}

void runGame(SDL_Window* win, SDL_Renderer* ren) {
    SDL_Texture* sprites = IMG_LoadTexture(ren, "./src/entidades/ss.png");
    SDL_Texture* ponte   = IMG_LoadTexture(ren, "./src/mapa/ponteportoes.png");
    SDL_Texture* fundo   = IMG_LoadTexture(ren, "./src/mapa/bg+lua.png");
    SDL_Texture* hud     = IMG_LoadTexture(ren, "./src/mapa/hud.png");
    SDL_Texture* parafu  = IMG_LoadTexture(ren, "./src/mapa/paralax fundo.png");
    SDL_Texture* parafr  = IMG_LoadTexture(ren, "./src/mapa/paralax frente.png");
    SDL_Texture* portao  = IMG_LoadTexture(ren, "./src/mapa/portão.png");
    
    assert(sprites && ponte && fundo && hud && parafu && parafr && portao);

    SDL_ShowCursor(SDL_DISABLE);

    int w, h;
    SDL_GetWindowSize(win, &w, &h);

    SDL_Rect camera = {0, 0, w, h};
    SDL_Rect vida = {0, 0, 384, 126};
    SDL_Rect player = { w/5, (h - ((15*h)/100)/3) - 100, 110, 100 };
    SDL_Rect ponteR = {0, h - ((100*h)/100), 4200, (100*h)/100};
    SDL_Rect chaoR  = {0, (h - ((15*h)/100)/3)-10, 2000, ((15*h)/100)/3};
    SDL_Rect f      = {0, 0, 230, 210};
    SDL_Rect porta = {0,h-400-((h*7)/100), 115, 400};

    // VARIÁVEIS DO JOGADOR
    int espera = 16;
    int vely = 0;
    int gravidade = 1;
    int puloInicial = -18;
    int noChao = 1;

    int direita = 1, esquerda = -1, frameAtual = direita;
    int virando = 0, frameVirada = 0;
    int frameID = 0, frameIE = 0;
    int pulando = 0, framePulo = 0;
    int intervaloFrame = 120;
    Uint32 ultimoFrameTroca = 0;

    while (!SDL_QuitRequested()) {
        SDL_Event evt;
        int isevt = SDL_WaitEventTimeout(&evt, espera);
        if (isevt && evt.type == SDL_QUIT) break;

        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        Uint32 agora = SDL_GetTicks();

        int movendo = 0;

        // PULO
        if (keys[SDL_SCANCODE_Z] && noChao) {
            vely = puloInicial;
            noChao = 0;
            pulando = 1;
            framePulo = 0;
            ultimoFrameTroca = agora;
        }

        // MOVIMENTO ESQUERDA
        if (keys[SDL_SCANCODE_LEFT]) {
            movendo = 1;
            if (frameAtual == direita && !virando && noChao) {
                virando = 1;
                frameVirada = 0;
                ultimoFrameTroca = agora;
            }
            if(player.x>62){
            player.x -= 13;
            }

            if (noChao) {
                if (virando == 1) {
                    if (agora - ultimoFrameTroca > intervaloFrame) {
                        ultimoFrameTroca = agora;
                        frameVirada++;
                        if (frameVirada > 2) { virando = 0; frameIE = 0; frameAtual = esquerda; }
                    }
                    f = (SDL_Rect){230 * frameVirada, 210 * 1, 230, 210};
                } else {
                    frameAtual = esquerda;
                    if (agora - ultimoFrameTroca > intervaloFrame) {
                        ultimoFrameTroca = agora;
                        frameIE++;
                        if (frameIE > 3) frameIE = 0;
                    }
                    f = (SDL_Rect){230 * frameIE, 210 * 2, 230, 210};
                }
            }
        }

        // MOVIMENTO DIREITA
        if (keys[SDL_SCANCODE_RIGHT]) {
            movendo = 1;
            if (frameAtual == esquerda && !virando && noChao) {
                virando = 2;
                frameVirada = 0;
                ultimoFrameTroca = agora;
            }
            player.x += 13;

            if (noChao) {
                if (virando == 2) {
                    if (agora - ultimoFrameTroca > intervaloFrame) {
                        ultimoFrameTroca = agora;
                        frameVirada++;
                        if (frameVirada > 2) { virando = 0; frameID = 0; frameAtual = direita; }
                    }
                    f = (SDL_Rect){230 * frameVirada, 210 * 3, 230, 210};
                } else {
                    frameAtual = direita;
                    if (agora - ultimoFrameTroca > intervaloFrame) {
                        ultimoFrameTroca = agora;
                        frameID++;
                        if (frameID > 3) frameID = 0;
                    }
                    f = (SDL_Rect){230 * frameID, 0, 230, 210};
                }
            }
        }

        // Troca de direção no ar
        if (!noChao && movendo) {
            if (keys[SDL_SCANCODE_RIGHT] && frameAtual != direita) frameAtual = direita;
            else if (keys[SDL_SCANCODE_LEFT] && frameAtual != esquerda) frameAtual = esquerda;
        }

        // ANIMAÇÃO DE PULO
        if (!noChao) {
            int linhaPulo = (frameAtual == direita) ? 4 : 5;
            if (vely < 0) {
                if (agora - ultimoFrameTroca > intervaloFrame) {
                    ultimoFrameTroca = agora;
                    framePulo++;
                    if (framePulo > 2) framePulo = 2;
                }
            } else framePulo = 3;
            f = (SDL_Rect){230 * framePulo, 210 * linhaPulo, 230, 210};
        } else if (!movendo && !virando) {
            if (frameAtual == direita) f = (SDL_Rect){0, 0, 230, 210};
            else f = (SDL_Rect){0, 210*2, 230, 210};
        }

        if (!noChao && virando != 0) virando = 0;

        // FÍSICA
        player.y += vely;
        vely += gravidade;
        noChao = 0;

        // Colisão com chão
        if (player.y + player.h >= chaoR.y) {
            player.y = chaoR.y - player.h;
            vely = 0;
            noChao = 1;
            pulando = 0;
        }

        // CÂMERA
        camera.x = player.x + player.w / 2 - w / 2;
        if (camera.x < 0) camera.x = 0;
        if (camera.x > 4200 - w) camera.x = 4200 - w;

        // RENDER
        SDL_RenderClear(ren);

        SDL_Rect fundoR = {(w-2080)/2, ((h-1040)/2)+(5*h)/100, 2080, 1040};

        SDL_RenderCopy(ren, fundo, NULL, &fundoR);        
        desenharParalax(ren, parafu, 3.0f, camera.x, 20); // fundo
        desenharParalax(ren, parafr, 1.5f, camera.x, 20); // frente

        SDL_Rect portaMundo = {0, h - 400 - ((h * 7) / 100), 115, 400}; // posição no mundo
        SDL_Rect portaTela = portaMundo;
        portaTela.x -= camera.x; // ajusta para a câmera
        SDL_RenderCopy(ren, portao, NULL, &portaTela);
        

        // Jogador
        SDL_Rect playerScreen = player;
        playerScreen.x -= camera.x;
        SDL_RenderCopy(ren, sprites, &f, &playerScreen);

        // Ponte na frente do jogador
        SDL_Rect ponteScreen = ponteR;
        ponteScreen.x -= camera.x;
        SDL_RenderCopy(ren, ponte, NULL, &ponteScreen);

        // HUD
        SDL_RenderCopy(ren, hud, NULL, &vida);

        SDL_RenderPresent(ren);
    }

    // LIMPEZA
    SDL_DestroyTexture(fundo);
    SDL_DestroyTexture(sprites);
    SDL_DestroyTexture(ponte);
    SDL_DestroyTexture(parafu);
    SDL_DestroyTexture(parafr);
    SDL_DestroyTexture(hud);
}


int main(int argc, char* args[]) {
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);
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
    
    Mix_Music* musica = Mix_LoadMUS("./src/msc/musica.ogg");
  if (!musica) {
    printf("Erro ao carregar música: %s\n", Mix_GetError());
  }
    //Mix_PlayMusic(musica, -1);
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
                        }
                        else if (selecionadoS == 1) {
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

                // Novo jogo
                if (mx >= novoJ.x && mx <= novoJ.x+novoJ.w &&
                    my >= novoJ.y && my <= novoJ.y+novoJ.h) {
                    n = (SDL_Rect){315,0,315,35};
                    c = (SDL_Rect){0,0,315,35};
                    s = (SDL_Rect){0,0,315,35};
                    selecionadoN=1; selecionadoC=0; selecionadoS=0;
                }

                // Continuar

                // Sair
                else if (mx >= sairJ.x && mx <= sairJ.x+sairJ.w &&
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
                        runGame(win, ren);
                    } else if (selecionadoC == 1) {
                        // continuar jogo (ainda não implementado)
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
