-- print('huge=', math.huge)
-- print('pi=', math.pi)
-- print('maxinteger=', math.maxinteger)
-- print('mininteger=', math.mininteger)
-- print('math.type(0.25)', math.type(0.25))
-- print('math.type(25)', math.type(25))

-- local a = 10

-- local right_shift_result = a >> 2 -- 结果为 2，二进制：0010
-- print(right_shift_result)

-- test yhlib
local sum = yhlib.add(1, 2, 3, 5.0)
print(sum, math.type(sum))
print(math.min(1, 2, 3, 4))
local t = yhlib.add(2, 3, 8)
print(t)