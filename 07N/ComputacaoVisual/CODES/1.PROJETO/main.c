// Copyright (c) 2025 Andre Kishimoto - https://kishimoto.com.br/
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------
// Exemplo: 04-to_gray_scale (com equalização de histograma)
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

//------------------------------------------------------------------------------
// Custom types, structs, constants, etc.
//------------------------------------------------------------------------------
static const char *FONT_FILENAME = "arial.ttf";
static const int FONT_SIZE = 18;

enum constants
{
    DEFAULT_WINDOW_WIDTH = 640,
    DEFAULT_WINDOW_HEIGHT = 480,

    DEFAULT_H_WINDOW_WIDTH = 540,
    DEFAULT_H_WINDOW_HEIGHT = 580,
};

typedef struct MyWindow MyWindow;
struct MyWindow
{
    SDL_Window *window;
    SDL_Renderer *renderer;
};

typedef struct MyImage MyImage;
struct MyImage
{
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_FRect rect;
};

typedef struct {
    SDL_FRect rect;
    SDL_Color normal;
    SDL_Color hover;
    SDL_Color active;
    bool hovered;
    bool pressed;
} Button;

//------------------------------------------------------------------------------
// Globals
//------------------------------------------------------------------------------
static bool image_equalized = false;
static Button h_button;
static MyWindow g_window = { .window = NULL, .renderer = NULL };
static MyWindow h_window = { .window = NULL, .renderer = NULL };

// g_image é a imagem ativa, que será modificada e exibida
static MyImage g_image = {
    .surface = NULL,
    .texture = NULL,
    .rect = { .x = 0.0f, .y = 0.0f, .w = 0.0f, .h = 0.0f }
};
// g_image_two é o backup da imagem original em tons de cinza
static MyImage g_image_two = {
    .surface = NULL,
    .texture = NULL,
    .rect = { .x = 0.0f, .y = 0.0f, .w = 0.0f, .h = 0.0f }
};

int histogram[256] = { 0 };
int histogram_equalized[256] = { 0 }; // Tabela de mapeamento (LUT)
static TTF_Font *g_font = NULL;

//------------------------------------------------------------------------------
// Function declarations (prototypes)
//------------------------------------------------------------------------------
static void render_text(SDL_Renderer *renderer, const char *text, int x, int y, SDL_Color color);
static bool MyWindow_initialize(MyWindow *window, const char *title, int width, int height, SDL_WindowFlags window_flags);
static void MyWindow_destroy(MyWindow *window);
static void MyImage_destroy(MyImage *image);
static void load_rgba32(const char *filename, SDL_Renderer *renderer, MyImage *output_image, MyImage *output_image_two);
static void to_gray_scale(SDL_Renderer *renderer, MyImage *image);
static void save_image_as_png(MyImage *image, const char *filename);

void apply_equalization(SDL_Renderer *renderer, MyImage *image);
void restore_original_image(SDL_Renderer *renderer, MyImage *image_to_restore, MyImage *original_backup);
void calculate_histogram(void);
void calculate_equilize_vector(void);

float calculate_intensity(Uint8 r, Uint8 g, Uint8 b);
float calculate_average_intensity(void);
float calculate_standard_deviation(void);


const char *classify_intensity_string(int intensity);
const char *classify_deviation_string(float deviation);
int render_histogram(char str_max_bar[16]);

//------------------------------------------------------------------------------
// Implementação de render_text
//------------------------------------------------------------------------------
static void render_text(SDL_Renderer *renderer, const char *text, int x, int y, SDL_Color color)
{
    if (!g_font)
    {
        SDL_Log("Erro: Fonte não carregada (g_font é NULL).");
        return;
    }
    SDL_Surface *text_surface = TTF_RenderText_Solid(g_font, text, 0, color);
    if (!text_surface)
    {
        SDL_Log("Erro ao criar a superfície de texto: %s", SDL_GetError());
        return;
    }
    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    if (!text_texture)
    {
        SDL_Log("Erro ao criar a textura de texto: %s", SDL_GetError());
        SDL_DestroySurface(text_surface);
        return;
    }
    SDL_FRect dest_rect = { .x = (float)x, .y = (float)y, .w = (float)text_surface->w, .h = (float)text_surface->h };
    SDL_RenderTexture(renderer, text_texture, NULL, &dest_rect);
    SDL_DestroyTexture(text_texture);
    SDL_DestroySurface(text_surface);
}

