return {
    memory_size = 65536,
    x = 128,
    y = 64,
    planes = 2,
    allow_high_res = true,
    palette = {
        type = "xochip",
    },
    instruction_set = {
        super_chip_1_0 = true,
        super_chip_1_1 = true,
        xochip = true,
    },
    quirks = {
        ["dxy0_16x16_highres"] = true,
        ["dxy0_16x16_lowres"] = true,
        ["fx75_fx85_allow_all"] = true,
        ["00fe_00ff_clear_screen"] = true,
        ["00fe_00ff_clear_all_planes"] = true,
    },
}
