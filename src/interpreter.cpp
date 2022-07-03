#include <iomanip>
#include <string>
#include <exception>
#include <stdexcept>
#include <cstdlib>
#include <cmath>

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "instruction_set.cpp"
#include "quirks.cpp"
#include "hardware.cpp"
#include "palette.cpp"

namespace chip8{
    template<class instruction_set, class quirks, class hardware> class chip8_interpreter : public hardware, public quirks, public instruction_set{
        protected:
            bool skip_instruction;

            void bcd_of_v(uint8_t x){
                std::stringstream s_stream;
                s_stream << std::setw(3) << std::setfill('0') << std::dec << (int)hardware::registers.at(x);
                std::string str = s_stream.str();

                hardware::memory.at(hardware::register_I) = std::stoi(str.substr(0, 1));
                hardware::memory.at(hardware::register_I + 1) = std::stoi(str.substr(1, 1));
                hardware::memory.at(hardware::register_I + 2) = std::stoi(str.substr(2, 1));
            }

            /// draw a pixel
            template<class frontend> void draw_pixel(frontend &f, int plane, int x, int y){
                hardware::screen_set(plane, x, y, hardware::screen_get(plane, x, y) ^ 0x01, f);

                if(!hardware::screen_get(plane, x, y)){
                    if(hardware::high_res && quirks::quirk_dxyn_count_collisions_highres){
                        hardware::registers.at(0xf)++;
                    }else{
                        hardware::registers.at(0xf) = 0x01;
                    }
                }
            }

            /// draw a sprite
            template<class frontend> void draw(frontend &f, uint8_t opcode_x, uint8_t opcode_y, uint8_t opcode_n){
                // number of rows in the sprite
                const unsigned int rows = [=,this]{
                    if(opcode_n == 0 && quirks::quirk_dxy0_16x16_highres && hardware::high_res){ // 16x16 sprite
                        return 16;
                    }else if(opcode_n == 0 && quirks::quirk_dxy0_8x16_lowres && !hardware::high_res){ // 8x16 sprite
                        return 16;
                    }else if(opcode_n == 0 && quirks::quirk_dxy0_16x16_lowres && !hardware::high_res){ // 16x16 sprite
                        return 16;
                    }else{
                        return (int)opcode_n;
                    }
                }();
                
                // bytes per row
                const unsigned int bytes_per_row = [=,this]{
                    if(opcode_n == 0 && quirks::quirk_dxy0_16x16_highres && hardware::high_res){
                        return 2;
                    }else if(opcode_n == 0 && quirks::quirk_dxy0_16x16_lowres && !hardware::high_res){
                        return 2;
                    }else{
                        return 1;
                    }
                }();

                // 4 screen pixels per sprite pixel ?
                const bool scale_up = (hardware::allow_high_res && !hardware::high_res);

                // distance between sprite pixels
                const unsigned int stride = scale_up ? 2 : 1;

                // x coordinate of the sprite
                const unsigned int sprite_x = scale_up ? hardware::registers.at(opcode_x) * 2 : hardware::registers.at(opcode_x);
                
                // y coordinate of the sprite
                const unsigned int sprite_y = scale_up ? hardware::registers.at(opcode_y) * 2 : hardware::registers.at(opcode_y);

                int sprite_index = hardware::register_I;

                // draw the sprite
                hardware::registers.at(0xf) = 0x00;

                for(unsigned int plane = 0; plane < hardware::screen_planes; plane++){
                    if(!hardware::active_screen_planes.at(plane)) continue;

                    unsigned int y = quirks::quirk_dxyn_no_wrapping ? sprite_y : sprite_y % hardware::screen_y;

                    for(unsigned int row = 0; row < rows; row++){
                        if(y >= hardware::screen_y) break;

                        unsigned int x = quirks::quirk_dxyn_no_wrapping ? sprite_x : sprite_x % hardware::screen_x;

                        for(unsigned int byte = 0; byte < bytes_per_row; byte++){
                            for(unsigned int column = 0; column < 8; column++){
                                if(x >= hardware::screen_x) break;

                                if((hardware::memory.at(sprite_index) << column) & 0x80){
                                    draw_pixel(f, plane, x, y);

                                    if(scale_up){
                                        draw_pixel(f, plane, x + 1, y);
                                        draw_pixel(f, plane, x, y + 1);
                                        draw_pixel(f, plane, x + 1, y + 1);
                                    }
                                }

                                x += stride;
                                x = quirks::quirk_dxyn_no_wrapping ? x : x % hardware::screen_x;
                            }

                            sprite_index++;
                        }

                        y += stride;
                        y = quirks::quirk_dxyn_no_wrapping ? y : y % hardware::screen_y;
                    }
                }
            }

            /// clear the screen
            template<class frontend> void clear_screen(frontend &f, bool force_all_planes=false){
                for(unsigned int plane = 0; plane < hardware::screen_planes; plane++){
                    if(!hardware::active_screen_planes.at(plane) && !force_all_planes) continue;
                    
                    for(unsigned int x = 0; x < hardware::screen_x; x++){
                        for(unsigned int y = 0; y < hardware::screen_y; y++){
                            if(hardware::screen_content.at(plane).at(y).at(x)){
                                hardware::screen_content.at(plane).at(y).at(x) = 0x00;
                                f.draw(x, y, hardware::palette.color(this, x, y));
                            }
                        }
                    }
                }
            }

            template<class frontend> void scroll_up(frontend &f, unsigned int n){
                for(unsigned int plane = 0; plane < hardware::screen_planes; plane++){
                    if(!hardware::active_screen_planes.at(plane)) continue;

                    for(unsigned int y = 0 ; y < hardware::screen_y - n;  y++){
                        hardware::screen_content.at(plane).at(y) = hardware::screen_content.at(plane).at(y + n);

                        for(unsigned int x = 0; x < hardware::screen_x; x++){
                            f.draw(x, y, hardware::palette.color(this, x, y));
                        }
                    }
                    for(unsigned int y = hardware::screen_y - n; y < hardware::screen_y; y++){
                        for(unsigned int x = 0; x < hardware::screen_content.at(plane).at(y).size(); x++){
                            hardware::screen_content.at(plane).at(y).at(x) = 0x00;
                        }
                        for(unsigned int x = 0; x < hardware::screen_x; x++){
                            f.draw(x, y, hardware::palette.color(this, x, y));
                        }
                    }
                }
            }

            template<class frontend> void scroll_down(frontend &f, unsigned int n){
                for(unsigned int plane = 0; plane < hardware::screen_planes; plane++){
                    if(!hardware::active_screen_planes.at(plane)) continue;

                    for(unsigned int y = hardware::screen_y - 1 ; y >= n;  y--){
                        hardware::screen_content.at(plane).at(y) = hardware::screen_content.at(plane).at(y - n);
                        
                        for(unsigned int x = 0; x < hardware::screen_x; x++){
                            f.draw(x, y, hardware::palette.color(this, x, y));
                        }
                    }
                    for(unsigned int y = 0; y < n; y++){
                        for(unsigned int x = 0; x < hardware::screen_content.at(plane).at(y).size(); x++){
                            hardware::screen_content.at(plane).at(y).at(x) = 0x00;
                        }
                        for(unsigned int x = 0; x < hardware::screen_x; x++){
                            f.draw(x, y, hardware::palette.color(this, x, y));
                        }
                    }
                }
            }

            template<class frontend> void scroll_right(frontend &f){
                for(unsigned int plane = 0; plane < hardware::screen_planes; plane++){
                    if(!hardware::active_screen_planes.at(plane)) continue;

                    for(unsigned int y = 0; y < hardware::screen_y; y++){
                        int x = hardware::screen_x - 5;
                        while(x >= 0){
                            hardware::screen_content.at(plane).at(y).at(x + 4) = hardware::screen_content.at(plane).at(y).at(x);
                            f.draw(x + 4, y, hardware::palette.color(this, x + 4, y));
                            x--;
                        }

                        hardware::screen_content.at(plane).at(y).at(0) = 0x00;
                        hardware::screen_content.at(plane).at(y).at(1) = 0x00;
                        hardware::screen_content.at(plane).at(y).at(2) = 0x00;
                        hardware::screen_content.at(plane).at(y).at(3) = 0x00;
                        f.draw(0, y, hardware::palette.color(this, 0, y));
                        f.draw(1, y, hardware::palette.color(this, 1, y));
                        f.draw(2, y, hardware::palette.color(this, 2, y));
                        f.draw(3, y, hardware::palette.color(this, 3, y));
                    }
                }
            }

            template<class frontend> void scroll_left(frontend &f){
                for(unsigned int plane = 0; plane < hardware::screen_planes; plane++){
                    if(!hardware::active_screen_planes.at(plane)) continue;

                    for(unsigned int y = 0; y < hardware::screen_y; y++){
                        unsigned int x = 4;
                        while(x < hardware::screen_x){
                            hardware::screen_content.at(plane).at(y).at(x - 4) = hardware::screen_content.at(plane).at(y).at(x);
                            f.draw(x - 4, y, hardware::palette.color(this, x - 4, y));
                            x++;
                        }

                        hardware::screen_content.at(plane).at(y).at(x - 4) = 0x00;
                        hardware::screen_content.at(plane).at(y).at(x - 3) = 0x00;
                        hardware::screen_content.at(plane).at(y).at(x - 2) = 0x00;
                        hardware::screen_content.at(plane).at(y).at(x - 1) = 0x00;
                        f.draw(x - 4, y, hardware::palette.color(this, x - 4, y));
                        f.draw(x - 3, y, hardware::palette.color(this, x - 3, y));
                        f.draw(x - 2, y, hardware::palette.color(this, x - 2, y));
                        f.draw(x - 1, y, hardware::palette.color(this, x - 1, y));
                    }
                }
            }
        
        public:
            chip8_interpreter(lua_State *L) : hardware(L){
                skip_instruction = false;
            }

            template<class frontend> void frontend_init(frontend &f){
                f.clear(hardware::palette.bg_color(this));
            }

            void print(std::ostream &outstream){
                outstream << "hardware:\n";
                hardware::print(outstream);

                outstream << "\ninstruction set:\n";
                instruction_set::print(outstream);

                outstream << "\nquirks:\n";
                quirks::print(outstream);
            }

            /// execute one instruction at pc and increment pc
            template<class frontend> int execute(frontend &f){
                int return_value = 1;
                bool matched_opcode;

                // decrement timers
                std::chrono::time_point<std::chrono::steady_clock> timer_now = std::chrono::steady_clock::now();
                if(std::chrono::duration_cast<std::chrono::microseconds>(timer_now - hardware::timer_start).count() >= hardware::timer_delay){
                    hardware::timer_start = timer_now;
                    if(hardware::sound_timer == 1) f.set_audio_state(false);
                    hardware::delay_timer = hardware::delay_timer > 0 ? hardware::delay_timer - 1 : 0;
                    hardware::sound_timer = hardware::sound_timer > 0 ? hardware::sound_timer - 1 : 0;
                }

                // do nothing if we are waiting for a keypress (on keyboard 1)
                if(hardware::waiting_for_key >= 0){
                    int key = -1;
                    for(unsigned int i = 0; i < hardware::keyboard_1.size(); i++){
                        if(hardware::keyboard_1.at(i)) key = i;
                    }
                    
                    if(key < 0){
                        return return_value;
                    }else{
                        hardware::registers.at(hardware::waiting_for_key) = key;
                        hardware::waiting_for_key = -1;
                    }
                }

                // or waiting for the timer to reach 0
                if(hardware::waiting_for_timer && hardware::delay_timer > 0x00){
                    return return_value;
                }else if(hardware::waiting_for_timer){
                    hardware::waiting_for_timer = false;
                }

                // skip this instruction?
                if(skip_instruction){
                    skip_instruction = false;

                    // 32 bit instruction?
                    if constexpr(instruction_set::xochip){
                        if(hardware::memory.at(hardware::pc) == 0xf0 && hardware::memory.at(hardware::pc + 1) == 0x00){
                            hardware::pc += 2;
                        }
                    }

                    hardware::pc += 2;
                }

                // get opcode from memory
                uint8_t high = hardware::memory.at(hardware::pc), low = hardware::memory.at(hardware::pc + 1);
                uint8_t high_h = (high >> 4), high_l = high & 0x0f, low_h = low >> 4, low_l = low & 0x0f;
                uint16_t opcode = (high << 8) | low;

                // increment pc
                hardware::pc += 2;

                // decode opcode
                if constexpr(instruction_set::chip8e){
                    matched_opcode = true;

                    // 00ed - stop execution (CHIP-8E)
                    if(opcode == 0x00ed){
                        return_value = 0;

                    // 00f2 - no operation (CHIP-8E)
                    }else if(opcode == 0x00f2){

                    // 0151 - wait until the delay timer reaches 0 (CHIP-8E)
                    }else if(opcode == 0x0151){
                        hardware::waiting_for_timer = true;
                    
                    // 0188 - skip the next instruction (CHIP-8E)
                    }else if(opcode == 0x0188){
                        skip_instruction = true;
                    
                    // 5xy1 - skip if Vx > Vy (CHIP-8E)
                    }else if(high_h == 0x05 && low_l == 0x01){
                        if(hardware::registers.at(high_l) > hardware::registers.at(low_h)) skip_instruction = true;
                    
                    // 5xy2 - store Vx to Vy in memory starting at I; I = I + x + 1 (CHIP-8E)
                    }else if(high_h == 0x05 && low_l == 0x02){
                        for(uint8_t i = high_l; i <= low_h; i++){
                            hardware::memory.at(hardware::register_I) = hardware::registers.at(i);
                            hardware::register_I++;
                        }
                    
                    // 5xy3 - load Vx to Vy from memory starting at I; I = I + x + 1 (CHIP-8E)
                    }else if(high_h == 0x05 && low_l == 0x03){
                        for(uint8_t i = high_l; i <= low_h; i++){
                            hardware::registers.at(i) = hardware::memory.at(hardware::register_I);
                            hardware::register_I++;
                        }
                    
                    // bbnn - jump to current instruction - nn bytes (CHIP-8E)
                    }else if(high == 0xbb){
                        hardware::pc -= 2;
                        hardware::pc -= low;
                    
                    // bfnn - jump to current instruction + nn bytes (CHIP-8E)
                    }else if(high == 0xbf){
                        hardware::pc -= 2;
                        hardware::pc += low;

                    // fx03 - send Vx to output port 3 (CHIP-8E)
                    }else if(high_h == 0x0f && low == 0x03){
                        throw std::runtime_error("opcode fx03 is not implemented");
                    
                    // fx1b - skip Vx bytes (CHIP-8E)
                    }else if(high_h == 0x0f && low == 0x1b){
                        hardware::pc += hardware::registers.at(high_l);
                    
                    // fx4f - delay timer = Vx; wait until the delay timer reaches 0 (CHIP-8E)
                    }else if(high_h == 0x0f && low == 0x4f){
                        hardware::delay_timer = hardware::registers.at(high_l);
                        hardware::waiting_for_timer = true;
                    
                    // fxe3 - wait for strobe at EF4; read Vx from input port 3 (CHIP-8E)
                    }else if(high_h == 0x0f && low == 0xe3){
                        throw std::runtime_error("opcode fxe3 is not implemented");
                    
                    // fxe7 - read Vx from input port 3 (CHIP-8E)
                    }else if(high_h == 0x0f && low == 0xe7){
                        throw std::runtime_error("opcode fxe7 is not implemented");

                    }else{
                        matched_opcode = false;
                    }
                    if(matched_opcode) return return_value;
                }

                if constexpr(instruction_set::super_chip_1_0){
                    matched_opcode = true;

                    // 00fd - stop execution (SUPER-CHIP 1.0)
                    if(opcode == 0x00fd){
                        return_value = 0;
                    
                    // 00fe - disable high resolution mode (SUPER-CHIP 1.0)
                    }else if(opcode == 0x00fe){
                        hardware::high_res = false;
                        if constexpr(quirks::quirk_00fe_00ff_clear_screen){
                            clear_screen(f, quirks::quirk_00fe_00ff_clear_all_planes);
                        }
                    
                    // 00ff - enable high resolution mode (SUPER-CHIP 1.0)
                    }else if(opcode == 0x00ff){
                        if(hardware::allow_high_res) hardware::high_res = true;
                        if constexpr(quirks::quirk_00fe_00ff_clear_screen){
                            clear_screen(f, quirks::quirk_00fe_00ff_clear_all_planes);
                        }

                    // fx75 - store V0 - Vx in RPL user flags (0 <= x <= 7) (SUPER-CHIP 1.0)
                    }else if(high_h == 0x0f && low == 0x75){
                        if constexpr(!quirks::quirk_fx75_fx85_allow_all){
                            if(high_l >= 8){
                                throw std::runtime_error("invalid usage of opcode fx75");
                            }
                        }

                        for(uint8_t i = 0; i <= high_l; i++){
                            hardware::flag_registers.at(i) = hardware::registers.at(i);
                        }
                    
                    // fx85 - load V0 - Vx from RPL user flags (0 <= x <= 7) (SUPER-CHIP 1.0)
                    }else if(high_h == 0x0f && low == 0x85){
                        if constexpr(!quirks::quirk_fx75_fx85_allow_all){
                            if(high_l >= 8){
                                throw std::runtime_error("invalid usage of opcode fx85");
                            }
                        }

                        for(uint8_t i = 0; i <= high_l; i++){
                            hardware::registers.at(i) = hardware::flag_registers.at(i);
                        }

                    }else{
                        matched_opcode = false;
                    }
                    if(matched_opcode) return return_value;
                }

                if constexpr(instruction_set::super_chip_1_1){
                    matched_opcode = true;

                    // 00cn - scroll display n pixels down (SUPER-CHIP 1.1)
                    if(high == 0x00 && low_h == 0x0c){
                        scroll_down(f, quirks::quirk_lowres_double_scroll && hardware::allow_high_res && !hardware::high_res ? low_l * 2 : low_l);
                    
                    // 00fb - scroll display 4 pixels right (SUPER-CHIP 1.1)
                    }else if(opcode == 0x00fb){
                        if(quirks::quirk_lowres_double_scroll && hardware::allow_high_res && !hardware::high_res) scroll_right(f);
                        scroll_right(f);
                    
                    // 00fc - scroll display 4 pixels left (SUPER-CHIP 1.1)
                    }else if(opcode == 0x00fc){
                        if(quirks::quirk_lowres_double_scroll && hardware::allow_high_res && !hardware::high_res) scroll_left(f);
                        scroll_left(f);
                    
                    // fx30 - I = address of large sprite of digit in Vx (SUPER-CHIP 1.1)
                    }else if(high_h == 0x0f && low == 0x30){
                        if(hardware::registers.at(high_l) < 10){
                            hardware::register_I = 80 + (hardware::registers.at(high_l)) * 10;
                        }else{
                            throw std::runtime_error("invalid usage of opcode fx30");
                        }

                    }else{
                        matched_opcode = false;
                    }
                    if(matched_opcode) return return_value;
                }

                if constexpr(instruction_set::set_rd0_fxf2){
                    // fxf2 - set the RD.0 register to x
                    if(high_h == 0x0f && low == 0xf2){
                        hardware::register_rd0 = high_l;

                        return return_value;
                    }
                }

                if constexpr(instruction_set::scroll_up_00bn){
                    // 00bn - scroll display n pixels up
                    if(high_h == 0x00 && low_h == 0x0b){
                        scroll_up(f, quirks::quirk_lowres_double_scroll && hardware::allow_high_res && !hardware::high_res ? low_l * 2 : low_l);
                        return return_value;
                    }
                }

                if constexpr(instruction_set::chip8x){
                    matched_opcode = true;
                    
                    // 02a0 - step background color (CHIP-8X)
                    if(opcode == 0x02a0){
                        hardware::screen_bg_color = (hardware::screen_bg_color + 1) % 4;
                        for(unsigned int y = 0; y < hardware::screen_y; y++){
                            for(unsigned int x = 0; x < hardware::screen_x; x++){
                                if(!hardware::screen_get(0, x, y)) f.draw(x, y, hardware::palette.color(this, x, y));
                            }
                        }
                    
                    // 5xy1 - for each nibble in Vx, Vy: Vx = (Vx + Vy) % 8 (CHIP-8X)
                    }else if(high_h == 0x05 && low_l == 0x01){
                        uint8_t result1 = ((hardware::registers.at(high_l) >> 4) + (hardware::registers.at(low_h) >> 4)) % 8;
                        uint8_t result2 = ((hardware::registers.at(high_l) & 0x0f) + (hardware::registers.at(low_h) & 0x0f)) % 8;
                        hardware::registers.at(high_l) = (result1 << 4) | result2;

                    // bxy0 - set foreground color in area given by Vx and Vx+1 to Vy (CHIP-8X)
                    }else if(high_h == 0x0b && low_l == 0x00){
                        // size of the zones
                        int zone_x = hardware::screen_x / 8;
                        int zone_y = hardware::screen_y / 8;

                        // start coordinates
                        unsigned int x0 = (hardware::registers.at(high_l) & 0x0f) * zone_x;
                        unsigned int y0 = (hardware::registers.at((high_l + 1) % hardware::registers.size()) & 0x0f) * zone_y;

                        // end coordinates
                        unsigned int x1 = x0 + zone_x + (hardware::registers.at(high_l) >> 4) * zone_x;
                        unsigned int y1 = y0 + zone_y + (hardware::registers.at((high_l + 1) % hardware::registers.size()) >> 4) * zone_y;

                        uint8_t color = hardware::registers.at(low_h);
                        if(color > 8){
                            throw std::runtime_error("invalid usage of opcode bxy0");
                        }
                        
                        for(unsigned int x = x0; x < x1; x++){
                            if(x >= hardware::screen_x) break;

                            for(unsigned int y = y0; y < y1; y++){
                                if(y >= hardware::screen_y) break;

                                hardware::screen_fg_color.at(y).at(x) = color;
                                if(hardware::screen_get(0, x, y)) f.draw(x, y, hardware::palette.color(this, x, y));
                            }
                        }
                        
                    // bxyn - set foreground color at Vx,Vx+1 for n rows to Vy (CHIP-8X)
                    }else if(high_h == 0x0b){
                        // start coordinates
                        unsigned int x0 = hardware::registers.at(high_l);
                        unsigned int y0 = hardware::registers.at((high_l + 1) % hardware::registers.size());

                        uint8_t color = hardware::registers.at(low_h);
                        if(color > 8){
                            throw std::runtime_error("invalid usage of opcode bxyn");
                        }
                        
                        for(unsigned int x = x0; x < x0 + 8; x++){
                            if(x >= hardware::screen_x) break;

                            for(unsigned int y = y0; y < y0 + low_l; y++){
                                if(y >= hardware::screen_y) break;

                                hardware::screen_fg_color.at(y).at(x) = color;
                                if(hardware::screen_get(0, x, y)) f.draw(x, y, hardware::palette.color(this, x, y));
                            }
                        }

                    // exf2 - skip if key Vx is pressed on keyboard 2 == Vx (CHIP-8X)
                    }else if(high_h == 0x0e && low == 0xf2){
                        if(hardware::keyboard_2.at(hardware::registers.at(high_l))) skip_instruction = true;
                    
                    // exf5 - skip if key Vx is not pressed on keyboard 2 == Vx (CHIP-8X)
                    }else if(high_h == 0x0e && low == 0xf5){
                        if(!hardware::keyboard_2.at(hardware::registers.at(high_l))) skip_instruction = true;

                    
                    // fxf8 - output Vx to port (set sound frequency) (CHIP-8X)
                    }else if(high_h == 0x0f && low == 0xf8){
                        f.set_audio_frequency((27535 / (hardware::registers.at(high_l) + 1)) * 8);

                    // fxfb - wait for input from port and store it in Vx (CHIP-8X)
                    }else if(high_h == 0x0f && low == 0xfb){
                        throw std::runtime_error("opcode fxfb is not implemented");
                    
                    }else{
                        matched_opcode = false;
                    }
                    if(matched_opcode) return return_value;
                }

                if constexpr(instruction_set::xochip){
                    matched_opcode = true;
                    
                    // 00dn - scroll display n pixels up (XO-CHIP)
                    if(high == 0x00 && low_h == 0x0d){
                        scroll_up(f, quirks::quirk_lowres_double_scroll && hardware::allow_high_res && !hardware::high_res ? low_l * 2 : low_l);

                    // 5xy2 - save Vx to Vy (ascending or descending) in memory starting at I (XO-CHIP)
                    }else if(high_h == 0x05 && low_l == 0x02){
                        int i = hardware::register_I;
                        if(high_l <= low_h){
                            for(int j = high_l; j <= low_h; j++){
                                hardware::memory.at(i) = hardware::registers.at(j);
                                i++;
                            }
                        }else{
                            for(int j = high_l; j >= low_h; j--){
                                hardware::memory.at(i) = hardware::registers.at(j);
                                i++;
                            }
                        }

                    // 5xy3 - load Vx to Vy (ascending or descending) from memory starting at I (XO-CHIP)
                    }else if(high_h == 0x05 && low_l == 0x03){
                        int i = hardware::register_I;
                        if(high_l <= low_h){
                            for(int j = high_l; j <= low_h; j++){
                                hardware::registers.at(j) = hardware::memory.at(i);
                                i++;
                            }
                        }else{
                            for(int j = high_l; j >= low_h; j--){
                                hardware::registers.at(j) = hardware::memory.at(i);
                                i++;
                            }
                        }

                    // f000 nnnn - I = nnnn (XO-CHIP)
                    }else if(opcode == 0xf000){
                        hardware::register_I = (hardware::memory.at(hardware::pc) << 8) | hardware::memory.at(hardware::pc + 1);
                        hardware::pc += 2;
                    
                    // fn01 - set active drawing planes to n (XO-CHIP)
                    }else if(high_h == 0x0f && low == 0x01){
                        switch(high_l){
                            case 0x00: hardware::active_screen_planes.at(0) = false; hardware::active_screen_planes.at(1) = false; break;
                            case 0x01: hardware::active_screen_planes.at(0) = true;  hardware::active_screen_planes.at(1) = false; break;
                            case 0x02: hardware::active_screen_planes.at(0) = false; hardware::active_screen_planes.at(1) = true;  break;
                            case 0x03: hardware::active_screen_planes.at(0) = true;  hardware::active_screen_planes.at(1) = true;  break;
                            default: throw std::runtime_error("invalid usage of opcode fn01");
                        }

                    // f002 - store 16 bytes starting at I in the audio pattern buffer (XO-CHIP)
                    }else if(opcode == 0xf002){
                        for(int i = 0; i < 16; i++){
                            f.set_audio_pattern(i, hardware::memory.at(hardware::register_I + i));
                        }
                    
                    // fx3a - pitch register = Vx (XO-CHIP)
                    }else if(high_h == 0x0f && low == 0x3a){
                        f.set_audio_frequency(4000.0 * exp2((hardware::registers.at(low_h) - 64) / 48.0));

                    }else{
                        matched_opcode = false;
                    }
                    if(matched_opcode) return return_value;
                }

                if constexpr(instruction_set::stop_0000){
                    // 0000 - stop execution
                    if(opcode == 0x0000){
                        return 0;
                    }
                }

                if constexpr(instruction_set::chip8run){
                    matched_opcode = true;

                    // 001n - stop execution with exit status n (chip8run)
                    if(high == 0x00 && low_h == 0x0c){
                        return_value = 0;
                    
                    // 00fa - set override_fx55_fx65_no_increment to true (chip8run)
                    }else if(opcode == 0x00fa){
                        quirks::override_fx55_fx65_no_increment = true;
                    
                    }else{
                        matched_opcode = false;
                    }
                    if(matched_opcode) return return_value;
                }

                // 00e0 - clear screen
                if(opcode == 0x00e0){
                    clear_screen(f);
                
                // 00ee - return
                }else if(opcode == 0x00ee){
                    if(hardware::call_stack.size() == 0) throw std::runtime_error("call stack empty - can not return");
                    hardware::pc = hardware::call_stack.top();
                    hardware::call_stack.pop();
                
                // 0nnn - call machine language subroutine at nnn
                }else if(high_h == 0x00){
                    throw std::runtime_error("opcode 0nnn is not implemented");
                
                // 1nnn - jump to nnn
                }else if(high_h == 0x01){
                    hardware::pc = ((high_l << 8) | low);
                
                // 2nnn - call subroutine at nnn
                }else if(high_h == 0x02){
                    hardware::call_stack.push(hardware::pc);
                    hardware::pc = ((high_l << 8) | low);
                
                // 3xnn - skip if Vx == nn
                }else if(high_h == 0x03){
                    if(hardware::registers.at(high_l) == low) skip_instruction = true;
                
                // 4xnn - skip if Vx != nn
                }else if(high_h == 0x04){
                    if(hardware::registers.at(high_l) != low) skip_instruction = true;
                
                // 5xy0 - skip if Vx == Vy
                }else if(high_h == 0x05 && low_l == 0x00){
                    if(hardware::registers.at(high_l) == hardware::registers.at(low_h)) skip_instruction = true;
                
                // 6xnn - Vx = nn
                }else if(high_h == 0x06){
                    hardware::registers.at(high_l) = low;
                
                // 7xnn - Vx += nn
                }else if(high_h == 0x07){
                    hardware::registers.at(high_l) += low;
                
                // 8xy0 - Vx = Vy
                }else if(high_h == 0x08 && low_l == 0x00){
                    hardware::registers.at(high_l) = hardware::registers.at(low_h);
                
                // 8xy1 - Vx |= Vy
                }else if(high_h == 0x08 && low_l == 0x01){
                    hardware::registers.at(high_l) |= hardware::registers.at(low_h);
                
                // 8xy2 - Vx &= Vy
                }else if(high_h == 0x08 && low_l == 0x02){
                    hardware::registers.at(high_l) &= hardware::registers.at(low_h);
                    
                // 8xy3 - Vx ^= Vy
                }else if(high_h == 0x08 && low_l == 0x03){
                    hardware::registers.at(high_l) ^= hardware::registers.at(low_h);

                // 8xy4 - Vx += Vy; Vf = carry ? 1 : 0
                }else if(high_h == 0x08 && low_l == 0x04){
                    uint8_t result;
                    result = hardware::registers.at(high_l) + hardware::registers.at(low_h);
                    uint8_t F = result <= hardware::registers.at(high & 0xf) && hardware::registers.at(low_h) > 0 ? 0x01 : 0x00;
                    hardware::registers.at(high_l) = result;
                    hardware::registers.at(0xf) = F;

                // 8xy5 - Vx -= Vy; Vf = borrow ? 0 : 1
                }else if(high_h == 0x08 && low_l == 0x05){
                    uint8_t result;
                    result = hardware::registers.at(high_l) - hardware::registers.at(low_h);
                    uint8_t F = result >= hardware::registers.at(high & 0xf) && hardware::registers.at(low_h) > 0 ? 0x00 : 0x01;
                    hardware::registers.at(high_l) = result;
                    hardware::registers.at(0xf) = F;

                // 8xy6 - Vx = Vy >> 1; Vf = Vy & 0x01 
                }else if(high_h == 0x08 && low_l == 0x06){
                    uint8_t F;
                    if constexpr(quirks::quirk_8xy6_8xye_shift_vx){
                        F = hardware::registers.at(high_l) & 0x01;
                        hardware::registers.at(high_l) = hardware::registers.at(high_l) >> 1;
                    }else{
                        F = hardware::registers.at(low_h) & 0x01;
                        hardware::registers.at(high_l) = hardware::registers.at(low_h) >> 1;
                    }
                    hardware::registers.at(0xf) = F;

                // 8xy7 - Vx = Vy - Vx; Vf = borrow ? 0 : 1
                }else if(high_h == 0x08 && low_l == 0x07){
                    uint8_t result;
                    result = hardware::registers.at(low_h) - hardware::registers.at(high_l);
                    uint8_t F = result >= hardware::registers.at(low_h) && hardware::registers.at(high_l) > 0 ? 0x00 : 0x01;
                    hardware::registers.at(high_l) = result;
                    hardware::registers.at(0xf) = F;

                // 8xye - Vx = Vy << 1; Vf = Vy & 0x80
                }else if(high_h == 0x08 && low_l == 0x0e){
                    uint8_t F;
                    if constexpr(quirks::quirk_8xy6_8xye_shift_vx){
                        F = hardware::registers.at(high_l) & 0x80;
                        hardware::registers.at(high_l) = hardware::registers.at(high_l) << 1;
                    }else{
                        F = hardware::registers.at(low_h) & 0x80;
                        hardware::registers.at(high_l) = hardware::registers.at(low_h) << 1;
                    }
                    hardware::registers.at(0xf) = F ? 1 : 0;
                
                // 9xy0 - skip if Vx != Vy
                }else if(high_h == 0x09 && low_l == 0x00){
                    if(hardware::registers.at(high_l) != hardware::registers.at(low_h)) skip_instruction = true;

                // annn - I = nnn
                }else if(high_h == 0x0a){
                    hardware::register_I = ((high_l << 8) | low);

                // bnnn - jump to nnn + V0
                }else if(high_h == 0x0b){
                    if constexpr(quirks::quirk_bnnn_bxnn_use_vx){
                        hardware::pc = ((high_l << 8) | low) + hardware::registers.at(high_l);
                    }else if constexpr(quirks::quirk_bnnn_use_rd0){
                        hardware::pc = ((high_l << 8) | low) + hardware::registers.at(hardware::register_rd0);
                    }else{
                        hardware::pc = ((high_l << 8) | low) + hardware::registers.at(0);
                    }

                // cxnn - Vx = random & nn
                }else if(high_h == 0x0c){
                    hardware::registers.at(high& 0x0f) = std::rand() & low;
                
                // dxyn - draw n bytes at (Vx, Vy)
                }else if(high_h == 0x0d){
                    draw(f, high_l, low_h, low_l);

                // ex9e - skip if key Vx is pressed
                }else if(high_h == 0x0e && low == 0x9e){
                    if(hardware::keyboard_1.at(hardware::registers.at(high_l))) skip_instruction = true;

                // exa1 - skip if key Vx is not pressed
                }else if(high_h == 0x0e && low == 0xa1){
                    if(!hardware::keyboard_1.at(hardware::registers.at(high_l))) skip_instruction = true;

                // fx07 - Vx = delay timer
                }else if(high_h == 0x0f && low == 0x07){
                    hardware::registers.at(high_l) = hardware::delay_timer;
                
                // fx0a - wait for keypress; Vx = key
                }else if(high_h == 0x0f && low == 0x0a){
                    hardware::waiting_for_key = high_l;

                // fx15 - delay timer = Vx
                }else if(high_h == 0x0f && low == 0x15){
                    hardware::delay_timer = hardware::registers.at(high_l);
                
                // fx18 - sound timer = Vx
                }else if(high_h == 0x0f && low == 0x18){
                    hardware::sound_timer = hardware::registers.at(high_l);
                    if(hardware::sound_timer > 1) f.set_audio_state(true);
                
                // fx1e - I += Vx
                }else if(high_h == 0x0f && low == 0x1e){
                    uint16_t old_I = hardware::register_I;
                    hardware::register_I += hardware::registers.at(high_l);
                    if(quirks::quirk_fx1e_overflow_at_memory_size){
                        hardware::register_I %= hardware::memory.size();
                    }
                    if(quirks::quirk_fx1e_set_vf){
                        hardware::registers.at(0xf) = (hardware::register_I < old_I) ? 1 : 0;
                    }
                
                // fx29 - I = address of sprite of hex digit in Vx
                }else if(high_h == 0x0f && low == 0x29){
                    if(quirks::quirk_fx29_digits_highres && hardware::registers.at(high_l) >= 0x10 && hardware::registers.at(high_l) <= 0x19){
                        hardware::register_I = 80 + (hardware::registers.at(high_l) & 0x0f) * 10;
                    }else{
                        hardware::register_I = (hardware::registers.at(high_l) & 0x0f) * 5;
                    }
                
                // fx33 - memory[I, I+1, I+2] = BCD of Vx
                }else if(high_h == 0x0f && low == 0x33){
                    bcd_of_v(high_l);

                // fx55 - store V0 to Vx in memory starting at I; I = I + x + 1
                }else if(high_h == 0x0f && low == 0x55){
                    for(uint8_t i = (quirks::quirk_fx55_fx65_use_rd0 ? hardware::register_rd0 : 0); i <= (high_l); i++){
                        hardware::memory.at(hardware::register_I) = hardware::registers.at(i);
                        hardware::register_I++;
                    }
                    
                    if constexpr(quirks::quirk_fx55_fx65_increment_less)
                        hardware::register_I--;
                    else if(quirks::quirk_fx55_fx65_no_increment || quirks::override_fx55_fx65_no_increment)
                        hardware::register_I -= (high_l + 1);

                // fx65 - load V0 to Vx from memory starting at I; I = I + x + 1
                }else if(high_h == 0x0f && low == 0x65){
                    for(uint8_t i = (quirks::quirk_fx55_fx65_use_rd0 ? hardware::register_rd0 : 0); i <= high_l; i++){
                        hardware::registers.at(i) = hardware::memory.at(hardware::register_I);
                        hardware::register_I++;
                    }

                    if constexpr(quirks::quirk_fx55_fx65_increment_less)
                        hardware::register_I--;
                    else if(quirks::quirk_fx55_fx65_no_increment || quirks::override_fx55_fx65_no_increment)
                        hardware::register_I -= (high_l + 1);

                }else{
                    throw std::runtime_error("unkown opcode");
                }

                return return_value;
            }
    };

