#include <iomanip>
#include <string>
#include <exception>
#include <stdexcept>

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
                                if(hardware::sound_timer > 1) f.set_audio_state(true);
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
    typedef chip8_interpreter<chip8_instruction_set<false, false, false>, quirks_chip8, chip8_hardware<4096, 128, 64, false>> chip10;
    typedef chip8_interpreter<chip8_instruction_set<true, false, false>, quirks_chip8, chip8_hardware<4096, 64, 32, false>> chip8e;
    typedef chip8_interpreter<chip8_instruction_set<false, false, false>, quirks_chip48, chip8_hardware<4096, 64, 32, false>> chip48;
    typedef chip8_interpreter<chip8_instruction_set<false, true, false>, quirks_schip10, chip8_hardware<4096, 128, 64, true>> schip10;
    typedef chip8_interpreter<chip8_instruction_set<false, true, true>, quirks_schip11, chip8_hardware<4096, 128, 64, true>> schip11;
    typedef chip8_interpreter<chip8_instruction_set<false, true, true>, quirks_schpc, chip8_hardware<4096, 128, 64, true>> schpc;
}