//------------------------------------------------------------------------------
// Window / Image helpers
//------------------------------------------------------------------------------
static bool MyWindow_initialize(MyWindow *window, const char *title, int width, int height, SDL_WindowFlags window_flags)
{
    return SDL_CreateWindowAndRenderer(title, width, height, window_flags, &window->window, &window->renderer);
}

static void MyWindow_destroy(MyWindow *window)
{
    if (!window) return;
    if (window->renderer) SDL_DestroyRenderer(window->renderer);
    if (window->window) SDL_DestroyWindow(window->window);
    *window = (MyWindow){ .window = NULL, .renderer = NULL };
}

static void MyImage_destroy(MyImage *image)
{
    if (!image) return;
    if (image->texture) SDL_DestroyTexture(image->texture);
    if (image->surface) SDL_DestroySurface(image->surface);
    *image = (MyImage){ .surface = NULL, .texture = NULL, .rect = {0,0,0,0} };
}

//------------------------------------------------------------------------------
// Funções de Manipulação de Imagem
//------------------------------------------------------------------------------
static void load_rgba32(const char *filename, SDL_Renderer *renderer, MyImage *output_image, MyImage *output_image_two)
{
    if (!filename || !renderer || !output_image || !output_image_two) return;

    MyImage_destroy(output_image);
    MyImage_destroy(output_image_two);

    SDL_Surface *surface = IMG_Load(filename);

    if (!surface) { SDL_Log("Erro ao carregar a imagem: %s", SDL_GetError()); return; }

    output_image->surface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
    output_image_two->surface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(surface);
    if (!output_image->surface || !output_image_two->surface) { SDL_Log("Erro ao converter superficie para RGBA32: %s", SDL_GetError()); return; }

    output_image->texture = SDL_CreateTextureFromSurface(renderer, output_image->surface);
    output_image_two->texture = SDL_CreateTextureFromSurface(renderer, output_image_two->surface);
    if (!output_image->texture || !output_image_two->texture) { SDL_Log("Erro ao criar textura: %s", SDL_GetError()); return; }

    SDL_GetTextureSize(output_image->texture, &output_image->rect.w, &output_image->rect.h);
    SDL_GetTextureSize(output_image_two->texture, &output_image_two->rect.w, &output_image_two->rect.h);
}

static int verify_gray_scale(SDL_Renderer *renderer, MyImage *image)
{
    if (!renderer || !image || !image->surface) return -1;
    SDL_LockSurface(image->surface);
    const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(image->surface->format);
    const size_t pixelCount = image->surface->w * image->surface->h;
    Uint32 *pixels = (Uint32 *)image->surface->pixels;
    Uint8 r, g, b, a;
    for (size_t i = 0; i < pixelCount; ++i)
    {
        SDL_GetRGBA(pixels[i], format, NULL, &r, &g, &b, &a);
        if (!(r == g && b == g)) return 0;
    }
    return 1;
}


static void to_gray_scale(SDL_Renderer *renderer, MyImage *image)
{
    if (!renderer || !image || !image->surface) return;
    SDL_LockSurface(image->surface);
    const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(image->surface->format);
    const size_t pixelCount = image->surface->w * image->surface->h;
    Uint32 *pixels = (Uint32 *)image->surface->pixels;
    Uint8 r, g, b, a;
    for (size_t i = 0; i < pixelCount; ++i)
    {
        SDL_GetRGBA(pixels[i], format, NULL, &r, &g, &b, &a);
        float y = 0.2125f * r + 0.7154f * g + 0.0721f * b;
        Uint8 yy = (Uint8)roundf(y);
        pixels[i] = SDL_MapRGBA(format, NULL, yy, yy, yy, a);
    }
    SDL_UnlockSurface(image->surface);
    if (image->texture) SDL_DestroyTexture(image->texture);
    image->texture = SDL_CreateTextureFromSurface(renderer, image->surface);
}

void apply_equalization(SDL_Renderer *renderer, MyImage *image)
{
    if (!renderer || !image || !image->surface) return;
    calculate_equilize_vector();
    SDL_LockSurface(image->surface);
    const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(image->surface->format);
    const size_t pixelCount = image->surface->w * image->surface->h;
    Uint32 *pixels = (Uint32 *)image->surface->pixels;
    for (size_t i = 0; i < pixelCount; i++)
    {
        Uint8 r, g, b, a;
        SDL_GetRGBA(pixels[i], format, NULL, &r, &g, &b, &a);

        Uint8 new_intensity = (Uint8)histogram_equalized[r];
        pixels[i] = SDL_MapRGBA(format, NULL, new_intensity, new_intensity, new_intensity, a);
    }
    SDL_UnlockSurface(image->surface);
    if (image->texture) SDL_DestroyTexture(image->texture);
    image->texture = SDL_CreateTextureFromSurface(renderer, image->surface);
}