    typedef chip8_interpreter<chip8_instruction_set<false, false, false, false, false, false, false, false, false>, quirks_chip8, chip8_hardware<chip8_palette>> chip8;
    typedef chip8_interpreter<chip8_instruction_set<false, false, false, false, false, false, false, false, false>, quirks_chip8, chip8_hardware<chip8_palette>> chip10;
    typedef chip8_interpreter<chip8_instruction_set<true,  false, false, false, false, false, false, false, false>, quirks_chip8, chip8_hardware<chip8_palette>> chip8e;
    typedef chip8_interpreter<chip8_instruction_set<false, false, false, false, true,  false, false, false, false>, quirks_chip8_fxf2_fx55_fx65, chip8_hardware<chip8_palette>> chip8_fxf2_fx55_fx65;
    typedef chip8_interpreter<chip8_instruction_set<false, false, false, false, true,  false, false, false, false>, quirks_chip8_fxf2_bnnn, chip8_hardware<chip8_palette>> chip8_fxf2_bnnn;
    typedef chip8_interpreter<chip8_instruction_set<false, false, false, false, true,  false, false, false, false>, quirks_chip8_fxf2, chip8_hardware<chip8_palette>> chip8_fxf2;
    typedef chip8_interpreter<chip8_instruction_set<false, false, false, false, false, false, false, false, false>, quirks_chip48, chip8_hardware<chip8_palette>> chip48;
    typedef chip8_interpreter<chip8_instruction_set<false, true,  false, false, false, false, false, false, false>, quirks_schip10, chip8_hardware<chip8_palette>> schip10;
    typedef chip8_interpreter<chip8_instruction_set<false, true,  true,  false, false, false, false, false, false>, quirks_schip11, chip8_hardware<chip8_palette>> schip11;
    typedef chip8_interpreter<chip8_instruction_set<false, true,  true,  false, false, false, false, false, false>, quirks_schpc, chip8_hardware<chip8_palette>> schpc;
    typedef chip8_interpreter<chip8_instruction_set<false, true,  true,  false, false, false, false, false, false>, quirks_schip11_fx1e, chip8_hardware<chip8_palette>> schip11_fx1e;
    typedef chip8_interpreter<chip8_instruction_set<false, true,  true,  false, false, false, false, false, false>, quirks_schpc_fx1e, chip8_hardware<chip8_palette>> schpc_fx1e;
    typedef chip8_interpreter<chip8_instruction_set<false, true,  true,  true , false, false, false, false, false>, quirks_schip11, chip8_hardware<chip8_palette>> schip11scu;
    typedef chip8_interpreter<chip8_instruction_set<false, false, false, false, false, true , false, false, false>, quirks_chip8, chip8_hardware<chip8x_palette>> chip8x;
    typedef chip8_interpreter<chip8_instruction_set<false, true,  true,  false, false, false, true,  false, false>, quirks_xochip, chip8_hardware<xochip_palette>> xochip;
    typedef chip8_interpreter<chip8_instruction_set<false, true,  true,  false, false, false, true,  true,  false>, quirks_octo, chip8_hardware<xochip_palette>> octo;
    typedef chip8_interpreter<chip8_instruction_set<false, true,  true,  false, false, false, false, false, false>, quirks_chip8run, chip8_hardware<chip8_palette>> chip8run;
}
