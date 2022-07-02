#include <vector>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <ncpp/NotCurses.hh>

class frontend_sdl{
    protected:
        unsigned int screen_width, screen_height;

    private:
        struct notcurses* nc;
        struct ncplane* nc_plane;
        struct ncplane* nc_std;
        int last_key, last_kb;

    public:
        frontend_sdl(int render_width, int render_height, int scale, int refresh_rate){
            (void)scale;
            (void)refresh_rate;

            screen_width = render_width * 2;
            screen_height = render_height;

            last_key = -1;
            last_kb = -1;
        
            // set locale
            if(setlocale(LC_ALL, "") == nullptr){
                throw std::runtime_error("setlocale failed");
            }

            // notcurses init
            notcurses_options ncopts{};
            ncopts.flags |= NCOPTION_SUPPRESS_BANNERS;
            nc = notcurses_init(&ncopts, NULL);
            if(nc == nullptr){
                throw std::runtime_error("notcurses_init failed");
            }

            // create plane
            unsigned int x, y;
            nc_std = notcurses_stddim_yx(nc, &y, &x);
            struct ncplane_options plane_opts = {
                .y = NCALIGN_CENTER,
                .x = NCALIGN_CENTER,
                .rows = screen_height,
                .cols = screen_width,
                .userptr = nullptr,
                .name = "chip8",
                .resizecb = nullptr,
                .flags = NCPLANE_OPTION_HORALIGNED | NCPLANE_OPTION_VERALIGNED,
                .margin_b = 0, .margin_r = 0,
            };
            nc_plane = ncplane_create(nc_std, &plane_opts);
            if(!nc_plane){
                throw std::runtime_error("ncplane_create failed");
            }
        }

        ~frontend_sdl(){
            notcurses_stop(nc);
        }

        void set_audio_state(bool playing){
            (void)playing;
        }

        void poll_event(){
        }

        bool get_quit_requested(){
            return false;
        }

        template<class chip8> void get_keys(chip8 &c8){
            if(last_kb > 0 && last_key >= 0){
                c8.set_key(last_kb, last_key, false);
            }

            ncinput nc_input;
            bool pressed, set_last = false;
            int key = -1, kb = 1;

            uint32_t event = notcurses_get(nc, nullptr, &nc_input);
            
            if(nc_input.evtype == NCTYPE_PRESS || nc_input.evtype == NCTYPE_REPEAT){
                pressed = true;
            }else if(nc_input.evtype == NCTYPE_RELEASE){
                pressed = false;
            }else{
                pressed = true;
                set_last = true;
            }

            switch(event){
                // keyboard 1
                case '0': key = 0; kb = 1; break;
                case '1': key = 1; kb = 1; break;
                case '2': key = 2; kb = 1; break;
                case '3': key = 3; kb = 1; break;
                case '4': key = 4; kb = 1; break;
                case '5': key = 5; kb = 1; break;
                case '6': key = 6; kb = 1; break;
                case '7': key = 7; kb = 1; break;
                case '8': key = 8; kb = 1; break;
                case '9': key = 9; kb = 1; break;
                case 'a': key = 10; kb = 1; break;
                case 'b': key = 11; kb = 1; break;
                case 'c': key = 12; kb = 1; break;
                case 'd': key = 13; kb = 1; break;
                case 'e': key = 14; kb = 1; break;
                case 'f': key = 15; kb = 1; break;
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
                
                if(set_last){
                    last_kb = kb;
                    last_key = key;
                }
            }
        }

        void draw(int x, int y, std::array<uint8_t, 3> color){
            ncplane_set_bg_rgb8(nc_plane, color.at(0), color.at(1), color.at(2));
            ncplane_putchar_yx(nc_plane, y, x * 2, ' ');
            ncplane_putchar_yx(nc_plane, y, x * 2 + 1, ' ');
        }

        void clear(std::array<uint8_t, 3> color){
            ncplane_set_bg_rgb8(nc_plane, color.at(0), color.at(1), color.at(2));
            for(unsigned int y = 0; y < screen_height; y++){
                for(unsigned int x = 0; x < screen_width; x++){
                    ncplane_putchar_yx(nc_plane, y, x, ' ');
                }
            }
        }

        void refresh(){
            notcurses_render(nc);
        }
};
