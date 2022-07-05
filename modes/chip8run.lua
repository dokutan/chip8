return {
	x = 128,
	y = 64,
	allow_high_res = true,
	instruction_set = {
		super_chip_1_0 = true,
		super_chip_1_1 = true,
		chip8run = true,
	},
	quirks = {
		["8xy6_8xye_shift_vx"] = true,
		["dxy0_16x16_highres"] = true,
		["dxyn_no_wrapping"] = true,
	},
}
