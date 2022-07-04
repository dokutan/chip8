return {
    x = 128,
    y = 64,
    allow_high_res = true,
    instruction_set = {
        super_chip_1_0 = true,
        super_chip_1_1 = true,
    },
    quirks = {
        ["dxy0_16x16_highres"] = true,
        ["fx1e_set_vf"] = true,
        ["fx1e_overflow_at_memory_size"] = true,
    },
}
