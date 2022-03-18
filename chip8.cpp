#include <array>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stack>
#include <stdexcept>
#include <string>
#include <thread>
#include <cstring>

#include "frontend_sdl.cpp"

namespace chip8{

    /** The available instruction set extensions.
    The instruction set only determines which opcodes are available, not their behaviour.
    */
    template<
        bool t_chip8e,
        bool t_super_chip_1_0,
        bool t_super_chip_1_1
    > class chip8_instruction_set{
        public:
            /// the CHIP-8E extension
            static constexpr bool chip8e = t_chip8e;
            /// the SUPER-CHIP 1.0 extension
            static constexpr bool super_chip_1_0 = t_super_chip_1_0;
            /// the SUPER-CHIP 1.1 extension (without SUPER-CHIP 1.0 opcodes)
            static constexpr bool super_chip_1_1 = t_super_chip_1_1;
    };

    /** The available quirks.
    These change the emulation behaviour.
    */
    template<
        bool t_bnnn_bxnn_use_vx,
        bool t_fx55_fx65_increment_less,
        bool t_fx55_fx65_no_increment,
        bool t_8xy6_8xye_shift_vx,
        bool t_dxy0_16x16_highres,
        bool t_dxy0_8x16_lowres,
        bool t_fx29_digits_highres,
        bool t_dxyn_count_collisions_highres,
        bool t_dxyn_no_wrapping
    > class chip8_quirks{
        public:
            /// bnnn (bxnn): jump to xnn + Vx instead of xnn + V0
            static constexpr bool quirk_bnnn_bxnn_use_vx = t_bnnn_bxnn_use_vx;
            /// fx55 and fx65: increment I by x instead of x+1
            static constexpr bool quirk_fx55_fx65_increment_less = t_fx55_fx65_increment_less;
            /// fx55 and fx65: don't increment I
            static constexpr bool quirk_fx55_fx65_no_increment = t_fx55_fx65_no_increment;
            /// 8xy6 and 8xye: shift Vx instead of Vy
            static constexpr bool quirk_8xy6_8xye_shift_vx = t_8xy6_8xye_shift_vx;
            /// dxyn (dxy0): draw a 16x16 sprite in high resolution mode
            static constexpr bool quirk_dxy0_16x16_highres = t_dxy0_16x16_highres;
            /// dxyn (dxy0): draw a 8x16 sprite in low resolution mode
            static constexpr bool quirk_dxy0_8x16_lowres = t_dxy0_8x16_lowres;
            /// fx29: point to 10 byte sprites for the digits 0-9 when 10 <= x <= 19
            static constexpr bool quirk_fx29_digits_highres = t_fx29_digits_highres;
            /// dxyn: in high resolution mode: set Vf to the number of rows that have a collision or are clipped at the bottom
            static constexpr bool quirk_dxyn_count_collisions_highres = t_dxyn_count_collisions_highres;
            /// dxyn: disable sprite wrapping
            static constexpr bool quirk_dxyn_no_wrapping = t_dxyn_no_wrapping;
    };

    typedef chip8_quirks<false, false, false, false, false, false, false, false, false> quirks_chip8;
    typedef chip8_quirks<true,  true,  false, true,  false, false, false, false, true > quirks_chip48;
    typedef chip8_quirks<true,  true,  false, true,  true,  true,  true,  false, true > quirks_schip10;
    typedef chip8_quirks<true,  false, true,  true,  true,  true,  false, true,  true > quirks_schip11;
    typedef chip8_quirks<false, false, false, false, true,  false, false, false, false> quirks_schpc;

