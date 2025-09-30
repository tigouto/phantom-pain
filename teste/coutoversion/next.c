int numPlataformas = 2;
    SDL_Rect plataformas[2] = {
        {200, h-200, 150, 20},
        {400, h-300, 150, 20}
    };





        // Colisão com plataformas (parte superior)
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

        // Colisão por baixo (bate na parte inferior de plataforma)
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







   SDL_SetRenderDrawColor(ren, 120, 70, 20, 255);
        for (int i = 0; i < numPlataformas; i++) {
            SDL_RenderFillRect(ren, &plataformas[i]);
        }
