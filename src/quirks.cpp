#include <iostream>

namespace chip8{
    /** The available quirks.
    These change the emulation behaviour.
    */
    class chip8_quirks{
        public:
            /// bnnn (bxnn): jump to xnn + Vx instead of xnn + V0
            bool quirk_bnnn_bxnn_use_vx = false;
            /// fx55 and fx65: increment I by x instead of x+1
            bool quirk_fx55_fx65_increment_less = false;
            /// fx55 and fx65: don't increment I
            bool quirk_fx55_fx65_no_increment = false;
            /// 8xy6 and 8xye: shift Vx instead of Vy
            bool quirk_8xy6_8xye_shift_vx = false;
            /// dxyn (dxy0): draw a 16x16 sprite in high resolution mode
            bool quirk_dxy0_16x16_highres = false;
            /// dxyn (dxy0): draw a 16x16 sprite in low resolution mode
            bool quirk_dxy0_16x16_lowres = false;
            /// dxyn (dxy0): draw a 8x16 sprite in low resolution mode
            bool quirk_dxy0_8x16_lowres = false;
            /// fx29: point to 10 byte sprites for the digits 0-9 when 10 <= x <= 19
            bool quirk_fx29_digits_highres = false;
            /// dxyn: in high resolution mode: set Vf to the number of rows that have a collision or are clipped at the bottom
            bool quirk_dxyn_count_collisions_highres = false;
            /// dxyn: disable sprite wrapping
            bool quirk_dxyn_no_wrapping = false;
            /// fx55 and fx65: load/store V(RD.0) to Vx
            bool quirk_fx55_fx65_use_rd0 = false;
            /// bnnn: jump to nnn + V(RD.0)
            bool quirk_bnnn_use_rd0 = false;
            /// fx75 and fx85 work for all registers
            bool quirk_fx75_fx85_allow_all = false;
            /// 00fe and 00ff clear the screen
            bool quirk_00fe_00ff_clear_screen = false;
            /// fx1e sets vF to 1 if I overflows, otherwise to 0
            bool quirk_fx1e_set_vf = false;
            /// fx1e sets vF to 1 if I overflows, otherwise to 0
            bool quirk_fx1e_overflow_at_memory_size = false;
            /// if 00fe and 00ff clear the screen, inactive screen planes are cleared as well
            bool quirk_00fe_00ff_clear_all_planes = false;
            /// scroll the double of the given pixels in low resolution mode
            bool quirk_lowres_double_scroll = false;
            /// fx30: enable big hex characters
            bool quirk_fx30_allow_hex = false;

            bool override_fx55_fx65_no_increment = false;

