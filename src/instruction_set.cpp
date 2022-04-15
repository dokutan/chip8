namespace chip8{
    /** The available instruction set extensions.
    The instruction set only determines which opcodes are available, not their behaviour.
    */
    template<
        bool t_chip8e,
        bool t_super_chip_1_0,
        bool t_super_chip_1_1,
        bool t_scroll_up_00bn,
        bool t_set_rd0_fxf2,
        bool t_chip8x,
        bool t_xochip,
        bool t_stop_0000
    > class chip8_instruction_set{
        public:
            /// the CHIP-8E extension
            static constexpr bool chip8e = t_chip8e;
            /// the SUPER-CHIP 1.0 extension
            static constexpr bool super_chip_1_0 = t_super_chip_1_0;
            /// the SUPER-CHIP 1.1 extension (without SUPER-CHIP 1.0 opcodes)
            static constexpr bool super_chip_1_1 = t_super_chip_1_1;
            //// 00bn instruction: scroll up n pixels
            static constexpr bool scroll_up_00bn = t_scroll_up_00bn;
            /// fxf2 instruction: set the RD.0 register to x
            static constexpr bool set_rd0_fxf2 = t_set_rd0_fxf2;
            /// the CHIP-8X extension
            static constexpr bool chip8x = t_chip8x;
            /// the XO-CHIP extension
            static constexpr bool xochip = t_xochip;
            /// 0000 stops the interpreter
            static constexpr bool stop_0000 = t_stop_0000;
    };
}