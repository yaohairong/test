os.execute('cls')
local function req_a(text1, text2, callback)
    print("running req_a, args:",  text1, text2)
    callback(text1 .. text2)
end

local function req_b(text, callback)
    print("running req_b, args:",  text)
    callback(text)
end

print("callback hell:")
req_a("a1", "a2", function(data1)
    print("req_a return:",  data1)
    req_b("b1", function(data2)
        print("req_b return:",  data2)
        req_a("aa1", "aa2", function(data3)
            print("req_a return:", data3)
        end)
    end)
end)

local function async(co_func)
    local co = coroutine.create(co_func)
    local function exec(...)
        local ok, result = coroutine.resume(co, ...)
        if not ok then
            error(debug.traceback(co, result))
        end
        if coroutine.status(co) ~= "dead" then
            result(exec)
        end
    end
    exec()
end

local function await(async_func, ...)
    local args = {...}
    return coroutine.yield(function(async_callback)
        table.insert(args, async_callback)
        async_func(table.unpack(args))
    end)
end

print("\ncoroutine:")
async(function()
    local data1 = await(req_a, "a1", "a2")
    print("req_a return:",  data1)
    local data2 = await(req_b, "b1")
    print("req_b return:",  data2)
    local data3 = await(req_a, "aa1", "aa2")
    print("req_a return:",  data3)
end)