void restore_original_image(SDL_Renderer *renderer, MyImage *image_to_restore, MyImage *original_backup)
{
    if (!renderer || !image_to_restore || !original_backup) return;
    SDL_memcpy(image_to_restore->surface->pixels, original_backup->surface->pixels, original_backup->surface->h * original_backup->surface->pitch);
    if (image_to_restore->texture) SDL_DestroyTexture(image_to_restore->texture);
    image_to_restore->texture = SDL_CreateTextureFromSurface(renderer, image_to_restore->surface);
}


//------------------------------------------------------------------------------
// initialize / shutdown
//------------------------------------------------------------------------------
static SDL_AppResult initialize(void)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) { SDL_Log("Erro ao iniciar a SDL: %s", SDL_GetError()); return SDL_APP_FAILURE; }
    if (TTF_Init() != 1) { SDL_Log("Erro ao iniciar a SDL_ttf: %s", SDL_GetError()); return SDL_APP_FAILURE; }
    if (!MyWindow_initialize(&g_window, "IMAGEM", DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, 0) ||
        !MyWindow_initialize(&h_window, "HISTOGRAMA", DEFAULT_H_WINDOW_WIDTH, DEFAULT_H_WINDOW_HEIGHT, 0))
    {
        SDL_Log("Erro ao criar a janela e/ou renderizador: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    return SDL_APP_CONTINUE;
}

static void shutdown(void)
{
    if (g_font) { TTF_CloseFont(g_font); g_font = NULL; }
    MyImage_destroy(&g_image);
    MyImage_destroy(&g_image_two);
    MyWindow_destroy(&g_window);
    MyWindow_destroy(&h_window);
    TTF_Quit();
    SDL_Quit();
}

//------------------------------------------------------------------------------
// Button (draw + event handling)
//------------------------------------------------------------------------------
void draw_button(SDL_Renderer *renderer, Button *btn, const char *label)
{
    SDL_Color color = btn->normal;
    if (btn->pressed) color = btn->active;
    else if (btn->hovered) color = btn->hover;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderFillRect(renderer, &btn->rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderRect(renderer, &btn->rect);
    if (g_font)
    {
        SDL_Color textColor = {255, 255, 255, 255};
        render_text(renderer, label, (int)(btn->rect.x + 10), (int)(btn->rect.y + 8), textColor);
    }
}

bool handle_button_event(Button *btn, SDL_Event *e, SDL_Window *win)
{
    SDL_WindowID winID = SDL_GetWindowID(win);
    if ((e->type == SDL_EVENT_MOUSE_MOTION && e->motion.windowID != winID) ||
        (e->type == SDL_EVENT_MOUSE_BUTTON_DOWN && e->button.windowID != winID) ||
        (e->type == SDL_EVENT_MOUSE_BUTTON_UP && e->button.windowID != winID)) {
        return false;
    }
    float mx = 0, my = 0;
    if (e->type == SDL_EVENT_MOUSE_MOTION) { mx = e->motion.x; my = e->motion.y; }
    else if (e->type == SDL_EVENT_MOUSE_BUTTON_DOWN || e->type == SDL_EVENT_MOUSE_BUTTON_UP) { mx = e->button.x; my = e->button.y; }
    bool inside = (mx > btn->rect.x && mx < btn->rect.x + btn->rect.w && my > btn->rect.y && my < btn->rect.y + btn->rect.h);
    bool old_hovered = btn->hovered;
    bool old_pressed = btn->pressed;
    btn->hovered = inside;
    bool state_changed = (btn->hovered != old_hovered);
    if (inside) {
        if (e->type == SDL_EVENT_MOUSE_BUTTON_DOWN) { btn->pressed = true; }
        else if (e->type == SDL_EVENT_MOUSE_BUTTON_UP) { if (btn->pressed) { btn->pressed = false; return true; } }
    } else { btn->pressed = false; }
    if (btn->pressed != old_pressed) { state_changed = true; }
    return state_changed;
}

//------------------------------------------------------------------------------
// Histogram rendering
//------------------------------------------------------------------------------
int render_histogram(char str_max_bar[16])
{
    int max_value = 1;
    for (int i = 0; i < 256; i++) { if (histogram[i] > max_value) max_value = histogram[i]; }
    snprintf(str_max_bar, 16, "%d", max_value);

    float bar_width = (float)(DEFAULT_H_WINDOW_WIDTH - 80) / 256.0f;
    int graph_height = DEFAULT_H_WINDOW_HEIGHT - 240;
    int graph_y_start = 60;

    for (int i = 0; i < 256; i++)
    {
        float bar_height = ((float)histogram[i] / (float)max_value) * (graph_height - 40);
        SDL_SetRenderDrawColor(h_window.renderer, 200, 200, 220, 255);
        SDL_FRect bar = {
            .x = 40.0f + i * bar_width,
            .y = (float)graph_y_start + (float)graph_height - bar_height,
            .w = bar_width,
            .h = bar_height
        };
        SDL_RenderFillRect(h_window.renderer, &bar);
    }
    return max_value;
}

//------------------------------------------------------------------------------
// Render principal (imagem + histograma)
//------------------------------------------------------------------------------
static void render(void)
{
    // Janela da imagem
    SDL_SetRenderDrawColor(g_window.renderer, 128, 128, 128, 255);
    SDL_RenderClear(g_window.renderer);
    if (g_image.texture) SDL_RenderTexture(g_window.renderer, g_image.texture, &g_image.rect, &g_image.rect);
    SDL_RenderPresent(g_window.renderer);

    // Janela do histograma
    SDL_SetRenderDrawColor(h_window.renderer, 40, 40, 60, 255);
    SDL_RenderClear(h_window.renderer);

    char str_max_bar[16];
    render_histogram(str_max_bar);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color light_gray = {200, 200, 200, 255};

    if(image_equalized)
        draw_button(h_window.renderer, &h_button, "         Equalizado");
    else
    {
        draw_button(h_window.renderer, &h_button, "          Original");
    }
    
    render_text(h_window.renderer, "Histograma de Intensidade", 150, 15, white);
    render_text(h_window.renderer, "Frequencia", 10, 35, light_gray);
    render_text(h_window.renderer, str_max_bar, 20, 55, light_gray);
    render_text(h_window.renderer, "Niveis de Cinza", 210 , DEFAULT_H_WINDOW_HEIGHT - 170, light_gray);
    render_text(h_window.renderer, "0", 35, DEFAULT_H_WINDOW_HEIGHT - 170, light_gray);
    render_text(h_window.renderer, "255", DEFAULT_H_WINDOW_WIDTH - 65, DEFAULT_H_WINDOW_HEIGHT - 170, light_gray);

    float avg_intensity = calculate_average_intensity();
    float std_deviation = calculate_standard_deviation();
    const char *class_intensity = classify_intensity_string((int)roundf(avg_intensity));
    const char *class_deviation = classify_deviation_string(std_deviation);

    char str_avg_intensity[16];
    char str_std_deviation[16];
    snprintf(str_avg_intensity, sizeof(str_avg_intensity), " %.2f", avg_intensity);
    snprintf(str_std_deviation, sizeof(str_std_deviation), " %.2f", std_deviation);

    render_text(h_window.renderer, "MD: ", 20, 450, light_gray);
    render_text(h_window.renderer, str_avg_intensity, 80, 450, light_gray);
    render_text(h_window.renderer, "DP: ", 20, 470, light_gray);
    render_text(h_window.renderer, str_std_deviation, 80, 470, light_gray);
    render_text(h_window.renderer, "CI:", 20, 490, light_gray);
    render_text(h_window.renderer, class_intensity, 80, 490, light_gray);
    render_text(h_window.renderer, "CC: ", 20, 510, light_gray);
    render_text(h_window.renderer, class_deviation, 80, 510, light_gray);

    SDL_RenderPresent(h_window.renderer);
}

//------------------------------------------------------------------------------
// Loop principal
//------------------------------------------------------------------------------
static void loop(void)
{
    bool mustRefresh = true;
    SDL_Event event;
    bool isRunning = true;

    while (isRunning)
    {
        if (mustRefresh) {
            render();
            mustRefresh = false;
        }

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                    isRunning = false;
                    break;
                case SDL_EVENT_KEY_DOWN:
                    if (event.key.key == SDLK_ESCAPE) isRunning = false;
                    else if (event.key.key == SDLK_S) // Salvar imagem
                    {
                        SDL_Log("Acao executada: Salvar Imagem.");
                        save_image_as_png(&g_image, "output_image.png");
                    }
                    break;
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                {
                    SDL_WindowID windowID = event.window.windowID;
                    if (windowID == SDL_GetWindowID(g_window.window)) isRunning = false;
                    else if (windowID == SDL_GetWindowID(h_window.window)) SDL_HideWindow(h_window.window);
                    break;
                }
            }

            if (handle_button_event(&h_button, &event, h_window.window))
            {
                if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && h_button.hovered)
                {
                    image_equalized = !image_equalized;
                    if (image_equalized)
                    {
                        SDL_Log("Acao executada: Equalizar Imagem.");
                        apply_equalization(g_window.renderer, &g_image);
                    }
                    else
                    {
                        SDL_Log("Acao executada: Restaurar Imagem Original.");
                        restore_original_image(g_window.renderer, &g_image, &g_image_two);
                    }

                    calculate_histogram();
                    
                    float avg_intensity = calculate_average_intensity();
                    float std_deviation = calculate_standard_deviation();
                    SDL_Log("Nova Media (%.0f) -> %s", avg_intensity, classify_intensity_string((int)roundf(avg_intensity)));
                    SDL_Log("Novo Desvio(%.2f) -> %s", std_deviation, classify_deviation_string(std_deviation));

                    mustRefresh = true;
                }
                else { mustRefresh = true; }
            }
        }
    }
}

