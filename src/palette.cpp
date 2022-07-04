#include <array>
#include <string>

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace chip8{
    class chip8_palette{
        private:
            std::string type = "";
            std::vector<std::array<uint8_t, 3>> colors;

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
            void load_config(lua_State *L){
                lua_getfield(L, -1, "palette");

                // get palette type
                if(lua_istable(L, -1)){
                    lua_getfield(L, -1, "type");
                    type = lua_isstring(L, -1) ? lua_tostring(L, -1) : "";
                    lua_pop(L, 1);
                }

                // set default colors
                if(type == "chip8x"){
                    colors = {
                        {0x00, 0x00, 0x00}, // black
                        {0xff, 0x00, 0x00}, // red
                        {0x00, 0x00, 0xff}, // blue
                        {0xff, 0x00, 0xff}, // violet
                        {0x00, 0xff, 0x00}, // green
                        {0xff, 0xff, 0x00}, // yellow
                        {0x00, 0xff, 0xff}, // aqua
                        {0xff, 0xff, 0xff}, // white
                    };
                }else if(type == "xochip"){
                    colors = {
                        {0x00, 0x00, 0x00},
                        {0x00, 0xff, 0x00},
                        {0xff, 0x00, 0x00},
                        {0xff, 0xff, 0x00},
                    };
                }else{
                    colors = {
                        {0x00, 0x00, 0x00},
                        {0xff, 0xff, 0xff},
                    };
                }

                // load colors from config
                if(lua_istable(L, -1)){
                    for(size_t i = 0; i < colors.size(); i++){
                        load_color(L, colors.at(i), i+1);
                    }
                }

                lua_pop(L, 1);
            }

            template<class hardware> std::array<uint8_t, 3> color_chip8(hardware hw, int x, int y){
                if(hw->screen_content.at(0).at(y).at(x)){
                    return colors.at(1);
                }
                return colors.at(0);
            }

            template<class hardware> std::array<uint8_t, 3> color_chip8x(hardware hw, int x, int y){
                if(hw->screen_content.at(0).at(y).at(x)){
                    return colors.at(hw->screen_fg_color.at(y).at(x));
                }
                return bg_color_chip8x(hw);
            }

            template<class hardware> std::array<uint8_t, 3> color_xochip(hardware hw, int x, int y){
                if(hw->screen_content.at(0).at(y).at(x) && !hw->screen_content.at(1).at(y).at(x)){
                    return colors.at(1);
                }else if(!hw->screen_content.at(0).at(y).at(x) && hw->screen_content.at(1).at(y).at(x)){
                    return colors.at(2);
                }else if(hw->screen_content.at(0).at(y).at(x) && hw->screen_content.at(1).at(y).at(x)){
                    return colors.at(3);
                }
                return colors.at(0);
            }

            template<class hardware> std::array<uint8_t, 3> bg_color_chip8(hardware hw){
                (void)hw;
                return colors.at(0);
            }

            template<class hardware> std::array<uint8_t, 3> bg_color_chip8x(hardware hw){
                switch(hw->screen_bg_color){
                    case 1: return colors.at(0); break;
                    case 2: return colors.at(4); break;
                    case 3: return colors.at(1); break;
                    default: return colors.at(2);
                }
            }

            template<class hardware> std::array<uint8_t, 3> bg_color_xochip(hardware hw){
                (void)hw;
                return colors.at(0);
            }

            /// returns the color of the pixel at (x, y)
            template<class hardware> std::array<uint8_t, 3> color(hardware hw, int x, int y){
                if(type == "chip8x"){
                    return color_chip8x(hw, x, y);
                }else if(type == "xochip"){
                    return color_xochip(hw, x, y);
                }
                return color_chip8(hw, x, y);
            }

            /// returns the background color for the whole screen
            template<class hardware> std::array<uint8_t, 3> bg_color(hardware hw){
                if(type == "chip8x"){
                    return bg_color_chip8x(hw);
                }else if(type == "xochip"){
                    return bg_color_xochip(hw);
                }
                return bg_color_chip8(hw);
            }
    };
}
