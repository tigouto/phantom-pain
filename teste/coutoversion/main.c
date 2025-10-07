#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <assert.h>
#include <stdio.h>

#define MAX_PARALAX 8
#define MAX_ELEMENTOS 32

// ---------- SISTEMA DE CENÁRIOS E TRANSIÇÕES ----------

#define MAX_PARALAX 8
#define MAX_ELEMENTOS 32
#define MAX_CENARIOS 8

typedef struct {
    SDL_Texture* tex;
    float fator;
    int posY;
} Paralax;

typedef struct {
    SDL_Texture* tex;
    SDL_Rect pos;
} Elemento;

typedef struct {
    SDL_Texture* fundo;
    Paralax paralax[MAX_PARALAX];
    int numParalax;

    Elemento atras[MAX_ELEMENTOS];
    int numAtras;

    Elemento frente[MAX_ELEMENTOS];
    int numFrente;

    int posX;       // posição X do início do cenário no "mundo"
    int largura;    // largura total do cenário
    int altura;     // altura (padrão pode ser a altura da janela)
} Cenario;

// Desenha paralax (ajustado para cobrir horizontalmente com repetição)
static void desenharParalax(SDL_Renderer* ren, SDL_Texture* tex, float fatorParalax, int cameraX, int posY, int screenW) {
    if (!tex) return;
    int texW, texH;
    SDL_QueryTexture(tex, NULL, NULL, &texW, &texH);
    if (texW <= 0) return;

  
    int dstW = (int)(texW * 0.6f);
    int dstH = (int)(texH * 0.6f);
    if (dstW <= 0) dstW = texW;

    // offset calculado pelo fator
    int offsetX = -(int)(cameraX / fatorParalax) % dstW;
    if (offsetX > 0) offsetX -= dstW;

    for (int x = offsetX; x < screenW; x += dstW) {
        SDL_Rect dest = { x, posY, dstW, dstH };
        SDL_RenderCopy(ren, tex, NULL, &dest);
    }
}

// Inicializa estrutura do cenário
static void initCenario(Cenario* c) {
    c->fundo = NULL;
    c->numParalax = 0;
    c->numAtras = 0;
    c->numFrente = 0;
    c->posX = 0;
    c->largura = 4000;
    c->altura = 1080;
}

static void addParalax(Cenario* c, SDL_Texture* tex, float fator, int posY) {
    if (c->numParalax < MAX_PARALAX) {
        c->paralax[c->numParalax++] = (Paralax){tex, fator, posY};
    }
}

static void addAtras(Cenario* c, SDL_Texture* tex, SDL_Rect pos) {
    if (c->numAtras < MAX_ELEMENTOS) {
        c->atras[c->numAtras++] = (Elemento){tex, pos};
    }
}

static void addFrente(Cenario* c, SDL_Texture* tex, SDL_Rect pos) {
    if (c->numFrente < MAX_ELEMENTOS) {
        c->frente[c->numFrente++] = (Elemento){tex, pos};
    }
}

// Desenha um cenário (fundo, paralaxes, elementos atrás)
// camera.x é em coordenadas mundiais; o cenário usa posX para ajustar
static void desenharCenario(SDL_Renderer* ren, Cenario* c, SDL_Rect camera, int screenW, int screenH) {
    if (!c) return;

    // Fundo (posicionado da mesma forma do seu código original)
    if (c->fundo) {
        SDL_Rect fundoR = { (screenW - 2080) / 2, ((screenH - 1040) / 2) + (5 * screenH) / 100, 2080, 1040 };
        SDL_RenderCopy(ren, c->fundo, NULL, &fundoR);
    }

    // Paralaxes
    for (int i = 0; i < c->numParalax; ++i) {
        desenharParalax(ren, c->paralax[i].tex, c->paralax[i].fator, camera.x - c->posX, c->paralax[i].posY, screenW);
    }

    // Elementos atrás
    for (int i = 0; i < c->numAtras; ++i) {
        SDL_Rect dest = c->atras[i].pos;
        dest.x -= (camera.x - c->posX);
        SDL_RenderCopy(ren, c->atras[i].tex, NULL, &dest);
    }
}

// Desenha os elementos da frente (após desenhar o jogador)
static void desenharFrente(SDL_Renderer* ren, Cenario* c, SDL_Rect camera) {
    for (int i = 0; i < c->numFrente; ++i) {
        SDL_Rect dest = c->frente[i].pos;
        dest.x -= (camera.x - c->posX);
        SDL_RenderCopy(ren, c->frente[i].tex, NULL, &dest);
    }
}

