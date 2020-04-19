--- Global utility definitions.
-- You can refer to these without a module qualifier, i.e.:
--
--    print(normalizeAngle(math.pi * 5));
--
-- @module utils

function pack2(...)
    return { n = select('#', ...), ... };
end

function unpack2(t)
    return unpack(t, 1, t.n);
end

--- Returns a shallow copy of arrays.
-- A variable number of additional arrays can be passed in as
-- optional arguments. If an array has a hole (a nil entry),
-- copying in a given source array stops at the last consecutive
-- item prior to the hole.
-- @tab t Array
-- @tab[opt=nil] t2 Array
-- @tab[opt=nil] tN Array
-- @treturn tab
table.copy = function(t, ...)
    local copyShallow = function(src, dst, dstStart)
        local result = dst or {};
        local resultStart = 0;
        if dst and dstStart then
            resultStart = dstStart;
        end
        local resultLen = 0;
        if "table" == type(src) then
            resultLen = #src;
            for i = 1, resultLen do
                local value = src[i];
                if nil ~= value then
                    result[i + resultStart] = value;
                else
                    resultLen = i - 1;
                    break;
                end
            end
        end
        return result, resultLen;
    end

    local result, resultStart = copyShallow(t);

    local srcs = { ... }
    for i = 1, #srcs do
        local _, len = copyShallow(srcs[i], result, resultStart);
        resultStart = resultStart + len;
    end

    return result;
end

--- Removes elements from a table based on condition.
-- @tab t Table
-- @func func Condition functor
-- @treturn int Number of elements removed
function table.remove_if(t, func)
    local num = 0;
    for k, v in pairs(t) do
        if func(k, v) then
            t[k] = nil;
            num = num + 1;
        end
    end
    return num;
end

table.size = function(t)
    local cnt = 0;
    for k, v in pairs(t) do
        cnt = cnt + 1;
    end
    return cnt;
end

table.shuffle = function(t)
    local iterations = #t;
    local j;

    for i = iterations, 2, -1 do
        j = math.random(i)
        t[i], t[j] = t[j], t[i]
    end
end

--- Executes a function after given time.
-- @number timeout After this time
-- @tparam func|tab func Functor to call
-- @param[opt] arg1
-- @param[opt] arg2
-- @param[opt] argN
-- @treturn int Timer cookier
function addTimeout(timeout, func, ...)
    if type(func) == "table" then
        local timer = nil;
        local args = pack2(...);
        local t = timeout;
        timer = scene:addTimer(function(dt)
            t = t - dt;
            if (t < 0) then
                t = timeout;
                func.update(timer, unpack2(args));
            end;
        end);
        return timer;
    else
        local timer = nil;
        local args = pack2(...);
        local t = timeout;
        timer = scene:addTimer(function(dt)
            t = t - dt;
            if (t < 0) then
                t = timeout;
                func(timer, unpack2(args));
            end;
        end);
        return timer;
    end
end

--- Executes a function after given time once.
-- @number timeout After this time
-- @tparam func|tab func Functor to call
-- @param[opt] arg1
-- @param[opt] arg2
-- @param[opt] argN
-- @treturn int Timer cookier
function addTimeoutOnce(timeout, func, ...)
    if type(func) == "table" then
        local timer = nil;
        local args = pack2(...);
        local t = timeout;
        timer = scene:addTimer(function(dt)
            t = t - dt;
            if (t < 0) then
                scene:removeTimer(timer);
                func.update(unpack2(args));
            end;
        end);
        return timer;
    else
        local timer = nil;
        local args = pack2(...);
        local t = timeout;
        timer = scene:addTimer(function(dt)
            t = t - dt;
            if (t < 0) then
                scene:removeTimer(timer);
                func(unpack2(args));
            end;
        end);
        return timer;
    end
end

function addTimeout0(func, ...)
    if type(func) == "table" then
        local timer = nil;
        local args = pack2(...);
        timer = scene:addTimer(function(dt)
            func.update(timer, dt, unpack2(args));
        end);
        return timer;
    else
        local timer = nil;
        local args = pack2(...);
        timer = scene:addTimer(function(dt)
            func(timer, dt, unpack2(args));
        end);
        return timer;
    end
end

--- Cancels timeout created by @{addTimeout}.
-- @int cookie Timer cookie returned from @{addTimeout}
function cancelTimeout(cookie)
    scene:removeTimer(cookie);
end

local function preRenderAddTimer(obj, order, func, ...)
    class 'UtilPreRenderTimer' (PhasedComponent)

    function UtilPreRenderTimer:__init(order, func, args)
        PhasedComponent.__init(self, self, const.PhasePreRender, order);
        self.func = func;
        self.args = args;
    end

    function UtilPreRenderTimer:onRegister()
    end

    function UtilPreRenderTimer:onUnregister()
    end

    function UtilPreRenderTimer:preRender(dt)
        self.func(dt, unpack2(self.args));
    end

    local c = UtilPreRenderTimer(order, func, pack2(...));

    obj:addComponent(c);

    return c;
end

function preRenderAddTimeout0(obj, order, func, ...)
    local f = func;
    local timer;
    timer = preRenderAddTimer(obj, order, function(dt, ...)
        f(timer, dt, ...);
    end, ...);
    return timer;
end

function preRenderCancelTimeout(cookie)
    cookie:removeFromParent();
end

--- Enter-only sensor listener helper.
-- @bool once Only call once
-- @func func Functor to call
-- @param[opt] arg1
-- @param[opt] arg2
-- @param[opt] argN
-- @treturn SensorListener Sensor listener
function createSensorEnterListener(once, func, ...)
    class 'UtilListener' (SensorListener)

    function UtilListener:__init(once, func, args)
        SensorListener.__init(self, self);
        self.once = once;
        self.func = func;
        self.args = args;
        self.done = false;
    end

    function UtilListener:sensorEnter(other)
        if self.once and self.done then
            return;
        end
        self.done = true;
        self.func(other, unpack2(self.args));
    end

    function UtilListener:sensorExit(other)
    end

    return UtilListener(once, func, pack2(...));
end

--- Set enter-only sensor listener.
-- @string name Sensor object name
-- @bool once Only call once
-- @func func Functor to call
-- @param[opt] arg1
-- @param[opt] arg2
-- @param[opt] argN
function setSensorEnterListener(name, once, func, ...)
    local objs = scene:getObjects(name);
    for _, obj in pairs(objs) do
        obj:findCollisionSensorComponent().listener = createSensorEnterListener(once, func, ...);
    end
end

function createSensorListener(enterFunc, exitFunc, ...)
    class 'UtilListener' (SensorListener)

    function UtilListener:__init(enterFunc, exitFunc, args)
        SensorListener.__init(self, self);
        self.enterFunc = enterFunc;
        self.exitFunc = exitFunc;
        self.args = args;
    end

    function UtilListener:sensorEnter(other)
        self.enterFunc(other, unpack2(self.args));
    end

    function UtilListener:sensorExit(other)
        self.exitFunc(other, unpack2(self.args));
    end

    return UtilListener(enterFunc, exitFunc, pack2(...));
end

function setSensorListener(name, enterFunc, exitFunc, ...)
    local obj = scene:getObjects(name)[1];
    obj:findCollisionSensorComponent().listener = createSensorListener(enterFunc, exitFunc, ...);
end
