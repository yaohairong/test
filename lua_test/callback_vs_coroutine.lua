os.execute('cls')
local function coroutinize(func, ...)
    local co = coroutine.create(func)
    local function exec(...)
        local ok, result = coroutine.resume(co, ...)
        if not ok then
            error(debug.traceback(co, result))
        end
        if coroutine.status(co) ~= "dead" then
            result(exec)
        end
    end
    exec(...)
end

local M = {}
function M.req_a(text1, text2, callback)
    print("running req_a, args:",  text1, text2)
    callback(text1 .. text2)
end

function M.req_b(text, callback)
    print("running req_b, args:",  text)
    callback(text)
end

local function co(co_callback, ...)
    local args = {...}
    return coroutine.yield(function(callback)
        table.insert(args, callback)
        co_callback(table.unpack(args))
    end)
end

print("callback hell:")
M.req_a("a1", "a2", function(data1)
    print("req_a return:",  data1)
    M.req_b("b1", function(data2)
        print("req_b return:",  data2)
        M.req_a("aa1", "aa2", function(data3)
            print("req_a return:", data3)
        end)
    end)
end)

local function co_m(m, func_name)
    m[func_name .. "_async"] = function(...)
        return co(m[func_name], ...)
    end
end
co_m(M, "req_a")
co_m(M, "req_b")

print("\ncoroutine:")
coroutinize(function(text1, text2)
    local data1 = M.req_a_async(text1, text2)
    print("req_a return:",  data1)
    local data2 = M.req_b_async("b1")
    print("req_b return:",  data2)
    local data3 = M.req_a_async("aa1", "aa2")
    print("req_a return:",  data3)
end, "a1", "a2")
