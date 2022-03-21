#include <array>

namespace chip8{
    class chip8_palette{
        private:
            std::array<uint8_t, 3> fg = {{0xff, 0xff, 0xff}};
            std::array<uint8_t, 3> bg = {{0x00, 0x00, 0x00}};

        public:
            /// returns the color of the pixel at (x, y)
            template<class hardware> std::array<uint8_t, 3> color(hardware hw, int x, int y){
                if(hw->screen_content.at(y).at(x)){
                    return fg;
                }
                return bg;
            }

            /// returns the background color for the whole screen
            template<class hardware> std::array<uint8_t, 3> bg_color(hardware hw){
                return bg;
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
                if(hw->screen_content.at(y).at(x)){
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
    };
}