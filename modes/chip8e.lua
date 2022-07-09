return {
	instruction_set = {
		chip8e = true,
	},
	output_port_3 = function(x) -- called for fx03
		print(string.format("port 3: %x", x))
	end,
	input_port_3_wait = function() -- called for fxe3
		return 0
	end,
	input_port_3 = function() -- called for fxe7
		return 0
	end,
}