    template<size_t memory_size, int t_screen_x, int t_screen_y, bool t_allow_high_res> class chip8_hardware {
        protected:
            static constexpr std::array<uint8_t, 80> font = {{
                0xf0, 0x90, 0x90, 0x90, 0xf0,
                0x20, 0x60, 0x20, 0x20, 0x70,
                0xf0, 0x10, 0xf0, 0x80, 0xf0,
                0xf0, 0x10, 0xf0, 0x10, 0xf0,
                0x90, 0x90, 0xf0, 0x10, 0x10,
                0xf0, 0x80, 0xf0, 0x10, 0xf0,
                0xf0, 0x80, 0xf0, 0x90, 0xf0,
                0xf0, 0x10, 0x20, 0x40, 0x40,
                0xf0, 0x90, 0xf0, 0x90, 0xf0,
                0xf0, 0x90, 0xf0, 0x10, 0xf0,
                0xf0, 0x90, 0xf0, 0x90, 0x90,
                0xe0, 0x90, 0xe0, 0x90, 0xe0,
                0xf0, 0x80, 0x80, 0x80, 0xf0,
                0xe0, 0x90, 0x90, 0x90, 0xe0,
                0xf0, 0x80, 0xf0, 0x80, 0xf0,
                0xf0, 0x80, 0xf0, 0x80, 0x80,
            }};

            static constexpr std::array<uint8_t, 100> big_font = {{
                0x3c, 0x7e, 0xe7, 0xc3, 0xc3, 0xc3, 0xc3, 0xe7, 0x7e, 0x3c,
                0x18, 0x38, 0x58, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c,
                0x3e, 0x7f, 0xc3, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xff, 0xff,
                0x3c, 0x7e, 0xc3, 0x03, 0x0e, 0x0e, 0x03, 0xc3, 0x7e, 0x3c,
                0x06, 0x0e, 0x1e, 0x36, 0x66, 0xc6, 0xff, 0xff, 0x06, 0x06,
                0xff, 0xff, 0xc0, 0xc0, 0xfc, 0xfe, 0x03, 0xc3, 0x7e, 0x3c,
                0x3e, 0x7c, 0xe0, 0xc0, 0xfc, 0xfe, 0xc3, 0xc3, 0x7e, 0x3c,
                0xff, 0xff, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x60, 0x60,
                0x3c, 0x7e, 0xc3, 0xc3, 0x7e, 0x7e, 0xc3, 0xc3, 0x7e, 0x3c,
                0x3c, 0x7e, 0xc3, 0xc3, 0x7f, 0x3f, 0x03, 0x03, 0x3e, 0x7c,
            }};
            
            // main memory
            std::array<uint8_t, memory_size> memory;
            
            // registers V0-Vf
            std::array<uint8_t, 16> registers;
            
            // register I (16 bit for a memory address)
            uint16_t register_I = 0x0000;

            // flag registers (used only for e.g. SUPER-CHIP)
            std::array<uint8_t, 8> flag_registers;
            
            // timers
            static constexpr int timer_delay = 1000000 / 60; // microseconds
            std::chrono::time_point<std::chrono::steady_clock> timer_start;
            uint8_t delay_timer = 0x00, sound_timer = 0x00;
            bool waiting_for_timer = false;
            
            // screen content
            static const int screen_x = t_screen_x;
            static const int screen_y = t_screen_y;
            std::array<std::array<uint8_t, screen_x>, screen_y> screen_content;
            static const bool allow_high_res = t_allow_high_res;
            bool high_res = false; // high resolution mode for SUPER-CHIP
            
            // program counter
            uint16_t pc = 0x200;
            
            // call stack (for returning from subroutines)
            std::stack< uint16_t > call_stack;

            // currently pressed key
            int pressed_key = -1;
            int waiting_for_key = -1;

            uint8_t screen_get(size_t x, size_t y){
                return screen_content.at(y).at(x);
            }

            template<class frontend> void screen_set(size_t x, size_t y, uint8_t value, frontend &f){
                screen_content.at(y).at(x) = value;

                if(value){
                    f.draw(x, y, 0xff, 0xff, 0xff);
                }else{
                    f.draw(x, y, 0x00, 0x00, 0x00);
                }
            }
        
        public:
            chip8_hardware(){
                memory.fill(0x00);
                registers.fill(0x00);
                flag_registers.fill(0x00);
                
                for(auto &i : screen_content){
                    i.fill(0x00);
                }

                // font
                for(size_t i = 0; i < font.size(); i++){
                    memory.at(i) = font.at(i);
                }

                // big font
                for(size_t i = 80; i < big_font.size(); i++){
                    memory.at(i) = big_font.at(i);
                }

                timer_start = std::chrono::steady_clock::now();
            }

            /// load file into memory
            int load_binary(std::string file_path){
                std::ifstream instream(file_path, std::ios::in | std::ios::binary);
                if(!instream.is_open()) return 1;

                uint16_t address = pc;
                while(address < memory_size){
                    uint8_t i = instream.get();
                    if(instream.eof()) break;
                    memory.at(address) = i;
                    address++;
                }
                instream.close();
                
                return 0;
            }

            void set_key(int key){
                pressed_key = key;
            }

            int get_screen_x(){
                return screen_x;
            }

            int get_screen_y(){
                return screen_y;
            }

            /* debug functions
            void print_screen(std::ostream &outstream){
                std::cout << "\e[H\e[2J";

                for(size_t x = 0; x < screen_content.size(); x++){
                    for(size_t y = 0; y < screen_content.at(0).size(); y++){
                        outstream << (screen_content.at(x).at(y) > 0 ? "██" : "  ");
                    }
                    outstream << "\n";
                }
            }

            void print_registers(std::ostream &outstream){
                outstream << "I=" << std::setw(4) << std::setfill('0') << std::hex << register_I << " ";
                for(int i = 0; i < 16; i++)
                    outstream << "V" << std::hex << i << "=" << std::setw(2) << std::setfill('0') << std::hex << (int)registers.at(i) << " ";
                outstream << "delay=" << std::setw(2) << std::setfill('0') << std::hex << (int)delay_timer << " ";
                outstream << "sound=" << std::setw(2) << std::setfill('0') << std::hex << (int)sound_timer << " ";
                outstream << "pc=" << std::setw(4) << std::setfill('0') << std::hex << pc << "\n";
            }
            */
    };

