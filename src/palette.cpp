#include <array>

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace chip8{
    class chip8_palette{
        private:
            std::array<uint8_t, 3> fg = {{0xff, 0xff, 0xff}};
            std::array<uint8_t, 3> bg = {{0x00, 0x00, 0x00}};

            /**
             * @brief load a color from the table at the top of the stack
             * 
             * @param L lua state
             * @param color the target color
             * @param index index of the color in the table
             */
            void load_color(lua_State *L, std::array<uint8_t, 3> &color, int index){
                lua_geti(L, -1, index);
                if(lua_istable(L, -1)){
                    for(int i = 1; i < 4; i++){
                        lua_geti(L, -1, i);
                        if(lua_isinteger(L, -1)) color.at(i - 1) = lua_tointeger(L, -1);
                        lua_pop(L, 1);
                    }
                }
                lua_pop(L, 1);
            }

        public:
            /// returns the color of the pixel at (x, y)
            template<class hardware> std::array<uint8_t, 3> color(hardware hw, int x, int y){
                if(hw->screen_content.at(0).at(y).at(x)){
                    return fg;
                }
                return bg;
            }

            /// returns the background color for the whole screen
            template<class hardware> std::array<uint8_t, 3> bg_color(hardware hw){
                (void)hw;
                return bg;
            }

            void load_config(lua_State *L){
                lua_getfield(L, -1, "palette");
                if(lua_istable(L, -1)){
                    load_color(L, fg, 1);
                    load_color(L, bg, 2);
                }
                lua_pop(L, 1);
            }
    };

    class chip8x_palette{
        private:
            std::array<uint8_t, 3> black = {{0x00, 0x00, 0x00}};
            std::array<uint8_t, 3> red = {{0xff, 0x00, 0x00}};
            std::array<uint8_t, 3> blue = {{0x00, 0x00, 0xff}};
            std::array<uint8_t, 3> violet = {{0xff, 0x00, 0xff}};
            std::array<uint8_t, 3> green = {{0x00, 0xff, 0x00}};
            std::array<uint8_t, 3> yellow = {{0xff, 0xff, 0x00}};
            std::array<uint8_t, 3> aqua = {{0x00, 0xff, 0xff}};
            std::array<uint8_t, 3> white = {{0xff, 0xff, 0xff}};

        public:
            /// returns the color of the pixel at (x, y)
            template<class hardware> std::array<uint8_t, 3> color(hardware hw, int x, int y){
                if(hw->screen_content.at(0).at(y).at(x)){
                    switch(hw->screen_fg_color.at(y).at(x)){
                        case 0: return black; break;
                        case 1: return red; break;
                        case 2: return blue; break;
                        case 3: return violet; break;
                        case 4: return green; break;
                        case 5: return yellow; break;
                        case 6: return aqua; break;
                        default: return white;
                    }
                }

                return bg_color(hw);
            }

            /// returns the background color for the whole screen
            template<class hardware> std::array<uint8_t, 3> bg_color(hardware hw){
                switch(hw->screen_bg_color){
                    case 1: return black; break;
                    case 2: return green; break;
                    case 3: return red; break;
                    default: return blue;
                }
            }

            void load_config(lua_State *L){
                
            }
    };

    class xochip_palette{
        private:
            std::array<uint8_t, 3> c0 = {{0x00, 0x00, 0x00}}; // plane 1 : 0, plane 0 : 0
            std::array<uint8_t, 3> c1 = {{0x00, 0xff, 0x00}}; // plane 1 : 0, plane 0 : 1
            std::array<uint8_t, 3> c2 = {{0xff, 0x00, 0x00}}; // plane 1 : 1, plane 0 : 0
            std::array<uint8_t, 3> c3 = {{0xff, 0xff, 0x00}}; // plane 1 : 1, plane 0 : 1

        public:
            /// returns the color of the pixel at (x, y)
            template<class hardware> std::array<uint8_t, 3> color(hardware hw, int x, int y){
                if(hw->screen_content.at(0).at(y).at(x) && !hw->screen_content.at(1).at(y).at(x)){
                    return c1;
                }else if(!hw->screen_content.at(0).at(y).at(x) && hw->screen_content.at(1).at(y).at(x)){
                    return c2;
                }else if(hw->screen_content.at(0).at(y).at(x) && hw->screen_content.at(1).at(y).at(x)){
                    return c3;
                }

                return c0;
            }

            /// returns the background color for the whole screen
            template<class hardware> std::array<uint8_t, 3> bg_color(hardware hw){
                (void)hw;
                return c0;
            }

            void load_config(lua_State *L){
                
            }
    };
}
