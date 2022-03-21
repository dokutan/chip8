namespace chip8{
    /** The available instruction set extensions.
    The instruction set only determines which opcodes are available, not their behaviour.
    */
    template<
        bool t_chip8e,
        bool t_super_chip_1_0,
        bool t_super_chip_1_1,
        bool t_fxf2_set_rd0,
        bool t_chip8x
    > class chip8_instruction_set{
        public:
            /// the CHIP-8E extension
            static constexpr bool chip8e = t_chip8e;
            /// the SUPER-CHIP 1.0 extension
            static constexpr bool super_chip_1_0 = t_super_chip_1_0;
            /// the SUPER-CHIP 1.1 extension (without SUPER-CHIP 1.0 opcodes)
            static constexpr bool super_chip_1_1 = t_super_chip_1_1;
            /// fxf2 instruction: set the RD.0 register to x
            static constexpr bool fxf2_set_rd0 = t_fxf2_set_rd0;
            /// The CHIP-8X extension
            static constexpr bool chip8x = t_chip8x;
    };
}