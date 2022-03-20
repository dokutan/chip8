#include <iomanip>
#include <string>
#include <exception>
#include <stdexcept>
#include <cstdlib>

namespace chip8{
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

            // draw a sprite
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
                    if(quirks::quirk_dxyn_no_wrapping && hardware::registers.at(y) >= (hardware::screen_y / 2)) return;
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
                            if(quirks::quirk_dxyn_count_collisions_highres && hardware::high_res)
                                hardware::registers.at(0xf) += (N - 1 - n);
                            break;
                        }

                        Y = (Y + 1) % hardware::screen_y;
                    }
                }
            }
        
        public:
            chip8_interpreter(){
                quirks::print(std::cout);
            }

            /// execute one instruction at pc and increment pc
            template<class frontend> int execute(frontend &f){
                int return_value = 1;
                bool matched_opcode = false;

                // decrement timers
                std::chrono::time_point<std::chrono::steady_clock> timer_now = std::chrono::steady_clock::now();
                if(std::chrono::duration_cast<std::chrono::microseconds>(timer_now - hardware::timer_start).count() >= hardware::timer_delay){
                    hardware::timer_start = timer_now;
                    if(hardware::sound_timer == 1) f.set_audio_state(false);
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
                        hardware::pc += 2;   
                    
                    // 5xy1 - skip if Vx > Vy (CHIP-8E)
                    }else if(high_h == 0x05 && low_l == 0x01){
                        if(hardware::registers.at(high_l) > hardware::registers.at(low_h)) hardware::pc += 2;
                    
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
                        throw std::runtime_error("opcode fx03 not implemented");
                    
                    // fx1b - skip Vx bytes (CHIP-8E)
                    }else if(high_h == 0x0f && low == 0x1b){
                        hardware::pc += hardware::registers.at(high_l);
                    
                    // fx4f - delay timer = Vx; wait until the delay timer reaches 0 (CHIP-8E)
                    }else if(high_h == 0x0f && low == 0x4f){
                        hardware::delay_timer = hardware::registers.at(high_l);
                        hardware::waiting_for_timer = true;
                    
                    // fxe3 - wait for strobe at EF4; read Vx from input port 3 (CHIP-8E)
                    }else if(high_h == 0x0f && low == 0xe3){
                        throw std::runtime_error("opcode fxe3 not implemented");
                    
                    // fxe7 - read Vx from input port 3 (CHIP-8E)
                    }else if(high_h == 0x0f && low == 0xe7){
                        throw std::runtime_error("opcode fxe7 not implemented");

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
                    
                    // 00ff - enable high resolution mode (SUPER-CHIP 1.0)
                    }else if(opcode == 0x00ff){
                        if(hardware::allow_high_res) hardware::high_res = true;

                    // fx75 - store V0 - Vx in RPL user flags (0 <= x <= 7) (SUPER-CHIP 1.0)
                    }else if(high_h == 0x0f && low == 0x75){
                        if(high_l < 8){
                            for(uint8_t i = 0; i <= high_l; i++){
                                hardware::flag_registers.at(i) = hardware::registers.at(i);
                            }
                        }else{
                            throw std::runtime_error("invalid usage of opcode fx75");
                        }
                    
                    // fx85 - load V0 - Vx from RPL user flags (0 <= x <= 7) (SUPER-CHIP 1.0)
                    }else if(high_h == 0x0f && low == 0x85){
                        if(high_l < 8){
                            for(uint8_t i = 0; i <= high_l; i++){
                                hardware::registers.at(i) = hardware::flag_registers.at(i);
                            }
                        }else{
                            throw std::runtime_error("invalid usage of opcode fx85");
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
                        for(int y = hardware::screen_content.size() - 1 ; y >= low_l;  y--){
                            hardware::screen_content.at(y) = hardware::screen_content.at(y - low_l);
                            
                            for(int x = 0; x < hardware::screen_content.at(y).size(); x++){
                                if(hardware::screen_content.at(y).at(x)){
                                    f.draw(x, y, 0xff, 0xff, 0xff);
                                }else{
                                    f.draw(x, y, 0x00, 0x00, 0x00);
                                }
                            }
                        }
                        for(int y = 0; y < low_l; y++){
                            hardware::screen_content.at(y).fill(0x00);
                            for(int x = 0; x < hardware::screen_content.at(y).size(); x++){
                                f.draw(x, y, 0x00, 0x00, 0x00);
                            }
                        }
                    
                    // 00fb - scroll display 4 pixels right (SUPER-CHIP 1.1)
                    }else if(opcode == 0x00fb){
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
                    
                    // 00fc - scroll display 4 pixels left (SUPER-CHIP 1.1)
                    }else if(opcode == 0x00fc){
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

                // 00e0 - clear screen
                if(opcode == 0x00e0){
                    for(auto &i : hardware::screen_content){
                        i.fill(0x00);
                    }
                    f.clear(0x00, 0x00, 0x00);
                
                // 00ee - return
                }else if(opcode == 0x00ee){
                    if(hardware::call_stack.size() == 0) throw std::runtime_error("call stack empty - can not return");
                    hardware::pc = hardware::call_stack.top();
                    hardware::call_stack.pop();
                
                // 0nnn - call machine language subroutine at nnn
                }else if(high_h == 0x00){
                    throw std::runtime_error("opcode 0nnn not implemented");
                
                // 1nnn - jump to nnn
                }else if(high_h == 0x01){
                    hardware::pc = ((high_l << 8) | low);
                
                // 2nnn - call subroutine at nnn
                }else if(high_h == 0x02){
                    hardware::call_stack.push(hardware::pc);
                    hardware::pc = ((high_l << 8) | low);
                
                // 3xnn - skip if Vx == nn
                }else if(high_h == 0x03){
                    if(hardware::registers.at(high_l) == low) hardware::pc += 2;
                
                // 4xnn - skip if Vx != nn
                }else if(high_h == 0x04){
                    if(hardware::registers.at(high_l) != low) hardware::pc += 2;
                
                // 5xy0 - skip if Vx == Vy
                }else if(high_h == 0x05 && low_l == 0x00){
                    if(hardware::registers.at(high_l) == hardware::registers.at(low_h)) hardware::pc += 2;
                
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
                    hardware::registers.at(0xf) = result <= hardware::registers.at(high & 0xf) && hardware::registers.at(low_h) > 0 ? 0x01 : 0x00;
                    hardware::registers.at(high_l) = result;

                // 8xy5 - Vx -= Vy; Vf = borrow ? 0 : 1
                }else if(high_h == 0x08 && low_l == 0x05){
                    uint8_t result;
                    result = hardware::registers.at(high_l) - hardware::registers.at(low_h);
                    hardware::registers.at(0xf) = result >= hardware::registers.at(high & 0xf) && hardware::registers.at(low_h) > 0 ? 0x00 : 0x01;
                    hardware::registers.at(high_l) = result;

                // 8xy6 - Vx = Vy >> 1; Vf = Vy & 0x01 
                }else if(high_h == 0x08 && low_l == 0x06){
                    if constexpr(quirks::quirk_8xy6_8xye_shift_vx){
                        hardware::registers.at(0xf) = hardware::registers.at(high_l) & 0x01;
                        hardware::registers.at(high_l) = hardware::registers.at(high_l) >> 1;
                    }else{
                        hardware::registers.at(0xf) = hardware::registers.at(low_h) & 0x01;
                        hardware::registers.at(high_l) = hardware::registers.at(low_h) >> 1;
                    }

                // 8xy7 - Vx = Vy - Vx; Vf = borrow ? 0 : 1
                }else if(high_h == 0x08 && low_l == 0x07){
                    uint8_t result;
                    result = hardware::registers.at(low_h) - hardware::registers.at(high_l);
                    hardware::registers.at(0xf) = result >= hardware::registers.at(low_h) && hardware::registers.at(high_l) > 0 ? 0x00 : 0x01;
                    hardware::registers.at(high_l) = result;

                // 8xye - Vx = Vy << 1; Vf = Vy & 0x80
                }else if(high_h == 0x08 && low_l == 0x0e){
                    if constexpr(quirks::quirk_8xy6_8xye_shift_vx){
                        hardware::registers.at(0xf) = hardware::registers.at(high_l) & 0x80;
                        hardware::registers.at(high_l) = hardware::registers.at(high_l) << 1;
                    }else{
                        hardware::registers.at(0xf) = hardware::registers.at(low_h) & 0x80;
                        hardware::registers.at(high_l) = hardware::registers.at(low_h) << 1;
                    }
                
                // 9xy0 - skip if Vx != Vy
                }else if(high_h == 0x09 && low_l == 0x00){
                    if(hardware::registers.at(high_l) != hardware::registers.at(low_h)) hardware::pc += 2;

                // annn - I = nnn
                }else if(high_h == 0x0a){
                    hardware::register_I = ((high_l << 8) | low);

                // bnnn - jump to nnn + V0
                }else if(high_h == 0x0b){
                    if constexpr(quirks::quirk_bnnn_bxnn_use_vx){
                        hardware::pc = ((high_l << 8) | low) + hardware::registers.at(high_l);
                    }else{
                        hardware::pc = ((high_l << 8) | low) + hardware::registers.at(0);
                    }

                // cxnn - Vx = random & nn
                }else if(high_h == 0x0c){
                    hardware::registers.at(high& 0x0f) = std::rand() & low;
                
                // dxyn - draw n bytes at (Vx, Vy)
                }else if(high_h == 0x0d){
                    draw(f, high_l, low_h, low_l);

                // ex9e - skip if pressed key == Vx
                }else if(high_h == 0x0e && low == 0x9e){
                    if(hardware::pressed_key == hardware::registers.at(high_l)) hardware::pc += 2;

                // exa1 - skip if pressed key != Vx
                }else if(high_h == 0x0e && low == 0xa1){
                    if(hardware::pressed_key != hardware::registers.at(high_l)) hardware::pc += 2;

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
                    hardware::register_I += hardware::registers.at(high_l);
                
                // fx29 - I = address of sprite of hex digit in Vx
                }else if(high_h == 0x0f && low == 0x29){
                    if(hardware::registers.at(high_l) < 0x10){
                        hardware::register_I = hardware::registers.at(high_l) * 5;
                    }else if(quirks::quirk_fx29_digits_highres && hardware::registers.at(high_l) >= 0x10 && hardware::registers.at(high_l) <= 0x19){
                        hardware::register_I = 80 + (hardware::registers.at(high_l) & 0x0f) * 10;
                    }else{
                        throw std::runtime_error("invalid usage of opcode fx29");
                    }
                
                // fx33 - memory[I, I+1, I+2] = BCD of Vx
                }else if(high_h == 0x0f && low == 0x33){
                    bcd_of_v(high_l);

                // fx55 - store V0 to Vx in memory starting at I; I = I + x + 1
                }else if(high_h == 0x0f && low == 0x55){
                    for(uint8_t i = 0; i <= (high_l); i++){
                        hardware::memory.at(hardware::register_I) = hardware::registers.at(i);
                        hardware::register_I++;
                    }
                    
                    if constexpr(quirks::quirk_fx55_fx65_increment_less)
                        hardware::register_I--;
                    else if constexpr(quirks::quirk_fx55_fx65_no_increment)
                        hardware::register_I -= (high_l + 1);

                // fx65 - load V0 to Vx from memory starting at I; I = I + x + 1
                }else if(high_h == 0x0f && low == 0x65){
                    for(uint8_t i = 0; i <= high_l; i++){
                        hardware::registers.at(i) = hardware::memory.at(hardware::register_I);
                        hardware::register_I++;
                    }

                    if constexpr(quirks::quirk_fx55_fx65_increment_less)
                        hardware::register_I--;
                    else if constexpr(quirks::quirk_fx55_fx65_no_increment)
                        hardware::register_I -= (high_l + 1);

                }else{
                    throw std::runtime_error("unkown opcode");
                }

                return return_value;
            }
    };

    typedef chip8_interpreter<chip8_instruction_set<false, false, false>, quirks_chip8, chip8_hardware<4096, 64, 32, false>> chip8;
    typedef chip8_interpreter<chip8_instruction_set<false, false, false>, quirks_chip8, chip8_hardware<4096, 128, 64, false>> chip10;
    typedef chip8_interpreter<chip8_instruction_set<true, false, false>, quirks_chip8, chip8_hardware<4096, 64, 32, false>> chip8e;
    typedef chip8_interpreter<chip8_instruction_set<false, false, false>, quirks_chip48, chip8_hardware<4096, 64, 32, false>> chip48;
    typedef chip8_interpreter<chip8_instruction_set<false, true, false>, quirks_schip10, chip8_hardware<4096, 128, 64, true>> schip10;
    typedef chip8_interpreter<chip8_instruction_set<false, true, true>, quirks_schip11, chip8_hardware<4096, 128, 64, true>> schip11;
    typedef chip8_interpreter<chip8_instruction_set<false, true, true>, quirks_schpc, chip8_hardware<4096, 128, 64, true>> schpc;
}