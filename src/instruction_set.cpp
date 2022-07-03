namespace chip8{
    /** The available instruction set extensions.
    The instruction set only determines which opcodes are available, not their behaviour.
    */
    class chip8_instruction_set{
        protected:
            /// the CHIP-8E extension
            bool chip8e;
            /// the SUPER-CHIP 1.0 extension
            bool super_chip_1_0;
            /// the SUPER-CHIP 1.1 extension (without SUPER-CHIP 1.0 opcodes)
            bool super_chip_1_1;
            //// 00bn instruction: scroll up n pixels
            bool scroll_up_00bn;
            /// fxf2 instruction: set the RD.0 register to x
            bool set_rd0_fxf2;
            /// the CHIP-8X extension
            bool chip8x;
            /// the XO-CHIP extension
            bool xochip;
            /// 0000 stops the interpreter
            bool stop_0000;
            /// the chip8run extension
            bool chip8run;

        public:
            explicit chip8_instruction_set(lua_State *L){
                lua_getfield(L, -1, "instruction_set");
                if(lua_istable(L, -1)){
                    lua_getfield(L, -1, "chip8e");
                    chip8e = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "super_chip_1_0");
                    super_chip_1_0 = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "super_chip_1_1");
                    super_chip_1_1 = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "scroll_up_00bn");
                    scroll_up_00bn = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "set_rd0_fxf2");
                    set_rd0_fxf2 = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "chip8x");
                    chip8x = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "xochip");
                    xochip = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "stop_0000");
                    stop_0000 = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "chip8run");
                    chip8run = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);
                }
                lua_pop(L, 1);
            }

            /// Print the instruction set to outstream
            void print(std::ostream &outstream){
                outstream
                << "chip8e                          " << (chip8e ? "true\n" : "false\n")
                << "super_chip_1_0                  " << (super_chip_1_0 ? "true\n" : "false\n")
                << "super_chip_1_1                  " << (super_chip_1_1 ? "true\n" : "false\n")
                << "scroll_up_00bn                  " << (scroll_up_00bn ? "true\n" : "false\n")
                << "set_rd0_fxf2                    " << (set_rd0_fxf2 ? "true\n" : "false\n")
                << "chip8x                          " << (chip8x ? "true\n" : "false\n")
                << "xochip                          " << (xochip ? "true\n" : "false\n")
                << "stop_0000                       " << (stop_0000 ? "true\n" : "false\n")
                << "chip8run                        " << (chip8run ? "true\n" : "false\n");
            }
    };
}