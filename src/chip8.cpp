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
        if(!luaL_dofile(L, "config.lua") == LUA_OK){
            throw std::runtime_error(lua_tostring(L, -1));
        }
        if(!lua_istable(L, -1)){
            throw std::runtime_error(argv[1] + std::string(" did not return a table"));
        }

        if(std::strcmp(argv[1], "chip8") == 0){
            run<chip8::chip8, frontend_sdl>(argv[2], L);
        }else if(std::strcmp(argv[1], "chip10") == 0){
            run<chip8::chip10, frontend_sdl>(argv[2], L);
        }else if(std::strcmp(argv[1], "chip8e") == 0){
            run<chip8::chip8e, frontend_sdl>(argv[2], L);
        }else if(std::strcmp(argv[1], "chip48") == 0){
            run<chip8::chip48, frontend_sdl>(argv[2], L);
        }else if(std::strcmp(argv[1], "chip8_fxf2_fx55_fx65") == 0){
            run<chip8::chip8_fxf2_fx55_fx65, frontend_sdl>(argv[2], L);
        }else if(std::strcmp(argv[1], "chip8_fxf2_bnnn") == 0){
            run<chip8::chip8_fxf2_bnnn, frontend_sdl>(argv[2], L);
        }else if(std::strcmp(argv[1], "chip8_fxf2") == 0){
            run<chip8::chip8_fxf2, frontend_sdl>(argv[2], L);
        }else if(std::strcmp(argv[1], "schip10") == 0){
            run<chip8::schip10, frontend_sdl>(argv[2], L);
        }else if(std::strcmp(argv[1], "schip11") == 0){
            run<chip8::schip11, frontend_sdl>(argv[2], L);
        }else if(std::strcmp(argv[1], "schip11scu") == 0){
            run<chip8::schip11scu, frontend_sdl>(argv[2], L);
        }else if(std::strcmp(argv[1], "schpc") == 0){
            run<chip8::schpc, frontend_sdl>(argv[2], L);
        }else if(std::strcmp(argv[1], "schip11_fx1e") == 0){
            run<chip8::schip11_fx1e, frontend_sdl>(argv[2], L);
        }else if(std::strcmp(argv[1], "schpc_fx1e") == 0){
            run<chip8::schpc_fx1e, frontend_sdl>(argv[2], L);
        }else if(std::strcmp(argv[1], "chip8x") == 0){
            run<chip8::chip8x, frontend_sdl>(argv[2], L);
        }else if(std::strcmp(argv[1], "xochip") == 0){
            run<chip8::xochip, frontend_sdl>(argv[2], L);
        }else if(std::strcmp(argv[1], "octo") == 0){
            run<chip8::octo, frontend_sdl>(argv[2], L);
        }else if(std::strcmp(argv[1], "chip8run") == 0){
            run<chip8::chip8run, frontend_sdl>(argv[2], L);
        }else{
            throw std::runtime_error(std::string("unknown mode ") + argv[1]);
        }

    }catch(std::runtime_error &e){
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}