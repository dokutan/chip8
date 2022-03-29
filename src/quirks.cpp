#include <iostream>

namespace chip8{
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
        bool t_dxyn_no_wrapping,
        bool t_fx55_fx65_use_rd0,
        bool t_bnnn_use_rd0,
        bool t_fx75_fx85_allow_all,
        bool t_00fe_00ff_clear_screen
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
            /// fx55 and fx65: load/store V(RD.0) to Vx
            static constexpr bool quirk_fx55_fx65_use_rd0 = t_fx55_fx65_use_rd0;
            /// bnnn: jump to nnn + V(RD.0)
            static constexpr bool quirk_bnnn_use_rd0 = t_bnnn_use_rd0;
            /// fx75 and fx85 work for all registers
            static constexpr bool quirk_fx75_fx85_allow_all = t_fx75_fx85_allow_all;
            /// 00fe and 00ff clear the screen
            static constexpr bool quirk_00fe_00ff_clear_screen = t_00fe_00ff_clear_screen;

            /// Print the quirks to outstream
            void print(std::ostream &outstream){
                outstream << "quirks:\n"
                << "bnnn_bxnn_use_vx                " << (quirk_bnnn_bxnn_use_vx ? "true\n" : "false\n")
                << "fx55_fx65_increment_less        " << (quirk_fx55_fx65_increment_less ? "true\n" : "false\n")
                << "fx55_fx65_no_increment          " << (quirk_fx55_fx65_no_increment ? "true\n" : "false\n")
                << "8xy6_8xye_shift_vx              " << (quirk_8xy6_8xye_shift_vx ? "true\n" : "false\n")
                << "dxy0_16x16_highres              " << (quirk_dxy0_16x16_highres ? "true\n" : "false\n")
                << "dxy0_8x16_lowres                " << (quirk_dxy0_8x16_lowres ? "true\n" : "false\n")
                << "fx29_digits_highres             " << (quirk_fx29_digits_highres ? "true\n" : "false\n")
                << "dxyn_count_collisions_highres   " << (quirk_dxyn_count_collisions_highres ? "true\n" : "false\n")
                << "dxyn_no_wrapping                " << (quirk_dxyn_no_wrapping ? "true\n" : "false\n")
                << "fx55_fx65_use_rd0               " << (quirk_fx55_fx65_use_rd0 ? "true\n" : "false\n")
                << "bnnn_use_rd0                    " << (quirk_bnnn_use_rd0 ? "true\n" : "false\n")
                << "fx75_fx85_allow_all             " << (quirk_fx75_fx85_allow_all ? "true\n" : "false\n")
                << "00fe_00ff_clear_screen          " << (quirk_00fe_00ff_clear_screen ? "true\n" : "false\n");
            }
    };

    typedef chip8_quirks<false, false, false, false, false, false, false, false, false, false, false, false, false> quirks_chip8;
    typedef chip8_quirks<true,  true,  false, true,  false, false, false, false, true,  false, false, false, false> quirks_chip48;
    typedef chip8_quirks<false, false, false, false, false, false, false, false, false, true,  false, false, false> quirks_chip8_fxf2_fx55_fx65;
    typedef chip8_quirks<false, false, false, false, false, false, false, false, false, false, true,  false, false> quirks_chip8_fxf2_bnnn;
    typedef chip8_quirks<false, false, false, false, false, false, false, false, false, true,  true,  false, false> quirks_chip8_fxf2;
    typedef chip8_quirks<true,  true,  false, true,  true,  true,  true,  false, true,  false, false, false, false> quirks_schip10;
    typedef chip8_quirks<true,  false, true,  true,  true,  true,  false, true,  true,  false, false, false, false> quirks_schip11;
    typedef chip8_quirks<false, false, false, false, true,  false, false, false, false, false, false, false, false> quirks_schpc;
    typedef chip8_quirks<false, false, false, false, false, false, false, false, false, false, false, true , true > quirks_xochip;
}