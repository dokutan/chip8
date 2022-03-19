#include <exception>
#include <stdexcept>
#include <SDL2/SDL.h>
#include <iostream>

class frontend_sdl{
    protected:
        int screen_width, screen_height, scale, frame_time;

    private:
        SDL_Window* sdl_window;
        SDL_Renderer* sdl_renderer;
        SDL_Texture* sdl_texture;
        SDL_Event sdl_event;
        SDL_AudioDeviceID sdl_audio_device_id;
        SDL_AudioSpec sdl_audio_spec;
        bool sdl_initialized = false;
        Uint64 time_to_refresh = 0;
        const Uint8 *keys;

    public:
        frontend_sdl(int render_width, int render_height, int scale, int refresh_rate){
            this->scale = scale * 2;
            screen_width = render_width * this->scale;
            screen_height = render_height * this->scale;
            frame_time = 1000 / refresh_rate;

            // initialize SDL
            if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0){
                throw std::runtime_error(SDL_GetError());
            }

            // create window
            sdl_window = SDL_CreateWindow("chip8 interpreter", 0, 0, screen_width, screen_height, SDL_WINDOW_SHOWN);
            if(sdl_window == nullptr){
                std::string error = SDL_GetError();
                SDL_Quit();
                throw std::runtime_error(error);
            }

            // create renderer
            sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
            if(sdl_renderer == nullptr){
                std::string error = SDL_GetError();
                SDL_DestroyWindow(sdl_window);
                SDL_Quit();
                throw std::runtime_error(error);
            }

            // create texture
            sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, screen_width, screen_height);
            if(sdl_texture == nullptr){
                std::string error = SDL_GetError();
                SDL_DestroyRenderer(sdl_renderer);
                SDL_DestroyWindow(sdl_window);
                SDL_Quit();
                throw std::runtime_error(error);
            }

            // get keyboard state
            keys = SDL_GetKeyboardState(nullptr);
            if(keys == nullptr){
                std::string error = SDL_GetError();
                SDL_DestroyTexture(sdl_texture);
                SDL_DestroyRenderer(sdl_renderer);
                SDL_DestroyWindow(sdl_window);
                SDL_Quit();
                throw std::runtime_error(error);
            }

            // open audio device
            SDL_zero(sdl_audio_spec);
            sdl_audio_spec.freq = 48000;
            sdl_audio_spec.format = AUDIO_U8;
            sdl_audio_spec.channels = 1;
            sdl_audio_spec.samples = 32;
            sdl_audio_spec.callback = this->audio_callback;
            sdl_audio_device_id = SDL_OpenAudioDevice(nullptr, 0, &sdl_audio_spec, nullptr, 0);
            if(sdl_audio_device_id == 0){
                std::string error = SDL_GetError();
                SDL_DestroyTexture(sdl_texture);
                SDL_DestroyRenderer(sdl_renderer);
                SDL_DestroyWindow(sdl_window);
                SDL_Quit();
                throw std::runtime_error(error);
            }

            sdl_initialized = true;

            // clear window
            SDL_SetRenderTarget(sdl_renderer, sdl_texture);
            SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0x00, 0x00, 0x00);
            SDL_RenderClear(sdl_renderer);
            refresh();
        }

        ~frontend_sdl(){
            if(sdl_initialized){
                SDL_CloseAudioDevice(sdl_audio_device_id);
                SDL_DestroyTexture(sdl_texture);
                SDL_DestroyRenderer(sdl_renderer);
                SDL_DestroyWindow(sdl_window);
                SDL_Quit();
            }
        }

        static void audio_callback(void *userdata, uint8_t *stream, int len){
            for(int i = 0; i < len; i++){
                stream[i] = (i % 20 == 0) ? 0xff : 0x00;
            }
        }

        /// start or stop the audio
        void set_audio_state(bool playing){
            SDL_PauseAudioDevice(sdl_audio_device_id, playing ? 0 : 1);
        }

        bool get_quit_requested(){
            SDL_PollEvent(&sdl_event);
            return sdl_event.type == SDL_QUIT;
        }

        int get_key(){
            int r = -1;
            SDL_PumpEvents();

            if(keys[SDL_SCANCODE_0]) r = 0;
            else if(keys[SDL_SCANCODE_1]) r = 1;
            else if(keys[SDL_SCANCODE_2]) r = 2;
            else if(keys[SDL_SCANCODE_3]) r = 3;
            else if(keys[SDL_SCANCODE_4]) r = 4;
            else if(keys[SDL_SCANCODE_5]) r = 5;
            else if(keys[SDL_SCANCODE_6]) r = 6;
            else if(keys[SDL_SCANCODE_7]) r = 7;
            else if(keys[SDL_SCANCODE_8]) r = 8;
            else if(keys[SDL_SCANCODE_9]) r = 9;
            else if(keys[SDL_SCANCODE_A]) r = 10;
            else if(keys[SDL_SCANCODE_B]) r = 11;
            else if(keys[SDL_SCANCODE_C]) r = 12;
            else if(keys[SDL_SCANCODE_D]) r = 13;
            else if(keys[SDL_SCANCODE_E]) r = 14;
            else if(keys[SDL_SCANCODE_F]) r = 15;

            return r;
        }
        
        void draw(int x, int y, uint8_t r, uint8_t g, uint8_t b){
            SDL_Rect rect;
            rect.x = x * scale;
            rect.y = y * scale;
            rect.w = scale;
            rect.h = scale;

            SDL_SetRenderDrawColor(sdl_renderer, r, g, b, 0x00);
            SDL_RenderFillRect(sdl_renderer, &rect);
        }

        void clear(uint8_t r, uint8_t g, uint8_t b){
            SDL_SetRenderDrawColor(sdl_renderer, r, g, b, 0x00);
            SDL_RenderClear(sdl_renderer);
        }

        void refresh(){
            if(SDL_GetTicks64() >= time_to_refresh){
                SDL_SetRenderTarget(sdl_renderer, NULL);
                SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
                SDL_RenderPresent(sdl_renderer);
                SDL_SetRenderTarget(sdl_renderer, sdl_texture);

                time_to_refresh = SDL_GetTicks64() + frame_time;
            }
        }
};