//------------------------------------------------------------------------------
// Funções de Cálculo e Classificação
//------------------------------------------------------------------------------
float calculate_intensity(Uint8 r, Uint8 g, Uint8 b) { return (r + g + b) / 3.0f; }

float calculate_average_intensity()
{
    if (!g_image.surface) return 0.0f;
    int pixelCount = g_image.surface->w * g_image.surface->h;
    if (pixelCount == 0) return 0.0f;
    float soma = 0.0f;
    SDL_LockSurface(g_image.surface);
    Uint32 *pixels = (Uint32 *)g_image.surface->pixels;
    const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(g_image.surface->format);
    for (int i = 0; i < pixelCount; i++) {
        Uint8 r, g, b, a;
        SDL_GetRGBA(pixels[i], format, NULL, &r, &g, &b, &a);
        soma += r; // Como a imagem já está em escala de cinza, não preciso calcular a média de RGB
    }
    SDL_UnlockSurface(g_image.surface);
    return soma / (float)pixelCount;
}

float calculate_standard_deviation()
{
    if (!g_image.surface) return 0.0f;
    int pixelCount = g_image.surface->w * g_image.surface->h;
    if (pixelCount == 0) return 0.0f;
    float media = calculate_average_intensity();
    float soma = 0.0f;
    SDL_LockSurface(g_image.surface);
    Uint32 *pixels = (Uint32 *)g_image.surface->pixels;
    const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(g_image.surface->format);
    for (int i = 0; i < pixelCount; i++) {
        Uint8 r, g, b, a;
        SDL_GetRGBA(pixels[i], format, NULL, &r, &g, &b, &a);
        float diff = r - media; // Como a imagem já está em escala de cinza, não preciso calcular a média de RGB, só preciso do r.
        soma += diff * diff;
    }
    SDL_UnlockSurface(g_image.surface);
    float variancia = soma / (float)pixelCount;
    return sqrtf(variancia);
}

