local str = "000900630500600100004000008902805361175260480006019275437091026269348507800700043"
local sudoku = {}
for row = 1, 9 do
	sudoku[row] = {}
	for col = 1, 9 do
		local index = (row - 1) * 9 + col
		sudoku[row][col] = tonumber(string.sub(str, index, index))
	end
end

local function Print()
	for _, v in ipairs(sudoku) do
		for _, vv in ipairs(v) do
			io.write(vv, " ")
		end
		io.write("\n")
	end
	io.write("\n")
end

local function IsOK(row, col)
	for i = 1, 9 do
		if i ~= col and sudoku[row][i] == sudoku[row][col] then
			return false
		end
	end
	for i = 1, 9 do
		if i ~= row and sudoku[i][col] == sudoku[row][col] then
			return false
		end
	end
	local s_row = math.floor((row - 1) / 3) * 3 + 1
	local s_col = math.floor((col - 1) / 3) * 3 + 1
	for i = s_row, s_row + 2 do
		for j = s_col, s_col + 2 do
			if (i ~= row or j ~= col) and sudoku[i][j] == sudoku[row][col] then
				return false
			end
		end
	end
	return true
end

local function TraceBack(row, col)
	if row == 10 then
		Print()
		return
	end
	if sudoku[row][col] ~= 0 then
		if col == 9 then
			TraceBack(row + 1, 1)
		else
			TraceBack(row, col + 1)
		end
	else
		for i = 1, 9 do
			sudoku[row][col] = i
			if IsOK(row, col) then
				if col == 9 then
					TraceBack(row + 1, 1)
				else
					TraceBack(row, col + 1)
				end
			end
		end
		sudoku[row][col] = 0
	end
end
Print()
TraceBack(1, 1)
