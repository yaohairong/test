local function req(arg1, arg2, callback)
    print("running req, args:", arg1, arg2)
    callback(arg1 .. arg2)
end
local function req_async(arg1, arg2)
    local function yield_func(callback)
        req(arg1, arg2, callback)
    end
    return coroutine.yield(yield_func)
end
local co = coroutine.create(function()
    print("ret:", req_async("arg1", "arg2"))
end)
local function exec(...)
    local ok, result = coroutine.resume(co, ...)
    if not ok then
        error(debug.traceback(co, result))
    end
    if coroutine.status(co) ~= "dead" then
        result(exec)
    end
end
local function trace(_, line)
    local info = debug.getinfo(2)
    print(line, info.name)
end
debug.sethook(trace, "l")
exec()