void calculate_histogram()
{
    if (!g_image.surface) return;
    for (int i = 0; i < 256; ++i) histogram[i] = 0;
    int pixelCount = g_image.surface->w * g_image.surface->h;
    if (pixelCount == 0) return;
    SDL_LockSurface(g_image.surface);
    Uint32 *pixels = (Uint32 *)g_image.surface->pixels;
    const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(g_image.surface->format);
    for (int i = 0; i < pixelCount; i++) {
        Uint8 r, g, b, a;
        SDL_GetRGBA(pixels[i], format, NULL, &r, &g, &b, &a);
        histogram[r]++;
    }
    SDL_UnlockSurface(g_image.surface);
}

void calculate_equilize_vector()
{
    if (!g_image.surface) return;
    int sum = 0;
    int pixelCount = g_image.surface->w * g_image.surface->h;
    for (int i = 0; i < 256; ++i) {
        sum += histogram[i];
        histogram_equalized[i] = roundf(((float)sum * (255.0f / (float)pixelCount)));
    }
}

const char *classify_intensity_string(int intensity)
{
    if (intensity < 85) return "Intensidade baixa";
    if (intensity <= 170) return "Intensidade media";
    return "Intensidade alta";
}

const char *classify_deviation_string(float deviation)
{
    if (deviation < 52.0f) return "Baixo contraste";
    if (deviation <= 104.0f) return "Medio contraste";
    return "Alto contraste";
}


