-- Flappy Bird Clone
-- Uses muro model as bird and plant model for pipes

-- Game state
local gameState = "ready"  -- ready, playing, gameover
local score = 0
local bird = nil
local pipes = {}
local pipeSpawnTimer = 0
local pipeSpawnInterval = 2.0
local pipeSpeed = 8.0
local flapForce = 12.0
local birdStartPos = Vec3(-8.0, 5.0, 0.0)

-- Pipe configuration
local pipeGap = 5.0  -- Gap between top and bottom pipes
local pipeMinY = 2.0
local pipeMaxY = 8.0
local pipeScale = Vec3(0.8, 0.8, 0.8)

-- Get references
bird = scene:getObjects("bird")[1]

-- Freeze bird initially
if bird then
    bird.linearVelocity = Vec3.zero
    bird.angularVelocity = Vec3.zero
end

-- Create a pipe pair at given x position
local function createPipePair(xPos)
    local gapCenterY = pipeMinY + math.random() * (pipeMaxY - pipeMinY)

    -- Bottom pipe (pointing up)
    local bottomPipe = factory:createMeshObject("Low-Poly Plant_.fbx", pipeScale)
    bottomPipe.name = "pipe_bottom"
    bottomPipe.bodyType = const.BodyTypeKinematic
    local bottomY = gapCenterY - pipeGap / 2 - 5.0  -- Offset to position correctly
    bottomPipe:setTransform(Vec3(xPos, bottomY, 0.0), Quaternion.identity)

    -- Add physics for collision detection
    local bottomPc = PhysicsBodyComponent()
    local bottomShape = CollisionShapeBox(Vec3(1.5, 10.0, 1.5))
    bottomShape.mass = 0
    bottomPc:addShape(bottomShape)
    bottomPipe:addComponent(bottomPc)

    scene:addObject(bottomPipe)

    -- Top pipe (pointing down - rotated 180 degrees)
    local topPipe = factory:createMeshObject("Low-Poly Plant_.fbx", pipeScale)
    topPipe.name = "pipe_top"
    topPipe.bodyType = const.BodyTypeKinematic
    local topY = gapCenterY + pipeGap / 2 + 5.0
    topPipe:setTransform(Vec3(xPos, topY, 0.0), Quaternion(Vec3(0, 0, 1), math.pi))

    -- Add physics for collision detection
    local topPc = PhysicsBodyComponent()
    local topShape = CollisionShapeBox(Vec3(1.5, 10.0, 1.5))
    topShape.mass = 0
    topPc:addShape(topShape)
    topPipe:addComponent(topPc)

    scene:addObject(topPipe)

    -- Score trigger (invisible, between the pipes)
    local scoreTrigger = factory:createSensor(false)
    scoreTrigger.name = "score_trigger"
    scoreTrigger.bodyType = const.BodyTypeKinematic
    scoreTrigger:setTransform(Vec3(xPos, gapCenterY, 0.0), Quaternion.identity)

    local triggerPc = scoreTrigger:findPhysicsBodyComponent()
    if not triggerPc then
        triggerPc = PhysicsBodyComponent()
        scoreTrigger:addComponent(triggerPc)
    end
    local triggerShape = CollisionShapeBox(Vec3(0.5, pipeGap, 2.0))
    triggerShape.mass = 0
    triggerPc:addShape(triggerShape)

    scene:addObject(scoreTrigger)

    -- Store pipe data
    local pipeData = {
        bottom = bottomPipe,
        top = topPipe,
        trigger = scoreTrigger,
        scored = false,
        x = xPos
    }

    table.insert(pipes, pipeData)

    return pipeData
end

-- Remove a pipe pair
local function removePipePair(pipeData)
    if pipeData.bottom then
        pipeData.bottom:removeFromParent()
    end
    if pipeData.top then
        pipeData.top:removeFromParent()
    end
    if pipeData.trigger then
        pipeData.trigger:removeFromParent()
    end
end

-- Reset game
local function resetGame()
    -- Remove all pipes
    for _, pipeData in ipairs(pipes) do
        removePipePair(pipeData)
    end
    pipes = {}

    -- Reset bird position and velocity
    if bird then
        bird:setTransform(birdStartPos, Quaternion(Vec3(0, 1, 0), math.pi / 2))
        bird.linearVelocity = Vec3.zero
        bird.angularVelocity = Vec3.zero
    end

    score = 0
    pipeSpawnTimer = 0
    gameState = "ready"

    print("Game Reset - Press SPACE to start!")
end

-- Flap the bird
local function flap()
    if bird and (gameState == "playing" or gameState == "ready") then
        if gameState == "ready" then
            gameState = "playing"
            print("Game Started!")
        end

        -- Apply upward velocity
        local vel = bird.linearVelocity
        bird.linearVelocity = Vec3(0.0, flapForce, 0.0)

        -- Add slight rotation for visual effect
        bird.angularVelocity = Vec3(0.0, 0.0, 2.0)
    end
