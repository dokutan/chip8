font = require("fonts.eti660")

return {
    x = 64,
    y = 48,
    palette = {
		type = "chip8x",
	},
    program_start = 0x600,
    instruction_set = {
		eti660 = true,
        eti660color = true,
		stop_0000 = true,
	},
    font = {
        [0] = font.small,
    },
}
