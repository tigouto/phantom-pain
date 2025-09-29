#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>

void runGame(SDL_Window* win, SDL_Renderer* ren) {
    // --- CARREGAMENTO DE TEXTURAS ---
    SDL_Texture* sprites = IMG_LoadTexture(ren, "./src/entidades/ss.png");
    SDL_Texture* ponte = IMG_LoadTexture(ren, "./src/mapa/ponte.png");
    SDL_Texture* fundo = IMG_LoadTexture(ren, "./src/mapa/bg+lua.png");
    SDL_Texture* hud = IMG_LoadTexture(ren, "./src/mapa/hud.png");
    SDL_Texture* parafu = IMG_LoadTexture(ren, "./src/mapa/paralax fundo.png");
    SDL_Texture* parafr = IMG_LoadTexture(ren, "./src/mapa/paralax frente.png");
    assert(sprites && ponte && fundo && hud && parafu && parafr);

    int w, h;
    SDL_GetWindowSize(win, &w, &h);

    SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
    SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
    SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE);

    // --- RECTs ---
    SDL_Rect mapa = {0, 0, w, h};
    SDL_Rect parafun = {0, 0, w, h};
    SDL_Rect parafre = {0, 0, w, h};
    SDL_Rect vida = {0, 0, 384, 126};

    SDL_Rect player = { w/5, (h - ((15*h)/100)/3) - 100 + 5, 110, 100 };
    SDL_Rect ponteR = {0, h - ((15*h)/100), w, (15*h)/100};
    SDL_Rect chaoR = {0, (h - ((15*h)/100)/3) + 5, w, ((15*h)/100)/3};
    SDL_Rect f = {0, 0, 230, 210};

    int numPlataformas = 2;
    SDL_Rect plataformas[2] = {
        {200, h-200, 150, 20},
        {400, h-300, 150, 20}
    };

    // --- VARIÁVEIS DO JOGADOR ---
    int espera = 16;
    int vely = 0;
    int gravidade = 1;
    int puloInicial = -18;
    int noChao = 1;

    int direita = 1, esquerda = -1, frameAtual = direita;
    int virando = 0, frameVirada = 0;
    int frameID = 0, frameIE = 0;
    Uint32 ultimoFrameTroca = 0;
    int intervaloFrame = 120;

    // --- LOOP PRINCIPAL DO JOGO ---
    while (!SDL_QuitRequested()) {
        // RENDERIZAÇÃO
        SDL_RenderCopy(ren, fundo, NULL, &mapa);
        SDL_RenderCopy(ren, parafu, NULL, &parafun);
        SDL_RenderCopy(ren, parafr, NULL, &parafre);
        SDL_RenderCopy(ren, hud, NULL, &vida);

        SDL_SetRenderDrawColor(ren, 0x80, 0x80, 0x80, 0x00);
        for (int i = 0; i < numPlataformas; i++)
            SDL_RenderFillRect(ren, &plataformas[i]);

        SDL_RenderCopy(ren, sprites, &f, &player);
        SDL_RenderCopy(ren, ponte, NULL, &ponteR);
        SDL_RenderPresent(ren);

        // EVENTOS
        SDL_Event evt;
        int isevt = SDL_WaitEventTimeout(&evt, espera);
        if (isevt && evt.type == SDL_QUIT) break;

        // TECLADO
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        Uint32 agora = SDL_GetTicks();

        // PULO
        if (keys[SDL_SCANCODE_Z] && noChao) {
            vely = puloInicial;
            noChao = 0;
        }

        int movendo = 0;

        // MOVIMENTO ESQUERDA
        if (keys[SDL_SCANCODE_LEFT]) {
            movendo = 1;
            if (frameAtual == direita && !virando) { virando = 1; frameVirada = 0; ultimoFrameTroca = agora; }
            player.x -= 13;
            if (virando == 1) {
                if (agora - ultimoFrameTroca > intervaloFrame) {
                    ultimoFrameTroca = agora;
                    frameVirada++;
                    if (frameVirada > 2) { virando = 0; frameIE = 0; frameAtual = esquerda; f = (SDL_Rect){0,210*2,230,210}; }
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

        // MOVIMENTO DIREITA
        if (keys[SDL_SCANCODE_RIGHT]) {
            movendo = 1;
            if (frameAtual == esquerda && !virando) { virando = 2; frameVirada = 0; ultimoFrameTroca = agora; }
            player.x += 13;
            if (virando == 2) {
                if (agora - ultimoFrameTroca > intervaloFrame) {
                    ultimoFrameTroca = agora;
                    frameVirada++;
                    if (frameVirada > 2) { virando = 0; frameID = 0; frameAtual = direita; f = (SDL_Rect){0,0,230,210}; }
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

        // PARADO
        if (!movendo && !virando) {
            if (frameAtual == direita) f = (SDL_Rect){0,0,230,210};
            else f = (SDL_Rect){0,210*2,230,210};
        }

        // GRAVIDADE
        player.y += vely;
        vely += gravidade;
        noChao = 0;

        // COLISÃO COM CHÃO
        if (player.y + player.h >= chaoR.y) { player.y = chaoR.y - player.h; vely = 0; noChao = 1; }

        // COLISÃO COM PLATAFORMAS (por cima)
        for (int i = 0; i < numPlataformas; i++) {
            SDL_Rect plat = plataformas[i];
            if (vely >= 0 &&
                player.y + player.h > plat.y &&
                player.y + player.h - vely <= plat.y &&
                player.x + player.w > plat.x &&
                player.x < plat.x + plat.w) {
                player.y = plat.y - player.h;
                vely = 0;
                noChao = 1;
            }
        }

        // COLISÃO POR BAIXO (bateu cabeça)
        for (int i = 0; i < numPlataformas; i++) {
            SDL_Rect plat = plataformas[i];
            if (vely < 0 &&
                player.y <= plat.y + plat.h &&
                player.y - vely >= plat.y + plat.h &&
                player.x + player.w > plat.x &&
                player.x < plat.x + plat.w) {
                player.y = plat.y + plat.h;
                vely = 0;
            }
        }
    } // fim do loop do jogo

    // --- LIMPEZA ---
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
    SDL_Window* win = SDL_CreateWindow("Game v0.1", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                       0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, 0);

    SDL_Texture* fundo      = IMG_LoadTexture(ren, "./src/menu/bg-menu.png");
    SDL_Texture* logo       = IMG_LoadTexture(ren, "./src/menu/pp-logo.png");
    SDL_Texture* novo       = IMG_LoadTexture(ren, "./src/menu/novo-j.png");
    SDL_Texture* continuar  = IMG_LoadTexture(ren, "./src/menu/continuar.png");
    SDL_Texture* sair       = IMG_LoadTexture(ren, "./src/menu/sair.png");
    assert(fundo && logo && novo && continuar && sair);

    int w, h;
    SDL_GetWindowSize(win, &w, &h);

    SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
    SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
    SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE);

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

    while(rodando && !SDL_QuitRequested()){
        SDL_RenderCopy(ren, fundo ,NULL, NULL);
        SDL_RenderCopy(ren, logo ,NULL, &titulo);
        SDL_RenderCopy(ren, novo ,&n, &novoJ);
        SDL_RenderCopy(ren, continuar ,&c, &continuarJ);
        SDL_RenderCopy(ren, sair ,&s, &sairJ);
        SDL_RenderPresent(ren);

        SDL_Event evt;
        int isevt = SDL_WaitEventTimeout(&evt, espera);
        if (isevt && evt.type == SDL_QUIT) break;

        if (evt.type == SDL_KEYDOWN){
            switch(evt.key.keysym.scancode) {
                case SDL_SCANCODE_DOWN:
                case SDL_SCANCODE_UP:
                    if (selecionadoN == 1) { n = (SDL_Rect){0,0,315,35}; s = (SDL_Rect){315,0,315,35}; selecionadoN=0; selecionadoS=1; }
                    else if (selecionadoS == 1) { s = (SDL_Rect){0,0,315,35}; n = (SDL_Rect){315,0,315,35}; selecionadoN=1; selecionadoS=0; }
                    break;

                case SDL_SCANCODE_RETURN:
                case SDL_SCANCODE_Z:
                    if(selecionadoN == 1){
                        // inicia o jogo
                        runGame(win, ren);
                    } else if(selecionadoS == 1){
                        rodando = 0;
                    }
                    break;
            }
        }
    }

    SDL_DestroyTexture(sair);
    SDL_DestroyTexture(continuar);
    SDL_DestroyTexture(novo);
    SDL_DestroyTexture(logo);
    SDL_DestroyTexture(fundo);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}