            explicit chip8_quirks(lua_State *L){
                override_fx55_fx65_no_increment = false;

                lua_getfield(L, -1, "quirks");
                if(lua_istable(L, -1)){
                    lua_getfield(L, -1, "bnnn_bxnn_use_vx");
                    quirk_bnnn_bxnn_use_vx = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "fx55_fx65_increment_less");
                    quirk_fx55_fx65_increment_less = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "fx55_fx65_no_increment");
                    quirk_fx55_fx65_no_increment = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "8xy6_8xye_shift_vx");
                    quirk_8xy6_8xye_shift_vx = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "dxy0_16x16_highres");
                    quirk_dxy0_16x16_highres = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "dxy0_16x16_lowres");
                    quirk_dxy0_16x16_lowres = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "dxy0_8x16_lowres");
                    quirk_dxy0_8x16_lowres = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "fx29_digits_highres");
                    quirk_fx29_digits_highres = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "dxyn_count_collisions_highres");
                    quirk_dxyn_count_collisions_highres = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "dxyn_no_wrapping");
                    quirk_dxyn_no_wrapping = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "fx55_fx65_use_rd0");
                    quirk_fx55_fx65_use_rd0 = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "bnnn_use_rd0");
                    quirk_bnnn_use_rd0 = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "fx75_fx85_allow_all");
                    quirk_fx75_fx85_allow_all = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "00fe_00ff_clear_screen");
                    quirk_00fe_00ff_clear_screen = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "fx1e_set_vf");
                    quirk_fx1e_set_vf = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "fx1e_overflow_at_memory_size");
                    quirk_fx1e_overflow_at_memory_size = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "00fe_00ff_clear_all_plan");
                    quirk_00fe_00ff_clear_all_planes = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "lowres_double_scroll");
                    quirk_lowres_double_scroll = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "fx30_allow_hex");
                    quirk_fx30_allow_hex = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);
                }
                lua_pop(L, 1);
            }

            /// Print the quirks to outstream
            void print(std::ostream &outstream){
                outstream
                << "bnnn_bxnn_use_vx                " << (quirk_bnnn_bxnn_use_vx ? "true\n" : "false\n")
                << "fx55_fx65_increment_less        " << (quirk_fx55_fx65_increment_less ? "true\n" : "false\n")
                << "fx55_fx65_no_increment          " << (quirk_fx55_fx65_no_increment ? "true\n" : "false\n")
                << "8xy6_8xye_shift_vx              " << (quirk_8xy6_8xye_shift_vx ? "true\n" : "false\n")
                << "dxy0_16x16_highres              " << (quirk_dxy0_16x16_highres ? "true\n" : "false\n")
                << "dxy0_16x16_lowres               " << (quirk_dxy0_16x16_lowres ? "true\n" : "false\n")
                << "dxy0_8x16_lowres                " << (quirk_dxy0_8x16_lowres ? "true\n" : "false\n")
                << "fx29_digits_highres             " << (quirk_fx29_digits_highres ? "true\n" : "false\n")
                << "dxyn_count_collisions_highres   " << (quirk_dxyn_count_collisions_highres ? "true\n" : "false\n")
                << "dxyn_no_wrapping                " << (quirk_dxyn_no_wrapping ? "true\n" : "false\n")
                << "fx55_fx65_use_rd0               " << (quirk_fx55_fx65_use_rd0 ? "true\n" : "false\n")
                << "bnnn_use_rd0                    " << (quirk_bnnn_use_rd0 ? "true\n" : "false\n")
                << "fx75_fx85_allow_all             " << (quirk_fx75_fx85_allow_all ? "true\n" : "false\n")
                << "00fe_00ff_clear_screen          " << (quirk_00fe_00ff_clear_screen ? "true\n" : "false\n")
                << "fx1e_set_vf                     " << (quirk_fx1e_set_vf ? "true\n" : "false\n")
                << "fx1e_overflow_at_memory_size    " << (quirk_fx1e_overflow_at_memory_size ? "true\n" : "false\n")
                << "00fe_00ff_clear_all_planes      " << (quirk_00fe_00ff_clear_all_planes ? "true\n" : "false\n")
                << "lowres_double_scroll            " << (quirk_lowres_double_scroll ? "true\n" : "false\n")
                << "fx30_allow_hex                  " << (quirk_fx30_allow_hex ? "true\n" : "false\n");
            }
    };

    /*
    typedef chip8_quirks<false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false> quirks_chip8;
    typedef chip8_quirks<true,  true,  false, true,  false, false, false, false, false, true,  false, false, false, false, false, false, false, false> quirks_chip48;
    typedef chip8_quirks<false, false, false, false, false, false, false, false, false, false, true,  false, false, false, false, false, false, false> quirks_chip8_fxf2_fx55_fx65;
    typedef chip8_quirks<false, false, false, false, false, false, false, false, false, false, false, true,  false, false, false, false, false, false> quirks_chip8_fxf2_bnnn;
    typedef chip8_quirks<false, false, false, false, false, false, false, false, false, false, true,  true,  false, false, false, false, false, false> quirks_chip8_fxf2;
    typedef chip8_quirks<true,  true,  false, true,  true,  false, true,  true,  false, true,  false, false, false, false, false, false, false, false> quirks_schip10;
    typedef chip8_quirks<true,  false, true,  true,  true,  false, true,  false, true,  true,  false, false, false, false, false, false, false, false> quirks_schip11;
    typedef chip8_quirks<false, false, false, false, true,  false, false, false, false, false, false, false, false, false, false, false, false, false> quirks_schpc;
    typedef chip8_quirks<true,  false, true,  true,  true,  false, true,  false, true,  true,  false, false, false, false, true,  true,  false, false> quirks_schip11_fx1e;
    typedef chip8_quirks<false, false, false, false, true,  false, false, false, false, false, false, false, false, false, true,  true,  false, false> quirks_schpc_fx1e;
    typedef chip8_quirks<false, false, false, false, true,  true,  false, false, false, false, false, false, true,  true,  false, false, true,  false> quirks_xochip;
    typedef chip8_quirks<false, false, false, false, true,  true,  false, false, false, false, false, false, true,  true,  false, false, true,  true > quirks_octo;
    typedef chip8_quirks<false, false, false, true,  true,  false, false, false, false, true,  false, false, false, false, false, false, false, false> quirks_chip8run;
    */
}