    template<class instruction_set, class quirks, class hardware> class chip8_interpreter : public hardware, public quirks, public instruction_set{
        protected:
            void bcd_of_v(uint8_t x){
                std::stringstream s_stream;
                s_stream << std::setw(3) << std::setfill('0') << std::dec << (int)hardware::registers.at(x);
                std::string str = s_stream.str();

                hardware::memory.at(hardware::register_I) = std::stoi(str.substr(0, 1));
                hardware::memory.at(hardware::register_I + 1) = std::stoi(str.substr(1, 1));
                hardware::memory.at(hardware::register_I + 2) = std::stoi(str.substr(2, 1));
            }

            // updates screen_content
            template<class frontend> void draw(frontend &f, uint8_t x, uint8_t y, uint8_t n){
                if(n == 0){

                    if(quirks::quirk_dxy0_16x16_highres && hardware::high_res){ // 16x16 sprite
                        hardware::registers.at(0xf) = 0x00;

                        for(int i = 0; i < 16; i++){
                            if(quirks::quirk_dxyn_no_wrapping && hardware::registers.at(y) + i >= hardware::screen_y){
                                if(quirks::quirk_dxyn_count_collisions_highres) hardware::registers.at(0xf) += (16 - i);
                                break;
                            }
                            int Y = (hardware::registers.at(y) + i) % hardware::screen_y;

                            for(int j = 0; j < 8; j++){
                                if(quirks::quirk_dxyn_no_wrapping && hardware::registers.at(x) + j >= hardware::screen_x){
                                    break;
                                }

                                int X = (hardware::registers.at(x) + j) % hardware::screen_x;
                                
                                if((hardware::memory.at(hardware::register_I + (2 * i)) << j) & 0x80){
                                    hardware::screen_set(X, Y, hardware::screen_get(X, Y) ^ 0x01, f);
                                    
                                    if(quirks::quirk_dxyn_count_collisions_highres && !hardware::screen_get(X, Y)){
                                        hardware::registers.at(0xf)++;
                                    }else if(quirks::quirk_dxyn_count_collisions_highres && !hardware::screen_get(X, Y)){
                                        hardware::registers.at(0xf) = 0x01;
                                    }
                                }
                            }
                            for(int j = 0; j < 8; j++){
                                if(quirks::quirk_dxyn_no_wrapping && hardware::registers.at(x) + j + 8 >= hardware::screen_x){
                                    break;
                                }

                                int X = (hardware::registers.at(x) + j + 8) % hardware::screen_x;
                                
                                if((hardware::memory.at(hardware::register_I + (2 * i) + 1) << j) & 0x80){
                                    hardware::screen_set(X, Y, hardware::screen_get(X, Y) ^ 0x01, f);
                                    
                                    if(quirks::quirk_dxyn_count_collisions_highres && !hardware::screen_get(X, Y)){
                                        hardware::registers.at(0xf)++;
                                    }else if(quirks::quirk_dxyn_count_collisions_highres && !hardware::screen_get(X, Y)){
                                        hardware::registers.at(0xf) = 0x01;
                                    }
                                }
                            }
                        }

                        return;

                    }else if(quirks::quirk_dxy0_8x16_lowres && !hardware::high_res){ // 8x16 sprite
                        n = 16;
                    
                    }else{ // 0 row sprite
                        return;
                    }
                }
                
                if(hardware::allow_high_res && !hardware::high_res){ // double the coordinates in low resolution mode
                    int Y = hardware::registers.at(y) % (hardware::screen_y / 2);
                    int X;
                    hardware::registers.at(0xf) = 0x00;

                    for(uint8_t N = 0; N < n; N++){
                        for(int i = 0; i < 8; i++){
                            if(quirks::quirk_dxyn_no_wrapping && (hardware::registers.at(x) + i) >= (hardware::screen_x / 2)) break;
                            
                            X = (hardware::registers.at(x) + i) % (hardware::screen_x / 2);

                            if((hardware::memory.at(hardware::register_I + N) << i) & 0x80){
                                hardware::screen_set(X * 2, Y * 2, hardware::screen_get(X * 2, Y * 2) ^ 0x01, f);
                                if(!hardware::screen_get(X * 2, Y * 2)) hardware::registers.at(0xf) = 0x01;
                                hardware::screen_set(X * 2, Y * 2 + 1, hardware::screen_get(X * 2, Y * 2 + 1) ^ 0x01, f);
                                if(!hardware::screen_get(X * 2, Y * 2 + 1)) hardware::registers.at(0xf) = 0x01;
                                hardware::screen_set(X * 2 + 1, Y * 2, hardware::screen_get(X * 2 + 1, Y * 2) ^ 0x01, f);
                                if(!hardware::screen_get(X * 2 + 1, Y * 2)) hardware::registers.at(0xf) = 0x01;
                                hardware::screen_set(X * 2 + 1, Y * 2 + 1, hardware::screen_get(X * 2 + 1, Y * 2 + 1) ^ 0x01, f);
                                if(!hardware::screen_get(X * 2 + 1, Y * 2 + 1)) hardware::registers.at(0xf) = 0x01;
                            }
                        }

                        if(quirks::quirk_dxyn_no_wrapping && (Y + 1) >= hardware::screen_y) break;

                        Y = (Y + 1) % (hardware::screen_y / 2);
                    }
                }else{
                    int Y = hardware::registers.at(y) % hardware::screen_y;
                    int X;
                    hardware::registers.at(0xf) = 0x00;

                    for(uint8_t N = 0; N < n; N++){
                        for(int i = 0; i < 8; i++){
                            if(quirks::quirk_dxyn_no_wrapping && (hardware::registers.at(x) + i) >= hardware::screen_x) break;

                            X = (hardware::registers.at(x) + i) % hardware::screen_x;
                            
                            if((hardware::memory.at(hardware::register_I + N) << i) & 0x80){
                                hardware::screen_set(X, Y, hardware::screen_get(X, Y) ^ 0x01, f);

                                if(quirks::quirk_dxyn_count_collisions_highres && !hardware::screen_get(X, Y)){
                                    hardware::registers.at(0xf)++;
                                }else if(!hardware::screen_get(X, Y)){
                                    hardware::registers.at(0xf) = 0x01;
                                }
                            }
                        }

                        if(quirks::quirk_dxyn_no_wrapping && (Y + 1) >= hardware::screen_y){
                            hardware::registers.at(0xf) += (N - 1 - n);
                            break;
                        }

                        Y = (Y + 1) % hardware::screen_y;
                    }
                }
            }
        
        public:
            /// execute one instruction at pc and increment pc
            template<class frontend> int execute(frontend &f){
                int return_value = 1;

                // decrement timers
                std::chrono::time_point<std::chrono::steady_clock> timer_now = std::chrono::steady_clock::now();
                if(std::chrono::duration_cast<std::chrono::microseconds>(timer_now - hardware::timer_start).count() >= hardware::timer_delay){
                    hardware::timer_start = timer_now;
                    hardware::delay_timer = hardware::delay_timer > 0 ? hardware::delay_timer - 1 : 0;
                    hardware::sound_timer = hardware::sound_timer > 0 ? hardware::sound_timer - 1 : 0;
                }

                // do nothing if we are waiting for a keypress
                if(hardware::waiting_for_key >= 0 && hardware::pressed_key < 0){
                    return return_value;
                }else if(hardware::waiting_for_key >= 0){
                    hardware::registers.at(hardware::waiting_for_key) = hardware::pressed_key;
                    hardware::waiting_for_key = -1;
                }

                // or waiting for the timer to reach 0
                if(hardware::waiting_for_timer && hardware::delay_timer > 0x00){
                    return return_value;
                }else if(hardware::waiting_for_timer){
                    hardware::waiting_for_timer = false;
                }

                // get opcode from memory
                uint8_t high = hardware::memory.at(hardware::pc), low = hardware::memory.at(hardware::pc + 1);
                hardware::pc += 2;

                // temporary variable during operations
                uint8_t result = 0;

                // decode opcode
                switch(high & 0xf0){
                    case 0x00:
                        if constexpr(instruction_set::chip8e){
                            if(high == 0x00 && low == 0xed){
                                // 00ed - stop execution (CHIP-8E)
                                return_value = 0;
                                break;

                            }else if(high == 0x00 && low == 0xf2){
                                // 00f2 - no operation (CHIP-8E)
                                break;

                            }else if(high == 0x01 && low == 0x51){
                                // 0151 - wait until the delay timer reaches 0 (CHIP-8E)
                                hardware::waiting_for_timer = true;
                                break;

                            }else if(high == 0x01 && low == 0x88){
                                // 0188 - skip the next instruction (CHIP-8E)
                                hardware::pc += 2;
                                break;                                
                            }
                        }

                        if constexpr(instruction_set::super_chip_1_0){
                            if(high == 0x00 && low == 0xfd){
                                // 00fd - stop execution (SUPER-CHIP 1.0)
                                return_value = 0;
                                break;

                            }else if(high == 0x00 && low == 0xfe){
                                // 00fe - disable high resolution mode (SUPER-CHIP 1.0)
                                hardware::high_res = false;
                                break;

                            }else if(high == 0x00 && low == 0xff){
                                // 00ff - enable high resolution mode (SUPER-CHIP 1.0)
                                if(hardware::allow_high_res) hardware::high_res = true;
                                break;
                            }
                        }
                        
                        if constexpr(instruction_set::super_chip_1_1){
                            if(high == 0x00 && (low >> 4) == 0x0c){
                                // 00cn - scroll display n pixels down (SUPER-CHIP 1.1)
                                int n = (low & 0x0f);
                                for(int y = hardware::screen_content.size() - 1 ; y >= n;  y--){
                                    hardware::screen_content.at(y) = hardware::screen_content.at(y - n);
                                    
                                    for(int x = 0; x < hardware::screen_content.at(y).size(); x++){
                                        if(hardware::screen_content.at(y).at(x)){
                                            f.draw(x, y, 0xff, 0xff, 0xff);
                                        }else{
                                            f.draw(x, y, 0x00, 0x00, 0x00);
                                        }
                                    }
                                }
                                for(int y = 0; y < n; y++){
                                    hardware::screen_content.at(y).fill(0x00);
                                    for(int x = 0; x < hardware::screen_content.at(y).size(); x++){
                                        f.draw(x, y, 0x00, 0x00, 0x00);
                                    }
                                }
                                break;

                            }else if(high == 0x00 && low == 0xfb){
                                // 00fb - scroll display 4 pixels right (SUPER-CHIP 1.1)
                                for(int y = 0; y < hardware::screen_content.size(); y++){
                                    int x = hardware::screen_content.at(y).size() - 5;
                                    while(x >= 0){
                                        hardware::screen_content.at(y).at(x + 4) = hardware::screen_content.at(y).at(x);
                                        
                                        if(hardware::screen_content.at(y).at(x + 4)){
                                            f.draw(x + 4, y, 0xff, 0xff, 0xff);
                                        }else{
                                            f.draw(x + 4, y, 0x00, 0x00, 0x00);
                                        }

                                        x--;
                                    }

                                    hardware::screen_content.at(y).at(0) = 0x00;
                                    hardware::screen_content.at(y).at(1) = 0x00;
                                    hardware::screen_content.at(y).at(2) = 0x00;
                                    hardware::screen_content.at(y).at(3) = 0x00;
                                    f.draw(0, y, 0x00, 0x00, 0x00);
                                    f.draw(1, y, 0x00, 0x00, 0x00);
                                    f.draw(2, y, 0x00, 0x00, 0x00);
                                    f.draw(3, y, 0x00, 0x00, 0x00);
                                }
                                break;
                                
                            }else if(high == 0x00 && low == 0xfc){
                                // 00fc - scroll display 4 pixels left (SUPER-CHIP 1.1)
                                for(int y = 0; y < hardware::screen_content.size(); y++){
                                    int x = 4;
                                    while(x < hardware::screen_content.at(y).size()){
                                        hardware::screen_content.at(y).at(x - 4) = hardware::screen_content.at(y).at(x);
                                        
                                        if(hardware::screen_content.at(y).at(x - 4)){
                                            f.draw(x - 4, y, 0xff, 0xff, 0xff);
                                        }else{
                                            f.draw(x - 4, y, 0x00, 0x00, 0x00);
                                        }

                                        x++;
                                    }

                                    hardware::screen_content.at(y).at(x - 4) = 0x00;
                                    hardware::screen_content.at(y).at(x - 3) = 0x00;
                                    hardware::screen_content.at(y).at(x - 2) = 0x00;
                                    hardware::screen_content.at(y).at(x - 1) = 0x00;
                                    f.draw(x - 4, y, 0x00, 0x00, 0x00);
                                    f.draw(x - 3, y, 0x00, 0x00, 0x00);
                                    f.draw(x - 2, y, 0x00, 0x00, 0x00);
                                    f.draw(x - 1, y, 0x00, 0x00, 0x00);
                                }
                                break;
                            }
                        }

                        if(high == 0x00 && low == 0xe0){
                            // 00e0 - clear screen
                            for(auto &i : hardware::screen_content){
                                i.fill(0x00);
                            }
                            f.clear(0x00, 0x00, 0x00);

                        }else if(high == 0x00 && low == 0xee){
                            // 00ee - return
                            if(hardware::call_stack.size() == 0) throw std::runtime_error("call stack empty - can not return");
                            hardware::pc = hardware::call_stack.top();
                            hardware::call_stack.pop();

                        }else{
                            // 0nnn - call machine language subroutine at nnn
                            throw std::runtime_error("opcode 0nnn not implemented");
                        }
                        break;
                    
                    case 0x10: // 1nnn - jump to nnn
                        hardware::pc = (((high & 0x0f) << 8) | low);
                        break;
                    
                    case 0x20: // 2nnn - call subroutine at nnn
                        hardware::call_stack.push(hardware::pc);
                        hardware::pc = (((high & 0x0f) << 8) | low);
                        break;
                    
                    case 0x30: // 3xnn - skip if Vx == nn
                        if(hardware::registers.at(high & 0x0f) == low) hardware::pc += 2;
                        break;
                    
                    case 0x40: // 3xnn - skip if Vx != nn
                        if(hardware::registers.at(high & 0x0f) != low) hardware::pc += 2;
                        break;
                    
                    case 0x50:
                        if((low & 0x0f) == 0x00){
                            // 5xy0 - skip if Vx == Vy
                            if(hardware::registers.at(high & 0x0f) == hardware::registers.at(low >> 4)) hardware::pc += 2;

                        }else if(instruction_set::chip8e && (low & 0x0f) == 0x01){
                            // 5xy1 - skip if Vx > Vy (CHIP-8E)
                            if(hardware::registers.at(high & 0x0f) > hardware::registers.at(low >> 4)) hardware::pc += 2;

                        }else if(instruction_set::chip8e && (low & 0x0f) == 0x02){
                            // 5xy2 - store Vx to Vy in memory starting at I; I = I + x + 1 (CHIP-8E)
                            for(uint8_t i = (high & 0x0f); i <= (low >> 4); i++){
                                hardware::memory.at(hardware::register_I) = hardware::registers.at(i);
                                hardware::register_I++;
                            }

                        }else if(instruction_set::chip8e && (low & 0x0f) == 0x03){
                            // 5xy3 - load Vx to Vy from memory starting at I; I = I + x + 1 (CHIP-8E)
                            for(uint8_t i = (high & 0x0f); i <= (low >> 4); i++){
                                hardware::registers.at(i) = hardware::memory.at(hardware::register_I);
                                hardware::register_I++;
                            }

                        }else{
                            throw std::runtime_error("unkown opcode");
                        }
                        break;
                    
                    case 0x60: // 6xnn - Vx = nn
                        hardware::registers.at(high & 0x0f) = low;
                        break;
                    
                    case 0x70: // 6xnn - Vx += nn
                        hardware::registers.at(high & 0x0f) += low;
                        break;
                    
                    case 0x80:
                        switch(low & 0x0f){
                            case 0x00: // 8xy0 - Vx = Vy
                                hardware::registers.at(high & 0x0f) = hardware::registers.at(low >> 4);
                                break;

                            case 0x01: // 8xy1 - Vx |= Vy
                                hardware::registers.at(high & 0x0f) |= hardware::registers.at(low >> 4);
                                break;

                            case 0x02: // 8xy2 - Vx &= Vy
                                hardware::registers.at(high & 0x0f) &= hardware::registers.at(low >> 4);
                                break;
                            
                            case 0x03: // 8xy3 - Vx ^= Vy
                                hardware::registers.at(high & 0x0f) ^= hardware::registers.at(low >> 4);
                                break;
                            
                            case 0x04: // 8xy4 - Vx += Vy; Vf = carry ? 1 : 0
                                result = hardware::registers.at(high & 0x0f) + hardware::registers.at(low >> 4);
                                hardware::registers.at(0xf) = result <= hardware::registers.at(high & 0xf) && hardware::registers.at(low >> 4) > 0 ? 0x01 : 0x00;
                                hardware::registers.at(high & 0x0f) = result;
                                break;
                            
                            case 0x05: // 8xy5 - Vx -= Vy; Vf = borrow ? 0 : 1
                                result = hardware::registers.at(high & 0x0f) - hardware::registers.at(low >> 4);
                                hardware::registers.at(0xf) = result >= hardware::registers.at(high & 0xf) && hardware::registers.at(low >> 4) > 0 ? 0x00 : 0x01;
                                hardware::registers.at(high & 0x0f) = result;
                                break;
                            
                            case 0x06: // 8xy6 - Vx = Vy >> 1; Vf = Vy & 0x01
                                if constexpr(quirks::quirk_8xy6_8xye_shift_vx){
                                    hardware::registers.at(0xf) = hardware::registers.at(high & 0x0f) & 0x01;
                                    hardware::registers.at(high & 0x0f) = hardware::registers.at(high & 0x0f) >> 1;
                                }else{
                                    hardware::registers.at(0xf) = hardware::registers.at(low >> 4) & 0x01;
                                    hardware::registers.at(high & 0x0f) = hardware::registers.at(low >> 4) >> 1;
                                }
                                break;
                            
                            case 0x07: // 8xy7 - Vx = Vy - Vx; Vf = borrow ? 0 : 1
                                result = hardware::registers.at(low >> 4) - hardware::registers.at(high & 0x0f);
                                hardware::registers.at(0xf) = result >= hardware::registers.at(low >> 4) && hardware::registers.at(high & 0x0f) > 0 ? 0x00 : 0x01;
                                hardware::registers.at(high & 0x0f) = result;
                                break;
                            
                            case 0x0e: // 8xye - Vx = Vy << 1; Vf = Vy & 0x80
                                if constexpr(quirks::quirk_8xy6_8xye_shift_vx){
                                    hardware::registers.at(0xf) = hardware::registers.at(high & 0x0f) & 0x80;
                                    hardware::registers.at(high & 0x0f) = hardware::registers.at(high & 0x0f) << 1;
                                }else{
                                    hardware::registers.at(0xf) = hardware::registers.at(low >> 4) & 0x80;
                                    hardware::registers.at(high & 0x0f) = hardware::registers.at(low >> 4) << 1;
                                }
                                break;
                            
                            default:
                                throw std::runtime_error("unkown opcode");
                                break;
                        }
                        break;
                    
                    case 0x90: // 5xy0 - skip if Vx != Vy
                        if((low & 0x0f) == 0x00){
                            if(hardware::registers.at(high & 0x0f) != hardware::registers.at(low >> 4)) hardware::pc += 2;
                        }else{
                            throw std::runtime_error("unkown opcode");
                        }
                        break;
                    
                    case 0xa0: // annn - I = nnn
                        hardware::register_I = (((high & 0x0f) << 8) | low);
                        break;
                    
                    case 0xb0:
                        if(instruction_set::chip8e && (high & 0x0f) == 0x0b){
                            // bbnn - jump to current instruction - nn bytes (CHIP-8E)
                            hardware::pc -= 2;
                            hardware::pc -= low;

                        }else if(instruction_set::chip8e && (high & 0x0f) == 0x0f){
                            // bfnn - jump to current instruction + nn bytes (CHIP-8E)
                            hardware::pc -= 2;
                            hardware::pc += low;
                            
                        }else{
                            // bnnn - jump to nnn + V0
                            if constexpr(quirks::quirk_bnnn_bxnn_use_vx)
                                hardware::pc = (((high & 0x0f) << 8) | low) + hardware::registers.at(high & 0x0f);
                            else
                                hardware::pc = (((high & 0x0f) << 8) | low) + hardware::registers.at(0);
                        }
                        break;
                    
                    case 0xc0: // cxnn - Vx = random & nn
                        hardware::registers.at(high& 0x0f) = std::rand() & low;
                        break;
                    
                    case 0xd0: // dxyn - draw n bytes at (Vx, Vy)
                        draw(f, (high & 0x0f), (low >> 4), (low & 0x0f));
                        break;
                    
                    case 0xe0:
                        switch(low){
                            case 0x9e: // ex9e - skip if pressed key == Vx
                                if(hardware::pressed_key == hardware::registers.at(high & 0x0f)) hardware::pc += 2;
                                break;

                            case 0xa1: // exa1 - skip if pressed key != Vx
                                if(hardware::pressed_key != hardware::registers.at(high & 0x0f)) hardware::pc += 2;
                                break;

                            default:
                                throw std::runtime_error("unkown opcode");
                                break;
                        }
                        break;
                    
                    case 0xf0:
                        if constexpr(instruction_set::chip8e){
                            bool outer_break = true;
                            switch(low){
                                case 0x03: // fx03 - send Vx to output port 3 (CHIP-8E)
                                    throw std::runtime_error("opcode fx03 not implemented");
                                    break;
                                
                                case 0x1b: // fx1b - skip Vx bytes (CHIP-8E)
                                    hardware::pc += hardware::registers.at(high & 0x0f);
                                    break;
                                
                                case 0x4f: // fx4f - delay timer = Vx; wait until the delay timer reaches 0 (CHIP-8E)
                                    hardware::delay_timer = hardware::registers.at(high & 0x0f);
                                    hardware::waiting_for_timer = true;
                                    break;
                                
                                case 0xe3: // fxe3 - wait for strobe at EF4; read Vx from input port 3 (CHIP-8E)
                                    throw std::runtime_error("opcode fxe3 not implemented");
                                    break;
                                
                                case 0xe7: // fxe7 - read Vx from input port 3 (CHIP-8E)
                                    throw std::runtime_error("opcode fxe7 not implemented");
                                    break;
                                
                                default:
                                    outer_break = false;
                                    break;
                            }

                            if(outer_break){
                                break;
                            }
                        }

                        if constexpr(instruction_set::super_chip_1_0){
                            bool outer_break = true;
                            switch(low){
                                case 0x75: // fx75 - store V0 - Vx in RPL user flags (0 <= x <= 7) (SUPER-CHIP 1.0)
                                    if((high & 0x0f) < 8){
                                        for(uint8_t i = 0; i <= (high & 0x0f); i++){
                                            hardware::flag_registers.at(i) = hardware::registers.at(i);
                                        }
                                    }else{
                                        throw std::runtime_error("invalid usage of opcode fx75");
                                    }
                                    break;
                                
                                case 0x85: // fx85 - load V0 - Vx from RPL user flags (0 <= x <= 7) (SUPER-CHIP 1.0)
                                    if((high & 0x0f) < 8){
                                        for(uint8_t i = 0; i <= (high & 0x0f); i++){
                                            hardware::registers.at(i) = hardware::flag_registers.at(i);
                                        }
                                    }else{
                                        throw std::runtime_error("invalid usage of opcode fx85");
                                    }
                                    break;

                                default:
                                    outer_break = false;
                                    break;
                            }

                            if(outer_break){
                                break;
                            }
                        }

                        if constexpr(instruction_set::super_chip_1_1){
                            if(low == 0x30){ // fx30 - I = address of large sprite of digit in Vx
                                if(hardware::registers.at(high & 0x0f) < 10){
                                    hardware::register_I = 80 + (hardware::registers.at(high & 0x0f)) * 10;
                                    break;

                                }else{
                                    throw std::runtime_error("invalid usage of opcode fx30");
                                }
                            }
                        }

                        switch(low){
                            case 0x07: // fx07 - Vx = delay timer
                                hardware::registers.at(high & 0x0f) = hardware::delay_timer;
                                break;
                            
                            case 0x0a: // fx0a - wait for keypress; Vx = key
                                hardware::waiting_for_key = (high & 0x0f);
                                break;
                            
                            case 0x15: // fx15 - delay timer = Vx
                                hardware::delay_timer = hardware::registers.at(high & 0x0f);
                                break;
                            
                            case 0x18: // fx18 - sound timer = Vx
                                hardware::sound_timer = hardware::registers.at(high & 0x0f);
                                break;
                            
                            case 0x1e: // fx1e - I += Vx
                                hardware::register_I += hardware::registers.at(high & 0x0f);
                                break;
                            
                            case 0x29: // fx29 - I = address of sprite of hex digit in Vx
                                if(hardware::registers.at(high & 0x0f) < 0x10){
                                    hardware::register_I = hardware::registers.at(high & 0x0f) * 5;
                                }else if(quirks::quirk_fx29_digits_highres && hardware::registers.at(high & 0x0f) >= 0x10 && hardware::registers.at(high & 0x0f) <= 0x19){
                                    hardware::register_I = 80 + (hardware::registers.at(high & 0x0f) & 0x0f) * 10;
                                }else{
                                    throw std::runtime_error("invalid usage of opcode fx29");
                                }
                                break;
                            
                            case 0x33: // fx33 - memory[I, I+1, I+2] = BCD of Vx
                                bcd_of_v(high & 0x0f);
                                break;
                            
                            case 0x55: // fx55 - store V0 to Vx in memory starting at I; I = I + x + 1
                                for(uint8_t i = 0; i <= (high & 0x0f); i++){
                                    hardware::memory.at(hardware::register_I) = hardware::registers.at(i);
                                    hardware::register_I++;
                                }
                                
                                if constexpr(quirks::quirk_fx55_fx65_increment_less)
                                    hardware::register_I--;
                                else if constexpr(quirks::quirk_fx55_fx65_no_increment)
                                    hardware::register_I -= ((high & 0x0f) + 1);
                                break;
                            
                            case 0x65: // fx65 - load V0 to Vx from memory starting at I; I = I + x + 1
                                for(uint8_t i = 0; i <= (high & 0x0f); i++){
                                    hardware::registers.at(i) = hardware::memory.at(hardware::register_I);
                                    hardware::register_I++;
                                }

                                if constexpr(quirks::quirk_fx55_fx65_increment_less)
                                    hardware::register_I--;
                                else if constexpr(quirks::quirk_fx55_fx65_no_increment)
                                    hardware::register_I -= ((high & 0x0f) + 1);
                                break;

                            default:
                                throw std::runtime_error("unkown opcode");
                                break;
                        }
                        break;
                    
                    default:
                        throw std::runtime_error("unkown opcode");
                        break;
                }

                return return_value;
            }
    };