// Fade usando retângulo preto com alpha (mais suave que loops com SDL_Delay grandes)
static void fade_out_in(SDL_Renderer* ren, int screenW, int screenH, int fadeOut) {
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    SDL_Rect r = {0, 0, screenW, screenH};
    if (fadeOut) {
        // fade out (0 -> 255)
        for (int a = 0; a <= 255; a += 12) {
            SDL_SetRenderDrawColor(ren, 0, 0, 0, a);
            SDL_RenderFillRect(ren, &r);
            SDL_RenderPresent(ren);
            SDL_Delay(8);
        }
    } else {
        // fade in (255 -> 0)
        for (int a = 255; a >= 0; a -= 12) {
            SDL_SetRenderDrawColor(ren, 0, 0, 0, a);
            SDL_RenderFillRect(ren, &r);
            SDL_RenderPresent(ren);
            SDL_Delay(8);
        }
    }
}


void runGame(SDL_Window* win, SDL_Renderer* ren) {
    // --- Carregamento de texturas utilizadas por vários cenários ---
    SDL_Texture* sprites = IMG_LoadTexture(ren, "./src/entidades/ss.png");
    SDL_Texture* hud     = IMG_LoadTexture(ren, "./src/mapa/hud.png");
    assert(sprites && hud);

    // Texturas de cenário (carregamos aqui e reaproveitamos nas áreas)
    SDL_Texture* fundo_tex     = IMG_LoadTexture(ren, "./src/mapa/bg+lua.png");
    SDL_Texture* parafu_tex    = IMG_LoadTexture(ren, "./src/mapa/paralax fundo.png");
    SDL_Texture* parafr_tex    = IMG_LoadTexture(ren, "./src/mapa/paralax frente.png");
    SDL_Texture* ponte_tex     = IMG_LoadTexture(ren, "./src/mapa/ponte.png");
    SDL_Texture* portao_tex    = IMG_LoadTexture(ren, "./src/mapa/portão.png");
    SDL_Texture* ponte_prox    = IMG_LoadTexture(ren, "./src/mapa/sala-port.png");   // textura extra exemplo (use se existir)
    SDL_Texture* arvore_tex    = IMG_LoadTexture(ren, "./src/mapa/arvore.png"); // pode falhar se arquivo não existir

    assert(fundo_tex && parafu_tex && parafr_tex && ponte_tex && portao_tex && ponte_prox);
    // arvore_tex é opcional, não assertamos

    int w, h;
    SDL_GetWindowSize(win, &w, &h);

    // --- Preparar cenários ---
    Cenario cenarios[MAX_CENARIOS];
    int totalCenarios = 0;
    // Cenário 0
    initCenario(&cenarios[0]);
    cenarios[0].fundo = fundo_tex;
    addParalax(&cenarios[0], parafu_tex, 3.0f, 20);
    addParalax(&cenarios[0], parafr_tex, 1.5f, 20);
    // ponte e portão (posições sugeridas; ajustei para coordenadas do mundo)
    SDL_Rect ponteR = { 0, h - ((100 * h) / 100), 4000, (100 * h) / 100 }; // mantive parecido ao seu original
    SDL_Rect portaR = { 0, h - 399 - ((h * 7) / 100), 115, 400 };
    SDL_Rect portaProxR = { 4000, 0, 100, h };
    
    addAtras(&cenarios[0], portao_tex, portaR);
    addAtras(&cenarios[0], ponte_prox, portaProxR);
    addFrente(&cenarios[0], ponte_tex, ponteR);
    cenarios[0].posX = 0;
    cenarios[0].largura = 4100;
    cenarios[0].altura = h;
    totalCenarios++;

    // Cenário 1 (exemplo lateral, começa imediatamente após o anterior)
    initCenario(&cenarios[1]);
    cenarios[1].fundo = fundo_tex;
    addParalax(&cenarios[1], parafu_tex, 3.0f, 20);
    addParalax(&cenarios[1], parafr_tex, 1.5f, 20);
    if (arvore_tex) {
        SDL_Rect arvR = { 500, h - 500, 200, 400 };
        addAtras(&cenarios[1], arvore_tex, arvR);
    }
    cenarios[1].posX = cenarios[0].posX + cenarios[0].largura; // encadeado à direita
    cenarios[1].largura = 4000;
    cenarios[1].altura = h;
    totalCenarios++;

    // (adicione mais cenários aqui ou faça carregar via JSON se quiser)

    // --- Variáveis do jogador (mantendo seu comportamento/animacao) ---
    SDL_ShowCursor(SDL_DISABLE);

    SDL_Rect vida = {0, 0, 384, 126};
    SDL_Rect player = { w/5, (h - ((15*h)/100)/3) - 100, 110, 100 };
    SDL_Rect chaoR  = {0, (h - ((15*h)/100)/3)-10, 2000, ((15*h)/100)/3};
    SDL_Rect f = {0, 0, 230, 210};

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

    SDL_Rect camera = {0, 0, w, h};

    // Cenário atual (começa no primeiro)
    int atual = 0;

    // Fade-in de entrada no primeiro cenário
    // Renderiza primeira frame do cenário antes do fade para efeito visual agradável
    SDL_RenderClear(ren);
    desenharCenario(ren, &cenarios[atual], camera, w, h);
    SDL_RenderCopy(ren, sprites, &f, &(SDL_Rect){player.x - camera.x, player.y, player.w, player.h});
    SDL_RenderPresent(ren);
    fade_out_in(ren, w, h, 0); // fade in (255->0) para aparecer

   
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
            
        if(atual != 0 || player.x > 55){
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

        int chaoY = chaoR.y; // mantém seu chão relativo à janela
        if (player.y + player.h >= chaoY) {
            player.y = chaoY - player.h;
            vely = 0;
            noChao = 1;
            pulando = 0;
        }

        // CÂMERA (mantendo dentro dos limites do cenário atual)
        camera.x = player.x + player.w / 2 - w / 2;
        if (camera.x < cenarios[atual].posX) camera.x = cenarios[atual].posX;
        if (camera.x > cenarios[atual].posX + cenarios[atual].largura - w)
            camera.x = cenarios[atual].posX + cenarios[atual].largura - w;

        // --- TRANSIÇÃO ENTRE CENÁRIOS ---
        // Saiu para a direita do cenário atual e existe um próximo
        if (player.x > cenarios[atual].posX + cenarios[atual].largura && (atual + 1) < totalCenarios) {
            // fade out
            fade_out_in(ren, w, h, 1);
            // avançar cenário
            atual++;
            // posicionar player na borda esquerda da nova área (com margem)
            player.x = cenarios[atual].posX + 50;
            // ajustar câmera para nova área
            camera.x = player.x + player.w / 2 - w / 2;
            if (camera.x < cenarios[atual].posX) camera.x = cenarios[atual].posX;
            if (camera.x > cenarios[atual].posX + cenarios[atual].largura - w)
                camera.x = cenarios[atual].posX + cenarios[atual].largura - w;
            // fade in
            fade_out_in(ren, w, h, 0);
        }

        // Saiu para a esquerda do cenário atual e existe anterior
        if (player.x + player.w < cenarios[atual].posX && atual > 0) {
            fade_out_in(ren, w, h, 1);
            atual--;
            // posicionar player na borda direita da nova área
            player.x = cenarios[atual].posX + cenarios[atual].largura - 120;
            camera.x = player.x + player.w / 2 - w / 2;
            if (camera.x < cenarios[atual].posX) camera.x = cenarios[atual].posX;
            if (camera.x > cenarios[atual].posX + cenarios[atual].largura - w)
                camera.x = cenarios[atual].posX + cenarios[atual].largura - w;
            fade_out_in(ren, w, h, 0);
        }

        // RENDER
        SDL_RenderClear(ren);

        // Desenha cenário atual (fundo, paralax, elementos atrás)
        desenharCenario(ren, &cenarios[atual], camera, w, h);

        // Jogador (mesmo sprite/recorte que você tinha)
        SDL_Rect playerScreen = player;
        playerScreen.x -= camera.x;
        SDL_RenderCopy(ren, sprites, &f, &playerScreen);

        // Elementos da frente do cenário atual (por cima do jogador)
        desenharFrente(ren, &cenarios[atual], camera);

        // HUD
        SDL_RenderCopy(ren, hud, NULL, &vida);

        SDL_RenderPresent(ren);
    }

    // --- LIMPEZA: destruímos texturas que carregamos aqui ---
    // Observação: cenários re-apontam para essas texturas, mas como destruímos as texturas
    // apenas uma vez aqui, não há double-free.
    if (fundo_tex) SDL_DestroyTexture(fundo_tex);
    if (parafu_tex) SDL_DestroyTexture(parafu_tex);
    if (parafr_tex) SDL_DestroyTexture(parafr_tex);
    if (ponte_tex) SDL_DestroyTexture(ponte_tex);
    if (portao_tex) SDL_DestroyTexture(portao_tex);
    if (arvore_tex) SDL_DestroyTexture(arvore_tex);

    SDL_DestroyTexture(hud);
    SDL_DestroyTexture(sprites);

    // esconder cursor novamente caso prefira
    SDL_ShowCursor(SDL_ENABLE);
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
