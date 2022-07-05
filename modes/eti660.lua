font = require("fonts.octo")

return {
    x = 64,
    y = 48,
    program_start = 0x600,
    instruction_set = {
		eti660 = true,
		stop_0000 = true,
	},
    font = {
        [0] = font.small,
    },
}