    typedef chip8_interpreter<chip8_instruction_set<false, false, false>, quirks_chip8, chip8_hardware<4096, 64, 32, false>> chip8;
    typedef chip8_interpreter<chip8_instruction_set<true, false, false>, quirks_chip8, chip8_hardware<4096, 64, 32, false>> chip8e;
    typedef chip8_interpreter<chip8_instruction_set<false, false, false>, quirks_chip48, chip8_hardware<4096, 64, 32, false>> chip48;
    typedef chip8_interpreter<chip8_instruction_set<false, true, false>, quirks_schip10, chip8_hardware<4096, 128, 64, true>> schip10;
    typedef chip8_interpreter<chip8_instruction_set<false, true, true>, quirks_schip11, chip8_hardware<4096, 128, 64, true>> schip11;
    typedef chip8_interpreter<chip8_instruction_set<false, true, true>, quirks_schpc, chip8_hardware<4096, 128, 64, true>> schpc;
}

template<class chip8_class, class frontend_class> void run(char* filename){
    using namespace std::chrono_literals;
    std::chrono::time_point<std::chrono::steady_clock> clock_start;

    chip8_class c8;

    if(c8.load_binary(filename)){
        throw std::runtime_error(std::string("couldn't open ") + filename);
    }

    frontend_class f(c8.get_screen_x(), c8.get_screen_y(), 10, 60);

    while(1){
        clock_start = std::chrono::steady_clock::now();
        
        // handle input
        c8.set_key(f.get_key());
        if(f.get_quit_requested()) break;
        
        // execute one opcode
        if(!c8.execute(f)) break;
        
        // update the screen
        f.refresh();

        // wait
        std::this_thread::sleep_until(clock_start + 1ms);
    }
}

int main(int argc, char* argv[]){
    if(argc < 3){
        std::cerr << "usage: " << argv[0] << " mode file\n";
        return 1;
    }

    try{

        if(std::strcmp(argv[1], "chip8") == 0){
            run<chip8::chip8, frontend_sdl>(argv[2]);
        }else if(std::strcmp(argv[1], "chip8e") == 0){
            run<chip8::chip8e, frontend_sdl>(argv[2]);
        }else if(std::strcmp(argv[1], "chip48") == 0){
            run<chip8::chip48, frontend_sdl>(argv[2]);
        }else if(std::strcmp(argv[1], "schip10") == 0){
            run<chip8::schip10, frontend_sdl>(argv[2]);
        }else if(std::strcmp(argv[1], "schip11") == 0){
            run<chip8::schip11, frontend_sdl>(argv[2]);
        }else if(std::strcmp(argv[1], "schpc") == 0){
            run<chip8::schpc, frontend_sdl>(argv[2]);
        }else{
            throw std::runtime_error(std::string("unknown mode ") + argv[1]);
        }

    }catch(std::runtime_error &e){
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}