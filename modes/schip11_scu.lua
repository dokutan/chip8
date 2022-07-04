return {
    x = 128,
    y = 64,
    allow_high_res = true,
    instruction_set = {
        super_chip_1_0 = true,
        super_chip_1_1 = true,
        scroll_up_00bn = true,
    },
    quirks = {
        ["bnnn_bxnn_use_vx"] = true,
        ["fx55_fx65_no_increment"] = true,
        ["8xy6_8xye_shift_vx"] = true,
        ["dxyn_no_wrapping"] = true,
        ["dxy0_16x16_highres"] = true,
        ["dxy0_8x16_lowres"] = true,
        ["dxyn_count_collisions_highres"] = true,
    },
}