//------------------------------------------------------------------------------
// Funções de Manipulação de Imagem
//------------------------------------------------------------------------------

static void save_image_as_png(MyImage *image, const char *filename)
{
    if (!image || !image->surface)
    {
        SDL_Log("Erro ao salvar: a imagem ou sua superficie e nula.");
        return;
    }

    if (IMG_SavePNG(image->surface, filename) != 1)
    {
        SDL_Log("Nao foi possivel salvar a imagem em '%s': %s", filename, SDL_GetError());
    }
    else
    {
        SDL_Log("Imagem salva com sucesso em '%s'.", filename);
    }
}

//------------------------------------------------------------------------------
// main()
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    atexit(shutdown);
    if (argc < 2) {
        SDL_Log("Uso: %s <arquivo_imagem>", argv[0]);
        return SDL_APP_FAILURE;
    }
    if (initialize() == SDL_APP_FAILURE) return SDL_APP_FAILURE;

    g_font = TTF_OpenFont(FONT_FILENAME, FONT_SIZE);
    if (!g_font) {
        SDL_Log("Erro ao carregar a fonte '%s': %s", FONT_FILENAME, SDL_GetError());
        return SDL_APP_FAILURE;
    }

    load_rgba32(argv[1], g_window.renderer, &g_image, &g_image_two);

    // Converte a imagem principal para tons de cinza
    if(verify_gray_scale(g_window.renderer, &g_image) == 0) to_gray_scale(g_window.renderer, &g_image);
    
    // CRÍTICO: Copia a versão em tons de cinza para o backup
    // para que a restauração funcione corretamente.
    SDL_memcpy(g_image_two.surface->pixels, g_image.surface->pixels, g_image.surface->h * g_image.surface->pitch);
    
    // Calcula o histograma inicial (da imagem em tons de cinza)
    calculate_histogram();

    h_button.rect = (SDL_FRect){ .x = 300, .y = DEFAULT_H_WINDOW_HEIGHT - 100, .w = 190, .h = 35 };
    h_button.normal = (SDL_Color){0, 0, 200, 255};
    h_button.hover = (SDL_Color){20, 150, 220, 255};
    h_button.active = (SDL_Color){0, 0, 100, 255};
    h_button.hovered = false;
    h_button.pressed = false;
   
    
    if ((int)g_image.rect.w > 1920 || (int)g_image.rect.h > 1080)
    {
        SDL_Log("Atenção: A imagem é muito grande (%dx%d). Reduzindo para 1920x1080.", (int)g_image.rect.w, (int)g_image.rect.h);
        float aspect_ratio = g_image.rect.w / g_image.rect.h;
        if (aspect_ratio > 1.0f) {
            g_image.rect.w = 1920.0f;
            g_image.rect.h = 1920.0f / aspect_ratio;
        } else {
            g_image.rect.h = 1080.0f;
            g_image.rect.w = 1080.0f * aspect_ratio;
        }
    }
    
    int imageWidth = (int)g_image.rect.w;
    int imageHeight = (int)g_image.rect.h;
    if (imageWidth > 0 && imageHeight > 0)
    {
        // Posiciona a janela da imagem no centro
        SDL_SetWindowSize(g_window.window, imageWidth, imageHeight);
        SDL_SetWindowPosition(g_window.window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        

        // 1. Obter os limites do display principal
        SDL_Rect display_bounds;
        SDL_DisplayID main_display_id = SDL_GetPrimaryDisplay();
        if (SDL_GetDisplayBounds(main_display_id, &display_bounds) != 1)
        {
            SDL_Log("Nao foi possivel obter os limites do display: %s", SDL_GetError());
            // Define um fallback (resolução comum) caso a função falhe
            display_bounds = (SDL_Rect){ .x = 0, .y = 0, .w = 1920, .h = 1080 };
        }
        display_bounds.y += 40; 

        // 2. Obter o tamanho da janela do histograma
        int h_win_w, h_win_h;
        SDL_GetWindowSize(h_window.window, &h_win_w, &h_win_h);

        // 3. Calcular a posição no canto superior direito do display
        int margin = 20; // Uma pequena margem das bordas da tela
        int h_win_x = display_bounds.x + display_bounds.w - h_win_w - margin;
        int h_win_y = display_bounds.y + margin;

        // 4. Define a posição final e fixa da janela do histograma
        SDL_SetWindowPosition(h_window.window, h_win_x, h_win_y);

    }
    
    loop();

    return 0;
}