#include <array>
#include <vector>
#include <chrono>
#include <fstream>
#include <stack>
#include <cstdlib>
#include <ctime>

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace chip8{
    template<class palette_t> class chip8_hardware {

        friend palette_t;

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
            size_t memory_size;
            std::vector<uint8_t> memory;

            // start of the program
            uint16_t program_start;
            
            // registers V0-Vf
            std::array<uint8_t, 16> registers;
            
            // register I (16 bit for a memory address)
            uint16_t register_I = 0x0000;

            // flag registers (used only for e.g. SUPER-CHIP)
            std::array<uint8_t, 16> flag_registers;

            // RD.0 register
            uint8_t register_rd0;
            
            // timers
            static constexpr int timer_delay = 1000000 / 60; // microseconds
            std::chrono::time_point<std::chrono::steady_clock> timer_start;
            uint8_t delay_timer = 0x00, sound_timer = 0x00;
            bool waiting_for_timer = false;
            
            // screen content
            unsigned int screen_x;
            unsigned int screen_y;
            unsigned int screen_planes;
            std::vector<std::vector<std::vector<uint8_t>>> screen_content;
            bool allow_high_res;
            bool high_res = false; // high resolution mode for SUPER-CHIP

            // active screen planes
            std::vector<bool> active_screen_planes;

            // foreground and background color for CHIP-8X
            std::vector<std::vector<uint8_t>> screen_fg_color;
            uint8_t screen_bg_color;

            // color palette
            palette_t palette;
            
            // program counter
            uint16_t pc;
            
            // call stack (for returning from subroutines)
            std::stack< uint16_t > call_stack;

            // currently pressed key
            std::array<bool, 16> keyboard_1, keyboard_2;
            int waiting_for_key = -1;

            uint8_t screen_get(size_t plane, size_t x, size_t y){
                return screen_content.at(plane).at(y).at(x);
            }

            template<class frontend> void screen_set(size_t plane, size_t x, size_t y, uint8_t value, frontend &f){
                screen_content.at(plane).at(y).at(x) = value;

                f.draw(x, y, palette.color(this, x, y));
            }
        
        public:
            void load_config(lua_State *L){
                palette.load_config(L);

                lua_getfield(L, -1, "x");
                screen_x = lua_isinteger(L, -1) ? lua_tointeger(L, -1) : 64;
                lua_pop(L, 1);

                lua_getfield(L, -1, "y");
                screen_y = lua_isinteger(L, -1) ? lua_tointeger(L, -1) : 32;
                lua_pop(L, 1);

                lua_getfield(L, -1, "planes");
                screen_planes = lua_isinteger(L, -1) ? lua_tointeger(L, -1) : 1;
                lua_pop(L, 1);

                lua_getfield(L, -1, "program_start");
                program_start = lua_isinteger(L, -1) ? lua_tointeger(L, -1) : 0x0200;
                lua_pop(L, 1);

                lua_getfield(L, -1, "memory_size");
                memory_size = lua_isinteger(L, -1) ? lua_tointeger(L, -1) : 4096;
                lua_pop(L, 1);

                lua_getfield(L, -1, "allow_high_res");
                allow_high_res = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                lua_pop(L, 1);
            }
            
            explicit chip8_hardware(lua_State *L){
                load_config(L);

                // resize and initialize memory
                memory.resize(memory_size);
                for(size_t i = 0; i < memory.size(); i++){
                    memory.at(i) = 0x00;
                }

                // initialize registers
                registers.fill(0x00);
                flag_registers.fill(0x00);
                register_rd0 = 0x00;
                pc = program_start;

                // resize and initialze screen content
                screen_content.resize(screen_planes);
                for(unsigned int plane = 0; plane < screen_content.size(); plane++){
                    screen_content.at(plane).resize(screen_y);
                    for(unsigned int y = 0; y < screen_content.at(plane).size(); y++){
                        screen_content.at(plane).at(y).resize(screen_x);
                        for(unsigned int x = 0; x < screen_content.at(plane).at(y).size(); x++){
                            screen_content.at(plane).at(y).at(x) = 0x00;
                        }
                    }
                }

                // font
                for(size_t i = 0; i < font.size(); i++){
                    memory.at(i) = font.at(i);
                }

                // big font
                for(size_t i = 0; i < big_font.size(); i++){
                    memory.at(80 + i) = big_font.at(i);
                }

                // screen colors
                screen_fg_color.resize(screen_y);
                for(unsigned int y = 0; y < screen_fg_color.size(); y++){
                    screen_fg_color.at(y).resize(screen_x);
                    for(unsigned int x = 0; x < screen_fg_color.at(y).size(); x++){
                        screen_fg_color.at(y).at(x) = 0x00;
                    }
                }
                screen_bg_color = 0x00;

                active_screen_planes.resize(screen_planes);
                for(unsigned int plane = 0; plane < screen_content.size(); plane++){
                    active_screen_planes.at(plane) = false;
                }
                active_screen_planes.at(0) = true;

                keyboard_1.fill(false);
                keyboard_2.fill(false);

                std::srand(std::time(nullptr));

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

            void set_key(int keyboard, int key, bool pressed){
                if(keyboard == 1){
                    keyboard_1.at(key) = pressed;
                }else if(keyboard == 2){
                    keyboard_2.at(key) = pressed;
                }
            }

            int get_screen_x(){
                return screen_x;
            }

            int get_screen_y(){
                return screen_y;
            }

            /* debug functions
            void print_registers(std::ostream &outstream){
                outstream << "I=" << std::setw(4) << std::setfill('0') << std::hex << register_I << " ";
                for(int i = 0; i < 16; i++)
                    outstream << "V" << std::hex << i << "=" << std::setw(2) << std::setfill('0') << std::hex << (int)registers.at(i) << " ";
                outstream << "delay=" << std::setw(2) << std::setfill('0') << std::hex << (int)delay_timer << " ";
                outstream << "sound=" << std::setw(2) << std::setfill('0') << std::hex << (int)sound_timer << " ";
                outstream << "pc=" << std::setw(4) << std::setfill('0') << std::hex << pc << "\n";
            }
            */

            /// Print the hardware configuration to outstream
            void print(std::ostream &outstream){
                outstream
                << "memory                          " << memory_size << " bytes\n"
                << "program start                   0x" << std::hex << std::setw(4) << std::setfill('0') << program_start << std::dec << std::setw(0) << std::setfill(' ') << "\n"
                << "screen resolution               " << screen_x << "x" << screen_y << "x" << screen_planes << "\n"
                << "high/low resolution modes       " << (allow_high_res ? "true\n" : "false\n");
            }
    };
}