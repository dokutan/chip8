return {
	instruction_set = {
		chip8elf = true,
	},
	print_hex = function(x)
		print(string.format("hex display: %x", x))
	end,
}
