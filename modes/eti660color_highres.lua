font = require("fonts.eti660")

return {
    x = 64,
    y = 64,
    palette = {
		type = "chip8x",
	},
    program_start = 0x700,
    instruction_set = {
		eti660 = true,
        eti660color_highres = true,
		stop_0000 = true,
	},
    font = {
        [0] = font.small,
    },
}
