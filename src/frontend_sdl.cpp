#include <array>
#include <exception>
#include <stdexcept>
#include <SDL2/SDL.h>
#include <iostream>
#include <cmath>

double audio_offset = 0;
std::array<uint8_t, 16> audio_pattern;
double audio_frequency = 4000.0;

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
            sdl_audio_spec.freq = (4096*8);
            sdl_audio_spec.format = AUDIO_U8;
            sdl_audio_spec.channels = 1;
            sdl_audio_spec.samples = 1024;
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
            audio_pattern.fill(0xf);

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
            (void)userdata;
            for(int i = 0; i < len; i++){
                int offset = audio_offset;
                stream[i] = (audio_pattern.at(offset >> 3) >> ((offset & 7) ^ 7)) & 1 ? 0x50 : 0x00;
                audio_offset = fmod(audio_offset + (audio_frequency / (4096*8)), 128.0);
            }
        }

        static void set_audio_frequency(double frequency){
            audio_frequency = frequency;
        }

        static void set_audio_pattern(size_t i, uint8_t p){
            audio_pattern.at(i) = p;
        }

        /// start or stop the audio
        void set_audio_state(bool playing){
            SDL_PauseAudioDevice(sdl_audio_device_id, playing ? 0 : 1);
        }

        void poll_event(){
            SDL_PollEvent(&sdl_event);
        }

        bool get_quit_requested(){
            return sdl_event.type == SDL_QUIT;
        }

        template<class chip8> void get_keys(chip8 &c8){
            
            if(sdl_event.type == SDL_KEYDOWN || sdl_event.type == SDL_KEYUP){
                bool pressed = sdl_event.key.state == SDL_PRESSED;
                int key = -1, kb = 1;

                switch(sdl_event.key.keysym.scancode){
                    // keyboard 1
                    case SDL_SCANCODE_0: key = 0; kb = 1; break;
                    case SDL_SCANCODE_1: key = 1; kb = 1; break;
                    case SDL_SCANCODE_2: key = 2; kb = 1; break;
                    case SDL_SCANCODE_3: key = 3; kb = 1; break;
                    case SDL_SCANCODE_4: key = 4; kb = 1; break;
                    case SDL_SCANCODE_5: key = 5; kb = 1; break;
                    case SDL_SCANCODE_6: key = 6; kb = 1; break;
                    case SDL_SCANCODE_7: key = 7; kb = 1; break;
                    case SDL_SCANCODE_8: key = 8; kb = 1; break;
                    case SDL_SCANCODE_9: key = 9; kb = 1; break;
                    case SDL_SCANCODE_A: key = 10; kb = 1; break;
                    case SDL_SCANCODE_B: key = 11; kb = 1; break;
                    case SDL_SCANCODE_C: key = 12; kb = 1; break;
                    case SDL_SCANCODE_D: key = 13; kb = 1; break;
                    case SDL_SCANCODE_E: key = 14; kb = 1; break;
                    case SDL_SCANCODE_F: key = 15; kb = 1; break;
                    // keyboard 2
                    /* TODO
                    case SDL_SCANCODE_0: key = 0; kb = 2; break;
                    case SDL_SCANCODE_1: key = 1; kb = 2; break;
                    case SDL_SCANCODE_2: key = 2; kb = 2; break;
                    case SDL_SCANCODE_3: key = 3; kb = 2; break;
                    case SDL_SCANCODE_4: key = 4; kb = 2; break;
                    case SDL_SCANCODE_5: key = 5; kb = 2; break;
                    case SDL_SCANCODE_6: key = 6; kb = 2; break;
                    case SDL_SCANCODE_7: key = 7; kb = 2; break;
                    case SDL_SCANCODE_8: key = 8; kb = 2; break;
                    case SDL_SCANCODE_9: key = 9; kb = 2; break;
                    case SDL_SCANCODE_A: key = 10; kb = 2; break;
                    case SDL_SCANCODE_B: key = 11; kb = 2; break;
                    case SDL_SCANCODE_C: key = 12; kb = 2; break;
                    case SDL_SCANCODE_D: key = 13; kb = 2; break;
                    case SDL_SCANCODE_E: key = 14; kb = 2; break;
                    case SDL_SCANCODE_F: key = 15; kb = 2; break;
                    */
                    default: break;
                }

                if(key >= 0){
                    c8.set_key(kb, key, pressed);
                }
            }
        }

        void draw(int x, int y, std::array<uint8_t, 3> color){
            SDL_Rect rect;
            rect.x = x * scale;
            rect.y = y * scale;
            rect.w = scale;
            rect.h = scale;

            SDL_SetRenderDrawColor(sdl_renderer, color.at(0), color.at(1), color.at(2), 0x00);
            SDL_RenderFillRect(sdl_renderer, &rect);
        }

        void clear(std::array<uint8_t, 3> color){
            SDL_SetRenderDrawColor(sdl_renderer, color.at(0), color.at(1), color.at(2), 0x00);
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
