#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

/* ----------------- CONSTANTES GLOBAIS ----------------- */
#define MAX_PARALAX 8
#define MAX_ELEMENTOS 32
#define MAX_CENARIOS 8

#define RECUO_PEDINTE 100
#define TEMPO_INVULNERAVEL 1000 // ms

/* ----------------- ESTRUTURAS ----------------- */

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
static void desenharParalax(SDL_Renderer* ren, SDL_Texture* tex, float fatorParalax, int cameraX, int posY, int screenW,int w,int h) {
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

/* Cenario utilitários */
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
    if (c->numParalax < MAX_PARALAX) c->paralax[c->numParalax++] = (Paralax){tex, fator, posY};
}
static void addAtras(Cenario* c, SDL_Texture* tex, SDL_Rect pos) {
    if (c->numAtras < MAX_ELEMENTOS) c->atras[c->numAtras++] = (Elemento){tex, pos};
}
static void addFrente(Cenario* c, SDL_Texture* tex, SDL_Rect pos) {
    if (c->numFrente < MAX_ELEMENTOS) c->frente[c->numFrente++] = (Elemento){tex, pos};
}

static void desenharCenario(SDL_Renderer* ren, Cenario* c, SDL_Rect camera, int screenW, int screenH,int w,int h) {
    if (!c) return;

    // Fundo (posicionado da mesma forma do seu código original)
    if (c->fundo) {
        SDL_Rect fundoR = { (screenW - 3120) / 2, ((screenH - 1560) / 2) + (5 * screenH) / 100, 3120, 1560 };
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

static void desenharFrente(SDL_Renderer* ren, Cenario* c, SDL_Rect camera) {
    for (int i = 0; i < c->numFrente; ++i) {
        SDL_Rect dest = c->frente[i].pos;
        dest.x -= (camera.x - c->posX);
        SDL_RenderCopy(ren, c->frente[i].tex, NULL, &dest);
    }
}

/* Fade */
static void fade_out_in(SDL_Renderer* ren, int screenW, int screenH, int fadeOut) {
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

/* ----------------- NPC: PEDINTE ----------------- */
enum Direcao { ESQUERDA = -1, DIREITA = 1 };
enum EstadoInimigo { INIMIGO_PARADO, INIMIGO_LEVANTANDO, INIMIGO_ANDANDO, INIMIGO_PATRULHANDO };

typedef struct {
    SDL_Rect pos;           // posição e tamanho do sprite no mundo
    SDL_Rect frameRect;     // região da spritesheet
    enum Direcao dir;
    enum EstadoInimigo estado;

    Uint32 ultimoFrame;
    int frameAtual;
    int intervaloFrame;

    int distanciaVisao;
    int limiteEsq, limiteDir;  // área de patrulha
} Pedinte;

static void initPedinte(Pedinte* p, int x, int y, int w, int h) {
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
}

static void updatePedinte(Pedinte* p, SDL_Rect playerRect, Uint32 agora) {
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
    } else if (p->estado == INIMIGO_ANDANDO) {
        p->dir = (playerRect.x < p->pos.x) ? ESQUERDA : DIREITA;
        p->pos.x += (p->dir == DIREITA) ? 2 : -2;

        if (agora - p->ultimoFrame > p->intervaloFrame) {
            p->ultimoFrame = agora;
            p->frameAtual++;
            if (p->frameAtual > 6) p->frameAtual = 0;
        }
        int linha = (p->dir == DIREITA) ? 2 : 1;
        p->frameRect = (SDL_Rect){210 * p->frameAtual, 170 * linha, 210, 170};
    } else if (p->estado == INIMIGO_PATRULHANDO) {
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
    } else if (p->estado == INIMIGO_PARADO) {
        p->frameAtual = 0;
        p->frameRect = (SDL_Rect){0,0,210,170};
    }
}

static void renderPedinte(SDL_Renderer* ren, SDL_Texture* tex, Pedinte* p, SDL_Rect camera) {
    SDL_Rect dest = p->pos;
    dest.x -= camera.x;
    SDL_RenderCopy(ren, tex, &p->frameRect, &dest);
}

/* ----------------- FUNÇÃO PRINCIPAL DO JOGO (runGame) ----------------- */
void runGame(SDL_Window* win, SDL_Renderer* ren) {
    // --- Texturas básicas (sprites, HUD, pedinte, flor) ---
    SDL_Texture* sprites = IMG_LoadTexture(ren, "./src/entidades/ss.png");
    SDL_Texture* hud     = IMG_LoadTexture(ren, "./src/mapa/hud.png");
    SDL_Texture* texPedinte = IMG_LoadTexture(ren, "./src/entidades/ss pedinte.png");
    SDL_Texture* texFlor = IMG_LoadTexture(ren, "./src/entidades/ss flor.png"); // novo

    assert(sprites && hud && texPedinte && texFlor);

    // Texturas de cenário
    SDL_Texture* fundo_tex  = IMG_LoadTexture(ren, "./src/mapa/ponte-f/bg+lua.png");
    SDL_Texture* parafu_tex = IMG_LoadTexture(ren, "./src/mapa/ponte-f/paralax fundo.png");
    SDL_Texture* parafr_tex = IMG_LoadTexture(ren, "./src/mapa/ponte-f/paralax frente.png");
    SDL_Texture* ponte_tex  = IMG_LoadTexture(ren, "./src/mapa/ponte-f/ponte.png");
    SDL_Texture* portao_tex = IMG_LoadTexture(ren, "./src/mapa/ponte-f/portão.png");
    SDL_Texture* ponte_prox = IMG_LoadTexture(ren, "./src/mapa/ponte-f/sala-port.png");

    // sala textures (exemplo)
    SDL_Texture* fundo_sala = IMG_LoadTexture(ren, "./src/mapa/sala-p/background.png");
    SDL_Texture* borda_sala = IMG_LoadTexture(ren, "./src/mapa/sala-p/borda.png");
    SDL_Texture* atras_sala = IMG_LoadTexture(ren, "./src/mapa/sala-p/fundo-atras.png");
    SDL_Texture* frente_sala = IMG_LoadTexture(ren, "./src/mapa/sala-p/fundo-frente.png");

    int w, h;
    SDL_GetWindowSize(win, &w, &h);

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
    cenarios[1].posX = cenarios[0].posX + cenarios[0].largura;
    cenarios[1].largura = 1155;
    cenarios[1].altura = h;
    
	addAtras(&cenarios[1], fundo_sala, (SDL_Rect){-125,0,w,h});
    addAtras(&cenarios[1], atras_sala, (SDL_Rect){-125,0,w,h});
    addAtras(&cenarios[1], frente_sala, (SDL_Rect){-125,0,w,h});
    addFrente(&cenarios[1], borda_sala, (SDL_Rect){-125,0,w,h});
    
	totalCenarios++;

    // --- Jogador / HUD / Física / Ataque (integração do teste.c) ---
    SDL_ShowCursor(SDL_DISABLE);

    SDL_Rect vidaRect = {0, 0, w/5, h/10};
    SDL_Rect player = { w/5, (h - ((15*h)/100)/3) - 100 + 5, 110, 100 };
    SDL_Rect chaoR = { 0, (h - ((15*h)/100)/3)-15, w, ((15*h)/100)/3 };

    // Plataformas (do teste.c)
    int numPlataformas = 0;
    SDL_Rect plataformas[2] = {
        { 200, h - 200, 150, 20 },
        { 400, h - 300, 150, 20 },
    };

    // HUD flores
    int vidas = 3;
    int animandoFlor = -1;
    int frameFlorMorrendo = 0;
    Uint32 ultimoFrameFlor = 0;
    int intervaloFrameFlor = 120;
    int totalFramesFlor = 13;
    int florAltura = 210;
    int invulneravel = 0;
    Uint32 tempoInvulneravel = 0;

    // animação player / física
    SDL_Rect f = { 0, 0, 230, 210 };
    int vely = 0, gravidade = 1, puloInicial = -18, noChao = 1;
    enum Direcao dirPlayer = DIREITA;
    int virando = 0, frameVirada = 0, frameID = 0, frameIE = 0;
    Uint32 ultimoFrameTroca = 0;
    int intervaloFrame = 120;

    // ataque player
    int atacando = 0;
    Uint32 tempoAtaque = 0;
    Uint32 duracaoHitbox = 300;
    Uint32 intervaloEntreAtaques = 800;
    SDL_Rect hitboxPlayer = {0,0,0,0};
    int frameAtaque = 0;
    Uint32 ultimoFrameAtaque = 0;
    int intervaloFrameAtaque = 100;
    int totalFramesAtaque = 4;

    // NPCs
    Pedinte pedintes[8];
    int numPedintes = 0;
    initPedinte(&pedintes[numPedintes++], 3*w/5, (h - ((15*h)/100)/3) - 100 -20, 110, 100);

    // câmera e estado de cenário
    SDL_Rect camera = {0, 0, w, h};
    int atual = 0;

    // render inicial + fade in
    SDL_RenderClear(ren);
    desenharCenario(ren, &cenarios[atual], camera, w, h,w,h);
    SDL_RenderCopy(ren, sprites, &f, &(SDL_Rect){player.x - camera.x, player.y, player.w, player.h});
    SDL_RenderPresent(ren);
    fade_out_in(ren, w, h, 0);

    int espera = 16;
    while (!SDL_QuitRequested()) {
        SDL_Event evt;
        int isevt = SDL_WaitEventTimeout(&evt, espera);
        if (isevt && evt.type == SDL_QUIT) break;

        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        Uint32 agora = SDL_GetTicks();

        // Atualiza pedintes
        for (int i = 0; i < numPedintes; i++) updatePedinte(&pedintes[i], player, agora);

        // Animação flor morrendo
        if (animandoFlor >= 0 && agora - ultimoFrameFlor > intervaloFrameFlor) {
            ultimoFrameFlor = agora;
            frameFlorMorrendo++;
            if (frameFlorMorrendo >= totalFramesFlor) {
                vidas--;
                animandoFlor = -1;
                frameFlorMorrendo = 0;
            }
        }

        // Invulnerabilidade timeout
        if (invulneravel && agora - tempoInvulneravel > TEMPO_INVULNERAVEL) {
            invulneravel = 0;
        }

        // --- ATAQUE ---
        if (keys[SDL_SCANCODE_X]) {
            if (!atacando && agora - tempoAtaque > intervaloEntreAtaques && vidas > 0) {
                atacando = 1;
                tempoAtaque = agora;
                frameAtaque = 0;
                ultimoFrameAtaque = tempoAtaque;
            }
        }

        if (atacando) {
            if (agora - ultimoFrameAtaque > intervaloFrameAtaque) {
                ultimoFrameAtaque = agora;
                frameAtaque++;
                if (frameAtaque >= totalFramesAtaque) atacando = 0;
            }
            // hitbox depende da direção
            if (dirPlayer == DIREITA)
                hitboxPlayer = (SDL_Rect){player.x + player.w, player.y + 30, 60, 40};
            else
                hitboxPlayer = (SDL_Rect){player.x - 60, player.y + 30, 60, 40};
        } else {
            hitboxPlayer = (SDL_Rect){0,0,0,0};
        }

        // --- MOVIMENTO PLAYER (somente se ainda tiver vidas) ---
        int movendo = 0;
        if (vidas > 0) {
            // esquerda
            if (keys[SDL_SCANCODE_LEFT]) {
                movendo = 1;
                if (dirPlayer == DIREITA && virando != 1) {
                    virando = 1; frameVirada = 0; ultimoFrameTroca = agora;
                }
                if (atual != 0 || player.x > 55) player.x -= 13;
                dirPlayer = ESQUERDA;
                if (virando == 0 && !atacando) {
                    if (agora - ultimoFrameTroca > intervaloFrame) {
                        ultimoFrameTroca = agora;
                        frameIE++;
                        if (frameIE > 3) frameIE = 0;
                    }
                    f = (SDL_Rect){230 * frameIE, 210 * 2, 230, 210};
                }
            }
            // direita
            else if (keys[SDL_SCANCODE_RIGHT]) {
                movendo = 1;
                if (dirPlayer == ESQUERDA && virando != 2) {
                    virando = 2; frameVirada = 0; ultimoFrameTroca = agora;
                }
                player.x += 13;
                dirPlayer = DIREITA;
                if (virando == 0 && !atacando) {
                    if (agora - ultimoFrameTroca > intervaloFrame) {
                        ultimoFrameTroca = agora;
                        frameID++;
                        if (frameID > 3) frameID = 0;
                    }
                    f = (SDL_Rect){230 * frameID, 0, 230, 210};
                }
            }

            // pulo
            if (keys[SDL_SCANCODE_Z] && noChao) {
                vely = puloInicial;
                noChao = 0;
            }
        }

        // animação de virada (continua automaticamente)
        if (virando == 1) {
            if (agora - ultimoFrameTroca > intervaloFrame) {
                ultimoFrameTroca = agora;
                frameVirada++;
                if (frameVirada > 2) { virando = 0; frameIE = 0; dirPlayer = ESQUERDA; }
                else f = (SDL_Rect){230 * frameVirada, 210 * 1, 230, 210};
            }
        } else if (virando == 2) {
            if (agora - ultimoFrameTroca > intervaloFrame) {
                ultimoFrameTroca = agora;
                frameVirada++;
                if (frameVirada > 2) { virando = 0; frameID = 0; dirPlayer = DIREITA; }
                else f = (SDL_Rect){230 * frameVirada, 210 * 3, 230, 210};
            }
        }

        if (!movendo && !virando && !atacando && vidas > 0) {
            if (dirPlayer == DIREITA) f = (SDL_Rect){0,0,230,210};
            else f = (SDL_Rect){0, 210 * 2, 230, 210};
        }

        // animação de ataque (se quiser recortar a linha de sprite adequada)
        if (atacando) {
            int linhaAtaque = (dirPlayer == DIREITA) ? 4 : 5; // supondo linhas 4/5
            f = (SDL_Rect){230 * (frameAtaque % totalFramesAtaque), 210 * linhaAtaque, 230, 210};
        }

        // física e plataformas
        player.y += vely;
        vely += gravidade;
        noChao = 0;

        // colisão com chão
        if (player.y + player.h >= chaoR.y) {
            player.y = chaoR.y - player.h; vely = 0; noChao = 1;
        }
        // colisão plataformas
        for (int i = 0; i < numPlataformas; i++) {
            SDL_Rect plat = plataformas[i];
            if (vely >= 0 && player.y + player.h > plat.y && player.y + player.h - vely <= plat.y &&
                player.x + player.w > plat.x && player.x < plat.x + plat.w) {
                player.y = plat.y - player.h;
                vely = 0;
                noChao = 1;
            }
        }

        // --- INTERAÇÕES: ataque acerta pedinte / pedinte acerta player ---
        // ataque acerta pedinte
        if (atacando) {
            for (int i = 0; i < numPedintes; i++) {
                if (SDL_HasIntersection(&hitboxPlayer, &pedintes[i].pos)) {
                    // recua o pedinte
                    pedintes[i].pos.x += (pedintes[i].dir == DIREITA) ? -RECUO_PEDINTE : RECUO_PEDINTE;
                    // opcional: empurra um pouco o player para evitar multiple hits; aqui só um break para evitar múltiplos acertos por ataque
                    break;
                }
            }
        }

        // pedinte colide com player -> player perde flor (se não invulneravel)
        for (int i = 0; i < numPedintes; i++) {
            if (SDL_HasIntersection(&player, &pedintes[i].pos)) {
                // recuo do pedinte
                pedintes[i].pos.x += (pedintes[i].dir == DIREITA ? -RECUO_PEDINTE : RECUO_PEDINTE);
                if (!invulneravel && vidas > 0 && animandoFlor == -1) {
                    animandoFlor = vidas - 1;
                    frameFlorMorrendo = 0;
                    ultimoFrameFlor = agora;
                    invulneravel = 1;
                    tempoInvulneravel = agora;
                }
            }
        }

        // câmera
        camera.x = player.x + player.w/2 - w/2;
        if (camera.x < cenarios[atual].posX) camera.x = cenarios[atual].posX;
        if (camera.x > cenarios[atual].posX + cenarios[atual].largura - w)
            camera.x = cenarios[atual].posX + cenarios[atual].largura - w;

        // transições de cenario
        if (player.x > cenarios[atual].posX + cenarios[atual].largura && (atual + 1) < totalCenarios) {
            fade_out_in(ren, w, h, 1);
            atual++;
            player.x = cenarios[atual].posX + 50;
            camera.x = player.x + player.w / 2 - w / 2;
            if (camera.x < cenarios[atual].posX) camera.x = cenarios[atual].posX;
            if (camera.x > cenarios[atual].posX + cenarios[atual].largura - w)
                camera.x = cenarios[atual].posX + cenarios[atual].largura - w;
            fade_out_in(ren, w, h, 0);
        }
        if (player.x + player.w < cenarios[atual].posX && atual > 0) {
            fade_out_in(ren, w, h, 1);
            atual--;
            player.x = cenarios[atual].posX + cenarios[atual].largura - 120;
            camera.x = player.x + player.w / 2 - w / 2;
            if (camera.x < cenarios[atual].posX) camera.x = cenarios[atual].posX;
            if (camera.x > cenarios[atual].posX + cenarios[atual].largura - w)
                camera.x = cenarios[atual].posX + cenarios[atual].largura - w;
            fade_out_in(ren, w, h, 0);
        }

        // --- RENDER ---
        SDL_RenderClear(ren);
        desenharCenario(ren, &cenarios[atual], camera, w, h,w,h);

        // plataformas (debug fill)
        for (int i = 0; i < numPlataformas; i++) {
            SDL_SetRenderDrawColor(ren, 0x80, 0x80, 0x80, 0xFF);
            SDL_RenderFillRect(ren, &plataformas[i]);
        }

        // jogador
        SDL_Rect playerScreen = player;
        playerScreen.x -= camera.x;
        SDL_RenderCopy(ren, sprites, &f, &playerScreen);

        // debug: hitbox ataque
        if (atacando) {
            SDL_Rect hitScreen = hitboxPlayer;
            hitScreen.x -= camera.x;
            SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(ren, 255, 0, 0, 128);
            SDL_RenderFillRect(ren, &hitScreen);
        }

        // pedintes
        for (int i = 0; i < numPedintes; i++) renderPedinte(ren, texPedinte, &pedintes[i], camera);

        // desenha frente cenário
        desenharFrente(ren, &cenarios[atual], camera);

        // HUD: desenha hud base
        SDL_RenderCopy(ren, hud, NULL, &vidaRect);

        // HUD: flores (animação por flor)
        for (int i = 0; i < 3; i++) {
            SDL_Rect florPos = { 15 + i * 70, vidaRect.h - 20, 60, 60 };
            SDL_Rect frame;
            if (i < vidas) frame = (SDL_Rect){0, 0, 280, florAltura};
            else if (i == animandoFlor) frame = (SDL_Rect){280 * frameFlorMorrendo, 0, 280, florAltura};
            else continue;
            SDL_RenderCopy(ren, texFlor, &frame, &florPos);
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
                            runGame(win, ren);
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
