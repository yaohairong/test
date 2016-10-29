MonoBehaviour =
{
    meta = {},
}

setmetatable(MonoBehaviour, MonoBehaviour.meta)
function MonoBehaviour.meta:__call(a, b)
    local monoBehaviour = {}
    monoBehaviour.coroutineList = {}
    monoBehaviour.a = a
    monoBehaviour.b = b
    setmetatable(monoBehaviour, {__index = MonoBehaviour})
    return monoBehaviour
end

function MonoBehaviour:Print(c)
    print(self.a, self.b, c)
end

function MonoBehaviour:StartCoroutine(func)
    table.insert(self.coroutineList, 1, coroutine.create(func))
end

function MonoBehaviour:Update()
    local coroutineList = self.coroutineList
    for i = #coroutineList, 1, -1 do
        local co = coroutineList[i]
        if not coroutine.resume(co) then
            table.remove(coroutineList, i)
        end
    end
end

function coroutine.wait(t)
    local time = os.time()
    while os.time() - time < t do
        coroutine.yield()
    end
end

function co(index)
    return function()
        for i = 1, 5 do
            local t = os.time()
            coroutine.wait(index)
            print("wait end:", index, os.time() - t, i)
        end
    end
end

function Main()
    m1 = MonoBehaviour(1, 2)
    m2 = MonoBehaviour(3, 4)
    m1:StartCoroutine(co(1))
    m2:StartCoroutine(co(3))
    while true do
        m1:Update()
        m2:Update()
    end
end

Main()
