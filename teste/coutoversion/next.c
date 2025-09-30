void runGame(SDL_Window* win, SDL_Renderer* ren) {
    SDL_Texture* sprites = IMG_LoadTexture(ren, "./src/entidades/ss.png");
    SDL_Texture* ponte   = IMG_LoadTexture(ren, "./src/mapa/ponte.png");
    SDL_Texture* fundo   = IMG_LoadTexture(ren, "./src/mapa/bg+lua.png");
    SDL_Texture* hud     = IMG_LoadTexture(ren, "./src/mapa/hud.png");
    SDL_Texture* parafu  = IMG_LoadTexture(ren, "./src/mapa/paralax fundo.png");
    SDL_Texture* parafr  = IMG_LoadTexture(ren, "./src/mapa/paralax frente.png");
    assert(sprites && ponte && fundo && hud && parafu && parafr);

    SDL_ShowCursor(SDL_DISABLE);

    int w, h;
    SDL_GetWindowSize(win, &w, &h);

    SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
    SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
    SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE);

    SDL_Rect camera = {0, 0, w, h};
    SDL_Rect vida = {0, 0, 384, 126};
    SDL_Rect player = { w/5, (h - ((15*h)/100)/3) - 100 + 5, 110, 100 };
    SDL_Rect ponteR = {0, h - ((15*h)/100), 2000, (15*h)/100};
    SDL_Rect chaoR  = {0, (h - ((15*h)/100)/3) + 5, 2000, ((15*h)/100)/3};
    SDL_Rect f      = {0, 0, 230, 210};

    // --- Plataformas ---
    int numPlataformas = 2;
    SDL_Rect plataformas[2] = {
        {200, h-200, 150, 20},
        {400, h-300, 150, 20}
    };

    // Variáveis do jogador
    int espera = 16;
    int vely = 0;
    int gravidade = 1;
    int puloInicial = -18;
    int noChao = 1;

    const int velX = 8; // velocidade horizontal ajustável

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
            player.x -= velX;
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
            player.x += velX;
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
            frameID = frameIE = 0;
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

        // Colisão com plataformas (cima)
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
                pulando = 0;
            }
        }

        // Colisão com plataformas (baixo)
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

        // --- CÂMERA SEGUINDO O JOGADOR ---
        camera.x = player.x + player.w / 2 - w / 2;
        camera.y = 0; // pode ajustar verticalmente se quiser
        if (camera.x < 0) camera.x = 0;
        if (camera.x > 4000 - w) camera.x = 4000 - w;

        // RENDER
        SDL_RenderClear(ren);

        SDL_Rect fundoR = {(w-2080)/2, 0, 2080, 1040};
        SDL_RenderCopy(ren, fundo, NULL, &fundoR);

        desenharParalax(ren, parafu, 3.0f, camera.x, 0);
        desenharParalax(ren, parafr, 1.5f, camera.x, 0);

        SDL_Rect ponteScreen = ponteR; ponteScreen.x -= camera.x;
        SDL_RenderCopy(ren, ponte, NULL, &ponteScreen);

        // --- PLATAFORMAS ---
        SDL_SetRenderDrawColor(ren, 120, 70, 20, 255);
        for (int i = 0; i < numPlataformas; i++) {
            SDL_Rect platScreen = plataformas[i];
            platScreen.x -= camera.x;
            SDL_RenderFillRect(ren, &platScreen);
        }

        SDL_Rect playerScreen = player; playerScreen.x -= camera.x;
        SDL_RenderCopy(ren, sprites, &f, &playerScreen);

        SDL_RenderCopy(ren, hud, NULL, &vida);
        SDL_RenderPresent(ren);
    }

    SDL_DestroyTexture(fundo);
    SDL_DestroyTexture(sprites);
    SDL_DestroyTexture(ponte);
    SDL_DestroyTexture(parafu);
    SDL_DestroyTexture(parafr);
    SDL_DestroyTexture(hud);
}