end

-- Check for collision (simple AABB)
local function checkBirdCollision()
    if not bird then return false end

    local birdPos = bird.pos
    local birdRadius = 0.8

    -- Check ground collision
    if birdPos.y < -3.5 then
        return true
    end

    -- Check ceiling
    if birdPos.y > 15.0 then
        return true
    end

    -- Check pipe collisions
    for _, pipeData in ipairs(pipes) do
        local pipeX = pipeData.x

        -- Only check pipes near the bird
        if math.abs(birdPos.x - pipeX) < 2.5 then
            -- Get gap center (approximate from pipe positions)
            local bottomY = pipeData.bottom.pos.y + 5.0
            local topY = pipeData.top.pos.y - 5.0
            local gapCenter = (bottomY + topY) / 2

            -- Check if bird is outside the gap
            if birdPos.y - birdRadius < bottomY or birdPos.y + birdRadius > topY then
                return true
            end
        end
    end

    return false
end

-- Update score
local function checkScore()
    if not bird then return end

    local birdX = bird.pos.x

    for _, pipeData in ipairs(pipes) do
        if not pipeData.scored and pipeData.x < birdX then
            pipeData.scored = true
            score = score + 1
            print("Score: " .. score)
        end
    end
end

-- Main game update
local function gameUpdate(timer, dt)
    -- Handle input
    if input:keyboard():triggered(const.KI_SPACE) then
        if gameState == "gameover" then
            resetGame()
        else
            flap()
        end
    end

    -- Restart with R key
    if input:keyboard():triggered(const.KI_R) then
        resetGame()
    end

    -- Quit with Escape
    if input:keyboard():triggered(const.KI_ESCAPE) then
        scene.quit = true
    end

    if gameState ~= "playing" then
        -- Keep bird from falling in ready state
        if gameState == "ready" and bird then
            bird.linearVelocity = Vec3.zero
            bird.angularVelocity = Vec3.zero
        end
        return
    end

    -- Update pipe spawn timer
    pipeSpawnTimer = pipeSpawnTimer + dt
    if pipeSpawnTimer >= pipeSpawnInterval then
        pipeSpawnTimer = 0
        createPipePair(15.0)  -- Spawn pipes ahead of bird
    end

    -- Move pipes toward bird
    local pipesToRemove = {}
    for i, pipeData in ipairs(pipes) do
        pipeData.x = pipeData.x - pipeSpeed * dt

        -- Update pipe positions
        if pipeData.bottom then
            local pos = pipeData.bottom.pos
            pipeData.bottom:setTransform(Vec3(pipeData.x, pos.y, pos.z), pipeData.bottom.rotation)
        end
        if pipeData.top then
            local pos = pipeData.top.pos
            pipeData.top:setTransform(Vec3(pipeData.x, pos.y, pos.z), pipeData.top.rotation)
        end
        if pipeData.trigger then
            local pos = pipeData.trigger.pos
            pipeData.trigger:setTransform(Vec3(pipeData.x, pos.y, pos.z), Quaternion.identity)
        end

        -- Mark pipes for removal if they're behind the bird
        if pipeData.x < -20.0 then
            table.insert(pipesToRemove, i)
        end
    end

    -- Remove old pipes (in reverse order to maintain indices)
    for i = #pipesToRemove, 1, -1 do
        local idx = pipesToRemove[i]
        removePipePair(pipes[idx])
        table.remove(pipes, idx)
    end

    -- Keep bird at fixed X position
    if bird then
        local pos = bird.pos
        local vel = bird.linearVelocity
        bird:setTransform(Vec3(birdStartPos.x, pos.y, 0.0), bird.rotation)
        bird.linearVelocity = Vec3(0.0, vel.y, 0.0)  -- Only keep Y velocity

        -- Clamp angular velocity
        local angVel = bird.angularVelocity
        if angVel.z < -3.0 then
            bird.angularVelocity = Vec3(0.0, 0.0, -3.0)
        end
    end

    -- Check collisions
    if checkBirdCollision() then
        gameState = "gameover"
        print("Game Over! Final Score: " .. score)
        print("Press SPACE or R to restart")

        -- Let bird fall
        if bird then
            bird.linearVelocity = Vec3(0.0, -5.0, 0.0)
        end
    end

    -- Check score
    checkScore()
end

-- Initialize
print("=== FLAPPY BIRD ===")
print("Press SPACE to flap")
print("Press R to restart")
print("Press ESC to quit")
print("==================")

-- Set initial bird rotation
if bird then
    bird:setTransform(birdStartPos, Quaternion(Vec3(0, 1, 0), math.pi / 2))
end

-- Start the game timer
addTimeout0(gameUpdate)
