/*
Copyright 2021 Jussi Pakkanen

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <SDL.h>
#include <nib.h>
#include <vector>

SDL_Window *window;
SDL_Renderer *renderer;
SDL_GameController *gamepad = nullptr;
SDL_Surface *surface;
SDL_Texture *texture;

const int win_w = 800;
const int win_h = 600;

const int nib_preview_w = win_w / 10;
const int nib_preview_h = win_h / 10;

const int nib_draw_w = win_w / 20;
const int nib_draw_h = win_h / 20;

int16_t xaxis = 0;
int16_t yaxis = 0;

struct Blob {
    SDL_Rect dst;
    double angle;
};

std::vector<Blob> stroke;

void draw_blob(const Blob &b) {
    SDL_RenderCopyEx(renderer, texture, nullptr, &b.dst, b.angle, nullptr, SDL_FLIP_NONE);
}

void do_preview() {
    Blob blob;
    if(SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_LEFTSHOULDER)) {
        xaxis = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_LEFTX);
        yaxis = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_LEFTY);
    }
    const auto trigger = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
    blob.angle = 0;
    if(abs(xaxis) > 2000 || abs(yaxis) > 2000) {
        blob.angle = atan2(yaxis, xaxis) / (2 * M_PI) * 360 + 90;
    }
    const double nib_size = trigger / double(32767);
    const int nib_w = int(nib_preview_w * (0.2 + 0.8 * nib_size));
    const int nib_h = nib_preview_h / 8;
    blob.dst.x = win_w - nib_preview_w / 2 - nib_w / 2;
    blob.dst.y = win_h - nib_preview_h / 2 - nib_h / 2;
    blob.dst.w = nib_w;
    blob.dst.h = nib_h;
    draw_blob(blob);
}

void do_stroke() {
    int x, y;
    if(SDL_GetMouseState(&x, &y) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        Blob blob;
        const auto trigger = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
        blob.angle = 0;
        if(abs(xaxis) > 2000 || abs(yaxis) > 2000) {
            blob.angle = atan2(yaxis, xaxis) / (2 * M_PI) * 360 + 90;
        }
        const double nib_size = trigger / double(32767);
        const int nib_w = int(nib_draw_w * (0.2 + 0.8 * nib_size));
        const int nib_h = nib_draw_h / 8;
        blob.dst.x = x - nib_w / 2;
        blob.dst.y = y - nib_h / 2;
        blob.dst.w = nib_w;
        blob.dst.h = nib_h;
        stroke.push_back(blob);
    }
    for(const auto &b : stroke) {
        draw_blob(b);
    }
}

void do_frame() {
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    do_preview();
    do_stroke();
    SDL_RenderPresent(renderer);
}

int main() {
    SDL_Event e;

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        printf("Couldn't initialize SDL: %s", SDL_GetError());
        return 1;
    }

    if(SDL_CreateWindowAndRenderer(win_w, win_h, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        printf("Couldn't create window and renderer: %s", SDL_GetError());
        return 1;
    }
    SDL_SetWindowTitle(window, "Gamepad painter");

    for(int i = 0; i < SDL_NumJoysticks(); ++i) {
        if(SDL_IsGameController(i)) {
            gamepad = SDL_GameControllerOpen(i);
            if(gamepad) {
                printf("Using controller: %s\n", SDL_GameControllerName(gamepad));
                break;
            } else {
                printf("Controller open fail: %s", SDL_GetError());
                std::abort();
            }
        }
    }
    if(!gamepad) {
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR, "Init failed", "Gamepad not detected.", window);
        std::abort();
    }

    SDL_RWops *rwops = SDL_RWFromMem(nib_bmp, nib_bmp_len);
    surface = SDL_LoadBMP_RW(rwops, 0);
    SDL_RWclose(rwops);
    if(!surface) {
        printf("Couldn't create surface from image: %s", SDL_GetError());
        return 1;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if(!texture) {
        printf("Couldn't create texture from surface: %s", SDL_GetError());
        return 1;
    }

    SDL_RenderClear(renderer);
    bool keep_going = true;
    while(keep_going) {
        while(SDL_PollEvent(&e)) {
            if(e.type == SDL_KEYDOWN) {
                if(e.key.keysym.sym == SDLK_ESCAPE || e.key.keysym.sym == SDLK_q) {
                    keep_going = false;
                }
            }
            if(e.type == SDL_CONTROLLERBUTTONDOWN) {
                if(e.cbutton.button == SDL_CONTROLLER_BUTTON_B ||
                   e.cbutton.button == SDL_CONTROLLER_BUTTON_BACK) {
                    keep_going = false;
                }
                if(e.cbutton.button == SDL_CONTROLLER_BUTTON_Y) {
                    stroke.clear();
                }
            }

            if(e.type == SDL_QUIT) {
                keep_going = false;
            }
        }
        do_frame();
        SDL_Delay(30);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
