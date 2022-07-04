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

#include "interpreter.cpp"
#include "frontend_sdl.cpp"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

template<class chip8_class, class frontend_class> void run(char* filename, lua_State* L){

    lua_getfield(L, -1, "frametime");
    int frametime = lua_isinteger(L, -1) ? lua_tointeger(L, -1) : 1000;
    lua_pop(L, 1);

    using namespace std::chrono_literals;
    std::chrono::time_point<std::chrono::steady_clock> clock_start;

    chip8_class c8(L);

    if(c8.load_binary(filename)){
        throw std::runtime_error(std::string("couldn't open ") + filename);
    }

    frontend_class f(c8.get_screen_x(), c8.get_screen_y(), 10, 60);
    c8.frontend_init(f);
    c8.print(std::cout);

    while(1){
        clock_start = std::chrono::steady_clock::now();
        
        // handle input
        f.poll_event();
        if(f.get_quit_requested()) break;
        f.get_keys(c8);
        
        // execute one opcode
        if(!c8.execute(f)) break;
        
        // update the screen
        f.refresh();

        // wait
        std::this_thread::sleep_until(clock_start + frametime * 1us);
    }
}

int main(int argc, char* argv[]){
    if(argc < 3){
        std::cerr << "usage: " << argv[0] << " mode file\n";
        return 1;
    }

    try{
        lua_State *L = luaL_newstate();
        luaL_openlibs(L);

        if(!luaL_dofile(L, argv[1]) == LUA_OK){
            throw std::runtime_error(lua_tostring(L, -1));
        }
        if(!lua_istable(L, -1)){
            throw std::runtime_error(argv[1] + std::string(" did not return a table"));
        }

        run<chip8::chip8_interpreter<chip8::chip8_instruction_set, chip8::chip8_quirks, chip8::chip8_hardware<chip8::chip8_palette>>, frontend_sdl>(argv[2], L);

    }catch(std::runtime_error &e){
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}