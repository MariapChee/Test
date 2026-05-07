#include "ai_chat.h"
#include "theme.h"
#include "utils.h"
#include <chrono>
#include <cstring>
#include <algorithm>
#include <sstream>

CrynosAI::CrynosAI() { init_templates(); }

std::string CrynosAI::to_lower(const std::string& s) const {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(), ::tolower);
    return r;
}

int CrynosAI::match_score(const std::string& input, const std::vector<std::string>& keywords) const {
    std::string low = to_lower(input);
    int score = 0;
    for (const auto& kw : keywords) {
        if (low.find(kw) != std::string::npos) {
            score += (int)kw.size();
        }
    }
    return score;
}

void CrynosAI::init_templates() {
    templates_ = {
        {"Speed Hack", {"speed", "fast", "walkspeed", "walk speed", "run"},
         "-- Crynos Speed Script\n-- Adjustable walk speed for your character\n\nlocal Players = game:GetService(\"Players\")\nlocal player = Players.LocalPlayer\nlocal character = player.Character or player.CharacterAdded:Wait()\nlocal humanoid = character:WaitForChild(\"Humanoid\")\n\nlocal SPEED = 100 -- Change this value (default is 16)\n\nhumanoid.WalkSpeed = SPEED\n\n-- Re-apply on respawn\nplayer.CharacterAdded:Connect(function(char)\n    local hum = char:WaitForChild(\"Humanoid\")\n    hum.WalkSpeed = SPEED\nend)\n\nprint(\"[Crynos] Speed set to \" .. SPEED)",
         "Sets your character walk speed to a custom value"},
        {"Fly Script", {"fly", "flying", "flight", "hover"},
         "-- Crynos Fly Script\n-- Press E to toggle flying, WASD to move, Space/Shift for up/down\n\nlocal Players = game:GetService(\"Players\")\nlocal UIS = game:GetService(\"UserInputService\")\nlocal RunService = game:GetService(\"RunService\")\nlocal player = Players.LocalPlayer\nlocal character = player.Character or player.CharacterAdded:Wait()\nlocal humanoidRootPart = character:WaitForChild(\"HumanoidRootPart\")\nlocal humanoid = character:WaitForChild(\"Humanoid\")\n\nlocal flying = false\nlocal FLY_SPEED = 80\nlocal bodyVelocity, bodyGyro\n\nlocal function startFly()\n    flying = true\n    bodyVelocity = Instance.new(\"BodyVelocity\")\n    bodyVelocity.MaxForce = Vector3.new(math.huge, math.huge, math.huge)\n    bodyVelocity.Velocity = Vector3.new(0, 0, 0)\n    bodyVelocity.Parent = humanoidRootPart\n    bodyGyro = Instance.new(\"BodyGyro\")\n    bodyGyro.MaxTorque = Vector3.new(math.huge, math.huge, math.huge)\n    bodyGyro.P = 9000\n    bodyGyro.Parent = humanoidRootPart\n    humanoid.PlatformStand = true\n    print(\"[Crynos] Flying enabled\")\nend\n\nlocal function stopFly()\n    flying = false\n    if bodyVelocity then bodyVelocity:Destroy() end\n    if bodyGyro then bodyGyro:Destroy() end\n    humanoid.PlatformStand = false\n    print(\"[Crynos] Flying disabled\")\nend\n\nUIS.InputBegan:Connect(function(input, processed)\n    if processed then return end\n    if input.KeyCode == Enum.KeyCode.E then\n        if flying then stopFly() else startFly() end\n    end\nend)\n\nRunService.Heartbeat:Connect(function()\n    if not flying then return end\n    local camera = workspace.CurrentCamera\n    local moveDir = Vector3.new(0, 0, 0)\n    if UIS:IsKeyDown(Enum.KeyCode.W) then moveDir = moveDir + camera.CFrame.LookVector end\n    if UIS:IsKeyDown(Enum.KeyCode.S) then moveDir = moveDir - camera.CFrame.LookVector end\n    if UIS:IsKeyDown(Enum.KeyCode.A) then moveDir = moveDir - camera.CFrame.RightVector end\n    if UIS:IsKeyDown(Enum.KeyCode.D) then moveDir = moveDir + camera.CFrame.RightVector end\n    if UIS:IsKeyDown(Enum.KeyCode.Space) then moveDir = moveDir + Vector3.new(0, 1, 0) end\n    if UIS:IsKeyDown(Enum.KeyCode.LeftShift) then moveDir = moveDir - Vector3.new(0, 1, 0) end\n    if moveDir.Magnitude > 0 then moveDir = moveDir.Unit end\n    bodyVelocity.Velocity = moveDir * FLY_SPEED\n    bodyGyro.CFrame = camera.CFrame\nend)\n\nprint(\"[Crynos] Fly script loaded - Press E to toggle\")",
         "Allows your character to fly with keyboard controls"},
        {"ESP / Wallhack", {"esp", "wallhack", "wall hack", "highlight", "see through", "xray", "chams"},
         "-- Crynos ESP Script\n-- Highlights all players through walls\n\nlocal Players = game:GetService(\"Players\")\nlocal RunService = game:GetService(\"RunService\")\nlocal player = Players.LocalPlayer\nlocal ESP_COLOR = Color3.fromRGB(0, 255, 255)\n\nlocal function createESP(targetPlayer)\n    if targetPlayer == player then return end\n    local function setup(character)\n        if not character then return end\n        local highlight = Instance.new(\"Highlight\")\n        highlight.Name = \"CrynosESP\"\n        highlight.FillColor = ESP_COLOR\n        highlight.FillTransparency = 0.7\n        highlight.OutlineColor = ESP_COLOR\n        highlight.OutlineTransparency = 0.3\n        highlight.Adornee = character\n        highlight.Parent = character\n        local head = character:WaitForChild(\"Head\", 5)\n        if not head then return end\n        local billboard = Instance.new(\"BillboardGui\")\n        billboard.Size = UDim2.new(0, 200, 0, 50)\n        billboard.StudsOffset = Vector3.new(0, 3, 0)\n        billboard.AlwaysOnTop = true\n        billboard.Adornee = head\n        billboard.Parent = character\n        local nameLabel = Instance.new(\"TextLabel\")\n        nameLabel.Size = UDim2.new(1, 0, 0.5, 0)\n        nameLabel.BackgroundTransparency = 1\n        nameLabel.Text = targetPlayer.Name\n        nameLabel.TextColor3 = ESP_COLOR\n        nameLabel.Font = Enum.Font.GothamBold\n        nameLabel.TextSize = 14\n        nameLabel.Parent = billboard\n    end\n    if targetPlayer.Character then setup(targetPlayer.Character) end\n    targetPlayer.CharacterAdded:Connect(setup)\nend\n\nfor _, p in pairs(Players:GetPlayers()) do createESP(p) end\nPlayers.PlayerAdded:Connect(createESP)\nprint(\"[Crynos] ESP loaded for all players\")",
         "Shows all players through walls with highlights"},
        {"God Mode", {"god", "godmode", "god mode", "immortal", "invincible", "infinite health", "inf health"},
         "-- Crynos God Mode Script\n\nlocal Players = game:GetService(\"Players\")\nlocal player = Players.LocalPlayer\n\nlocal function applyGodMode(character)\n    local humanoid = character:WaitForChild(\"Humanoid\")\n    humanoid.MaxHealth = math.huge\n    humanoid.Health = math.huge\n    humanoid.HealthChanged:Connect(function(health)\n        if health < humanoid.MaxHealth then humanoid.Health = humanoid.MaxHealth end\n    end)\n    print(\"[Crynos] God mode activated\")\nend\n\nif player.Character then applyGodMode(player.Character) end\nplayer.CharacterAdded:Connect(function(char) wait(0.5) applyGodMode(char) end)\nprint(\"[Crynos] God mode script loaded\")",
         "Makes your character invincible with infinite health"},
        {"Noclip", {"noclip", "no clip", "ghost", "walk through", "phase"},
         "-- Crynos Noclip Script\n-- Press N to toggle\n\nlocal Players = game:GetService(\"Players\")\nlocal RunService = game:GetService(\"RunService\")\nlocal UIS = game:GetService(\"UserInputService\")\nlocal player = Players.LocalPlayer\nlocal noclip = false\n\nUIS.InputBegan:Connect(function(input, processed)\n    if processed then return end\n    if input.KeyCode == Enum.KeyCode.N then\n        noclip = not noclip\n        print(\"[Crynos] Noclip \" .. (noclip and \"enabled\" or \"disabled\"))\n    end\nend)\n\nRunService.Stepped:Connect(function()\n    if noclip and player.Character then\n        for _, part in pairs(player.Character:GetDescendants()) do\n            if part:IsA(\"BasePart\") then part.CanCollide = false end\n        end\n    end\nend)\nprint(\"[Crynos] Noclip loaded - Press N to toggle\")",
         "Walk through walls by toggling noclip mode"},
        {"Teleport Script", {"teleport", "tp", "goto", "go to", "warp"},
         "-- Crynos Teleport Script\n\nlocal Players = game:GetService(\"Players\")\nlocal player = Players.LocalPlayer\n\nlocal function teleportNearest()\n    local myChar = player.Character\n    if not myChar or not myChar:FindFirstChild(\"HumanoidRootPart\") then return end\n    local nearest, minDist = nil, math.huge\n    for _, p in pairs(Players:GetPlayers()) do\n        if p ~= player and p.Character and p.Character:FindFirstChild(\"HumanoidRootPart\") then\n            local dist = (p.Character.HumanoidRootPart.Position - myChar.HumanoidRootPart.Position).Magnitude\n            if dist < minDist then nearest = p; minDist = dist end\n        end\n    end\n    if nearest then\n        myChar.HumanoidRootPart.CFrame = nearest.Character.HumanoidRootPart.CFrame + Vector3.new(0, 3, 0)\n        print(\"[Crynos] Teleported to \" .. nearest.Name)\n    end\nend\n\nteleportNearest()\nprint(\"[Crynos] Teleport script loaded\")",
         "Teleports your character to the nearest player"},
        {"Infinite Jump", {"infinite jump", "inf jump", "double jump", "multi jump", "jump hack"},
         "-- Crynos Infinite Jump Script\n\nlocal Players = game:GetService(\"Players\")\nlocal UIS = game:GetService(\"UserInputService\")\nlocal player = Players.LocalPlayer\n\nUIS.JumpRequest:Connect(function()\n    local character = player.Character\n    if character then\n        local humanoid = character:FindFirstChildOfClass(\"Humanoid\")\n        if humanoid then humanoid:ChangeState(Enum.HumanoidStateType.Jumping) end\n    end\nend)\nprint(\"[Crynos] Infinite Jump enabled\")",
         "Jump unlimited times even while in the air"},
        {"Auto Farm", {"auto farm", "autofarm", "farm", "auto collect", "auto grind", "grind"},
         "-- Crynos Auto Farm Script\n\nlocal Players = game:GetService(\"Players\")\nlocal player = Players.LocalPlayer\nlocal farming = true\n\nspawn(function()\n    while farming and wait(0.1) do\n        local myChar = player.Character\n        if not myChar then continue end\n        local hrp = myChar:FindFirstChild(\"HumanoidRootPart\")\n        if not hrp then continue end\n        for _, obj in pairs(workspace:GetDescendants()) do\n            if obj:IsA(\"BasePart\") and (obj.Name:lower():find(\"coin\") or obj.Name:lower():find(\"gem\")) then\n                local dist = (obj.Position - hrp.Position).Magnitude\n                if dist < 50 then hrp.CFrame = obj.CFrame; wait(0.05) end\n            end\n        end\n    end\nend)\nprint(\"[Crynos] Auto Farm started\")",
         "Automatically collects nearby items"},
        {"Aimbot", {"aimbot", "aim bot", "aim assist", "auto aim", "aim lock", "lock on"},
         "-- Crynos Aimbot Script\n-- Hold right-click to lock aim\n\nlocal Players = game:GetService(\"Players\")\nlocal RunService = game:GetService(\"RunService\")\nlocal UIS = game:GetService(\"UserInputService\")\nlocal Camera = workspace.CurrentCamera\nlocal player = Players.LocalPlayer\nlocal aiming = false\n\nlocal function getClosest()\n    local closest, minDist = nil, 200\n    for _, p in pairs(Players:GetPlayers()) do\n        if p ~= player and p.Character then\n            local head = p.Character:FindFirstChild(\"Head\")\n            local hum = p.Character:FindFirstChild(\"Humanoid\")\n            if head and hum and hum.Health > 0 then\n                local sp, onScreen = Camera:WorldToScreenPoint(head.Position)\n                if onScreen then\n                    local d = (Vector2.new(sp.X, sp.Y) - UIS:GetMouseLocation()).Magnitude\n                    if d < minDist then closest = head; minDist = d end\n                end\n            end\n        end\n    end\n    return closest\nend\n\nUIS.InputBegan:Connect(function(i) if i.UserInputType == Enum.UserInputType.MouseButton2 then aiming = true end end)\nUIS.InputEnded:Connect(function(i) if i.UserInputType == Enum.UserInputType.MouseButton2 then aiming = false end end)\n\nRunService.RenderStepped:Connect(function()\n    if not aiming then return end\n    local t = getClosest()\n    if t then Camera.CFrame = CFrame.new(Camera.CFrame.Position, t.Position) end\nend)\nprint(\"[Crynos] Aimbot loaded - Hold right-click\")",
         "Auto-aims at the nearest player"},
        {"Kill Aura", {"kill", "kill all", "kill aura", "fling", "destroy", "attack"},
         "-- Crynos Kill Aura / Fling Script\n\nlocal Players = game:GetService(\"Players\")\nlocal RunService = game:GetService(\"RunService\")\nlocal player = Players.LocalPlayer\n\nRunService.Heartbeat:Connect(function()\n    local myChar = player.Character\n    if not myChar or not myChar:FindFirstChild(\"HumanoidRootPart\") then return end\n    local myHrp = myChar.HumanoidRootPart\n    for _, p in pairs(Players:GetPlayers()) do\n        if p ~= player and p.Character then\n            local hrp = p.Character:FindFirstChild(\"HumanoidRootPart\")\n            if hrp and (hrp.Position - myHrp.Position).Magnitude < 30 then\n                hrp.Velocity = Vector3.new(math.random(-9999, 9999), 9999, math.random(-9999, 9999))\n            end\n        end\n    end\nend)\nprint(\"[Crynos] Kill Aura active\")",
         "Flings nearby players with high force"},
        {"Teleport GUI", {"gui", "teleport gui", "tp gui", "player gui", "player list", "menu"},
         "-- Crynos Teleport GUI\n\nlocal Players = game:GetService(\"Players\")\nlocal player = Players.LocalPlayer\nlocal sg = Instance.new(\"ScreenGui\")\nsg.Name = \"CrynosTeleportGUI\"\nsg.ResetOnSpawn = false\nsg.Parent = game:GetService(\"CoreGui\")\n\nlocal frame = Instance.new(\"Frame\")\nframe.Size = UDim2.new(0, 250, 0, 350)\nframe.Position = UDim2.new(0, 20, 0.5, -175)\nframe.BackgroundColor3 = Color3.fromRGB(25, 25, 35)\nframe.BorderSizePixel = 0\nframe.Parent = sg\nInstance.new(\"UICorner\", frame).CornerRadius = UDim.new(0, 12)\n\nlocal title = Instance.new(\"TextLabel\")\ntitle.Size = UDim2.new(1, 0, 0, 40)\ntitle.BackgroundColor3 = Color3.fromRGB(0, 255, 255)\ntitle.TextColor3 = Color3.fromRGB(0, 0, 0)\ntitle.Font = Enum.Font.GothamBold; title.TextSize = 16\ntitle.Text = \"Crynos Teleport\"\ntitle.Parent = frame\n\nlocal scroll = Instance.new(\"ScrollingFrame\")\nscroll.Size = UDim2.new(1, -20, 1, -50)\nscroll.Position = UDim2.new(0, 10, 0, 45)\nscroll.BackgroundTransparency = 1\nscroll.ScrollBarThickness = 4\nscroll.Parent = frame\nlocal layout = Instance.new(\"UIListLayout\")\nlayout.Padding = UDim.new(0, 5)\nlayout.Parent = scroll\n\nlocal function refreshList()\n    for _, c in pairs(scroll:GetChildren()) do if c:IsA(\"TextButton\") then c:Destroy() end end\n    for _, p in pairs(Players:GetPlayers()) do\n        if p ~= player then\n            local btn = Instance.new(\"TextButton\")\n            btn.Size = UDim2.new(1, 0, 0, 35)\n            btn.BackgroundColor3 = Color3.fromRGB(35, 35, 50)\n            btn.TextColor3 = Color3.new(1, 1, 1)\n            btn.Font = Enum.Font.Gotham; btn.TextSize = 14\n            btn.Text = p.Name; btn.Parent = scroll\n            Instance.new(\"UICorner\", btn).CornerRadius = UDim.new(0, 8)\n            btn.MouseButton1Click:Connect(function()\n                if p.Character and p.Character:FindFirstChild(\"HumanoidRootPart\") then\n                    local mc = player.Character\n                    if mc and mc:FindFirstChild(\"HumanoidRootPart\") then\n                        mc.HumanoidRootPart.CFrame = p.Character.HumanoidRootPart.CFrame + Vector3.new(0,3,0)\n                    end\n                end\n            end)\n        end\n    end\n    scroll.CanvasSize = UDim2.new(0, 0, 0, layout.AbsoluteContentSize.Y)\nend\n\nrefreshList()\nPlayers.PlayerAdded:Connect(function() wait(1) refreshList() end)\nprint(\"[Crynos] Teleport GUI created\")",
         "Creates a player list GUI to teleport to any player"},
        {"Jump Power", {"jump", "jump power", "jump high", "super jump", "high jump", "jump boost"},
         "-- Crynos Jump Power Script\n\nlocal Players = game:GetService(\"Players\")\nlocal player = Players.LocalPlayer\nlocal character = player.Character or player.CharacterAdded:Wait()\nlocal humanoid = character:WaitForChild(\"Humanoid\")\nlocal JUMP_POWER = 150\n\nhumanoid.UseJumpPower = true\nhumanoid.JumpPower = JUMP_POWER\nplayer.CharacterAdded:Connect(function(char)\n    local hum = char:WaitForChild(\"Humanoid\")\n    hum.UseJumpPower = true; hum.JumpPower = JUMP_POWER\nend)\nprint(\"[Crynos] Jump Power set to \" .. JUMP_POWER)",
         "Increases your character jump height"},
        {"Low Gravity", {"gravity", "low gravity", "moon", "moon gravity"},
         "-- Crynos Low Gravity Script\n\nworkspace.Gravity = 40\nprint(\"[Crynos] Gravity set to 40 (default: 196.2)\")",
         "Reduces game gravity for a floaty feel"},
        {"Fullbright", {"fullbright", "bright", "no dark", "lighting", "see in dark", "night vision"},
         "-- Crynos Fullbright Script\n\nlocal Lighting = game:GetService(\"Lighting\")\nLighting.Brightness = 2\nLighting.ClockTime = 14\nLighting.FogEnd = 100000\nLighting.GlobalShadows = false\nLighting.OutdoorAmbient = Color3.fromRGB(128, 128, 128)\nLighting.Ambient = Color3.fromRGB(178, 178, 178)\n\nfor _, effect in pairs(Lighting:GetChildren()) do\n    if effect:IsA(\"Atmosphere\") or effect:IsA(\"BlurEffect\") then\n        effect:Destroy()\n    end\nend\nprint(\"[Crynos] Fullbright enabled\")",
         "Makes everything bright and removes darkness"},
        {"Click Teleport", {"click tp", "click teleport", "mouse teleport", "click to tp"},
         "-- Crynos Click TP Script\n-- Ctrl+Click to teleport\n\nlocal Players = game:GetService(\"Players\")\nlocal UIS = game:GetService(\"UserInputService\")\nlocal Mouse = Players.LocalPlayer:GetMouse()\n\nMouse.Button1Down:Connect(function()\n    if UIS:IsKeyDown(Enum.KeyCode.LeftControl) then\n        local c = Players.LocalPlayer.Character\n        if c and c:FindFirstChild(\"HumanoidRootPart\") and Mouse.Hit then\n            c.HumanoidRootPart.CFrame = Mouse.Hit + Vector3.new(0, 5, 0)\n        end\n    end\nend)\nprint(\"[Crynos] Click TP loaded - Ctrl+Click to teleport\")",
         "Ctrl+Click anywhere to teleport there"},
        {"Anti AFK", {"anti afk", "afk", "no kick", "anti idle", "stay online"},
         "-- Crynos Anti-AFK Script\n\nlocal VirtualUser = game:GetService(\"VirtualUser\")\nlocal player = game:GetService(\"Players\").LocalPlayer\n\nplayer.Idled:Connect(function()\n    VirtualUser:CaptureController()\n    VirtualUser:ClickButton2(Vector2.new())\nend)\nprint(\"[Crynos] Anti-AFK enabled\")",
         "Prevents AFK kick by simulating activity"},
        {"Chat Spam", {"chat", "spam", "chat spam", "message spam"},
         "-- Crynos Chat Spam Script\n\nlocal MESSAGE = \"Crynos Executor!\"\nlocal DELAY = 2\n\nspawn(function()\n    while wait(DELAY) do\n        game:GetService(\"ReplicatedStorage\"):FindFirstChild(\"DefaultChatSystemChatEvents\")\n            :FindFirstChild(\"SayMessageRequest\"):FireServer(MESSAGE, \"All\")\n    end\nend)\nprint(\"[Crynos] Chat spam started\")",
         "Spams a custom message in chat"},
        {"Mega Script", {"combo", "all", "everything", "all hacks", "all scripts", "mega", "full"},
         "-- Crynos Mega Script - All-in-One\n-- Speed + Jump + Noclip + Infinite Jump\n\nlocal Players = game:GetService(\"Players\")\nlocal UIS = game:GetService(\"UserInputService\")\nlocal RunService = game:GetService(\"RunService\")\nlocal player = Players.LocalPlayer\n\nlocal function applySpeed(char)\n    local hum = char:WaitForChild(\"Humanoid\")\n    hum.WalkSpeed = 80; hum.UseJumpPower = true; hum.JumpPower = 120\nend\n\nif player.Character then applySpeed(player.Character) end\nplayer.CharacterAdded:Connect(applySpeed)\n\nlocal noclip = false\nUIS.InputBegan:Connect(function(input, processed)\n    if processed then return end\n    if input.KeyCode == Enum.KeyCode.N then noclip = not noclip end\nend)\n\nRunService.Stepped:Connect(function()\n    if noclip and player.Character then\n        for _, p in pairs(player.Character:GetDescendants()) do\n            if p:IsA(\"BasePart\") then p.CanCollide = false end\n        end\n    end\nend)\n\nUIS.JumpRequest:Connect(function()\n    local c = player.Character\n    if c then local h = c:FindFirstChildOfClass(\"Humanoid\"); if h then h:ChangeState(Enum.HumanoidStateType.Jumping) end end\nend)\n\nprint(\"[Crynos] Mega Script loaded!\")\nprint(\"[Crynos] Speed: 80 | Jump: 120 | Press N for Noclip | Inf Jump ON\")",
         "All-in-one combo: speed, jump, noclip, infinite jump"},
        {"Rejoin Server", {"rejoin", "reconnect"},
         "-- Crynos Rejoin Script\n\nlocal TPS = game:GetService(\"TeleportService\")\nlocal Players = game:GetService(\"Players\")\nTPS:Teleport(game.PlaceId, Players.LocalPlayer)\nprint(\"[Crynos] Rejoining server...\")",
         "Rejoins the current server"},
        {"Server Hop", {"server hop", "hop", "switch server", "change server"},
         "-- Crynos Server Hop Script\n\nlocal TPS = game:GetService(\"TeleportService\")\nlocal Http = game:GetService(\"HttpService\")\nlocal Players = game:GetService(\"Players\")\n\nlocal servers = Http:JSONDecode(game:HttpGet(\"https://games.roblox.com/v1/games/\" .. game.PlaceId .. \"/servers/Public?sortOrder=Asc&limit=100\"))\n\nfor _, server in pairs(servers.data or {}) do\n    if server.playing < server.maxPlayers and server.id ~= game.JobId then\n        TPS:TeleportToPlaceInstance(game.PlaceId, server.id, Players.LocalPlayer)\n        break\n    end\nend\nprint(\"[Crynos] Server hop initiated\")",
         "Switches to a different server"},
        {"Character Size", {"size", "tiny", "small", "big", "giant", "resize", "scale"},
         "-- Crynos Character Size Script\n\nlocal Players = game:GetService(\"Players\")\nlocal player = Players.LocalPlayer\nlocal character = player.Character or player.CharacterAdded:Wait()\nlocal humanoid = character:WaitForChild(\"Humanoid\")\nlocal SCALE = 2\n\nlocal function setScale(hum, scale)\n    for _, name in pairs({\"BodyDepthScale\",\"BodyHeightScale\",\"BodyWidthScale\",\"HeadScale\"}) do\n        local s = hum:FindFirstChild(name); if s then s.Value = scale end\n    end\nend\n\nsetScale(humanoid, SCALE)\nplayer.CharacterAdded:Connect(function(char) wait(0.5) setScale(char:WaitForChild(\"Humanoid\"), SCALE) end)\nprint(\"[Crynos] Character scale set to \" .. SCALE .. \"x\")",
         "Changes your character size (tiny to giant)"},
        {"Bring All Players", {"bring", "bring all", "bring players", "pull players"},
         "-- Crynos Bring All Script\n\nlocal Players = game:GetService(\"Players\")\nlocal player = Players.LocalPlayer\nlocal myChar = player.Character\nif not myChar or not myChar:FindFirstChild(\"HumanoidRootPart\") then return end\n\nlocal myPos = myChar.HumanoidRootPart.CFrame\nlocal count = 0\n\nfor _, p in pairs(Players:GetPlayers()) do\n    if p ~= player and p.Character and p.Character:FindFirstChild(\"HumanoidRootPart\") then\n        p.Character.HumanoidRootPart.CFrame = myPos + Vector3.new(math.random(-5,5), 0, math.random(-5,5))\n        count = count + 1\n    end\nend\nprint(\"[Crynos] Brought \" .. count .. \" players to your location\")",
         "Teleports all players to your position"}
    };
}

std::string CrynosAI::generate_response(const std::string& prompt) {
    std::string input_lower = to_lower(prompt);

    if ((input_lower.find("hello") != std::string::npos ||
         input_lower.find("hi") != std::string::npos ||
         input_lower.find("hey") != std::string::npos) && input_lower.size() < 15) {
        return "Hey! I'm Crynos AI - your built-in Lua script generator.\n\n"
               "I can create scripts for:\n"
               "- Speed, Fly, ESP, Noclip, God Mode\n"
               "- Aimbot, Kill Aura, Teleport\n"
               "- Auto Farm, Infinite Jump\n"
               "- GUI creation, Server Hop\n"
               "- And much more!\n\n"
               "Just tell me what you need!";
    }

    if (input_lower == "help" || input_lower == "commands" || input_lower == "list" ||
        input_lower.find("what can") != std::string::npos) {
        return "Here's everything I can generate:\n\n"
               "MOVEMENT: speed, fly, noclip, teleport, click tp\n"
               "COMBAT: god mode, aimbot, kill aura, bring all\n"
               "FARMING: auto farm, infinite jump\n"
               "VISUAL: ESP/wallhack, fullbright\n"
               "UTILITY: anti-afk, rejoin, server hop, chat spam\n"
               "GUI: teleport gui, player list\n"
               "CHARACTER: size change, jump power, low gravity\n"
               "COMBO: all-in-one mega script\n\n"
               "Just describe what you want and I'll generate it!";
    }

    int best_score = 0;
    int best_idx = -1;

    for (int i = 0; i < (int)templates_.size(); i++) {
        int score = match_score(prompt, templates_[i].keywords);
        if (score > best_score) { best_score = score; best_idx = i; }
    }

    if (best_idx >= 0 && best_score >= 3) {
        std::string response = "-- " + templates_[best_idx].description + "\n\n";
        response += templates_[best_idx].code;
        return response;
    }

    return generate_custom(prompt);
}

std::string CrynosAI::generate_custom(const std::string& prompt) const {
    std::string response = "-- Crynos AI Generated Script\n";
    response += "-- Request: " + prompt + "\n\n";
    response += "-- I don't have a specific template for that,\n";
    response += "-- but here's a starter script you can customize:\n\n";
    response += "local Players = game:GetService(\"Players\")\n";
    response += "local player = Players.LocalPlayer\n";
    response += "local character = player.Character or player.CharacterAdded:Wait()\n";
    response += "local humanoid = character:WaitForChild(\"Humanoid\")\n\n";
    response += "-- Add your custom logic here\n";
    response += "print(\"[Crynos] Custom script executed\")\n\n";
    response += "-- Tip: Try asking for specific scripts like:\n";
    response += "-- \"speed script\", \"ESP\", \"fly\", \"god mode\"";
    return response;
}

// AiChat implementation

AiChat::AiChat() {
    ChatMessage welcome;
    welcome.id = generate_id();
    welcome.role = "assistant";
    welcome.content = "Welcome to Crynos AI! Built-in Lua script generator - no API key needed!\n\n"
        "I can create 20+ different scripts. Try:\n"
        "- \"speed script\"\n"
        "- \"fly hack\"\n"
        "- \"ESP wallhack\"\n"
        "- \"god mode\"\n"
        "- \"auto farm\"\n"
        "- \"teleport GUI\"\n"
        "- \"aimbot\"\n"
        "- Type \"help\" for the full list!";
    welcome.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    messages_.push_back(welcome);
}

std::string AiChat::generate_id() const {
    auto now = std::chrono::steady_clock::now();
    return std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count());
}

void AiChat::render(float width, float height,
                     std::function<void(const std::string&)> on_insert_code) {
    ImGui::TextColored(Theme::colors().accent, "Crynos AI");
    ImGui::SameLine();
    ImGui::TextColored(Theme::colors().success, "(Free - No API Key)");
    ImGui::SameLine(width - 80);
    if (ImGui::Button("Clear Chat")) { clear_history(); }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    render_messages(width, height - 80);
    render_input(width, on_insert_code);
}

void AiChat::render_messages(float width, float height) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::colors().console_bg);
    ImGui::BeginChild("ChatMessages", ImVec2(width, height), true);

    for (size_t i = 0; i < messages_.size(); i++) {
        const auto& msg = messages_[i];
        ImGui::PushID((int)i);
        bool is_user = (msg.role == "user");

        if (is_user) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg,
                ImVec4(Theme::colors().accent.x * 0.15f, Theme::colors().accent.y * 0.15f,
                       Theme::colors().accent.z * 0.15f, 0.8f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::colors().surface);
        }

        float msg_height = ImGui::CalcTextSize(msg.content.c_str(), nullptr, false, width - 40).y + 30;
        msg_height = std::max(msg_height, 40.0f);

        ImGui::BeginChild(("msg_" + msg.id).c_str(), ImVec2(width - 16, msg_height), true);

        if (is_user) { ImGui::TextColored(Theme::colors().accent, "You"); }
        else { ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.6f, 1.0f), "Crynos AI"); }
        ImGui::SameLine();
        ImGui::TextColored(Theme::colors().text_dim, "%s", Utils::get_timestamp().c_str());

        ImGui::TextWrapped("%s", msg.content.c_str());

        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::PopID();
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) { ImGui::SetScrollHereY(1.0f); }

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

void AiChat::render_input(float width, std::function<void(const std::string&)> on_insert_code) {
    ImGui::Spacing();

    float input_width = width - 160;
    ImGui::SetNextItemWidth(input_width);
    bool enter = ImGui::InputTextWithHint("##ai_input",
        "Ask Crynos AI to generate a script...",
        input_buf_, sizeof(input_buf_), ImGuiInputTextFlags_EnterReturnsTrue);

    ImGui::SameLine();
    bool can_send = strlen(input_buf_) > 0 && !is_generating_;

    if (can_send) {
        ImGui::PushStyleColor(ImGuiCol_Button, Theme::colors().accent);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
    }

    if ((ImGui::Button("Send", ImVec2(70, 0)) || enter) && can_send) {
        send_message(std::string(input_buf_));
        input_buf_[0] = '\0';
    }

    if (can_send) { ImGui::PopStyleColor(2); }

    ImGui::SameLine();

    if (!messages_.empty()) {
        bool has_code = false;
        std::string last_code;
        for (auto it = messages_.rbegin(); it != messages_.rend(); ++it) {
            if (it->role == "assistant") {
                last_code = extract_code(it->content);
                if (!last_code.empty()) { has_code = true; break; }
                if (it->content.find("local ") != std::string::npos ||
                    it->content.find("game:") != std::string::npos ||
                    it->content.find("print(") != std::string::npos) {
                    last_code = it->content; has_code = true; break;
                }
            }
        }

        if (has_code && on_insert_code) {
            ImGui::PushStyleColor(ImGuiCol_Button, Theme::colors().success);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
            if (ImGui::Button("Insert", ImVec2(70, 0))) { on_insert_code(last_code); }
            ImGui::PopStyleColor(2);
            if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Insert last AI code into editor"); }
        }
    }
}

void AiChat::send_message(const std::string& message) {
    if (message.empty() || is_generating_) return;

    ChatMessage user_msg;
    user_msg.id = generate_id();
    user_msg.role = "user";
    user_msg.content = message;
    user_msg.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    messages_.push_back(user_msg);

    std::string response = ai_engine_.generate_response(message);

    ChatMessage ai_msg;
    ai_msg.id = generate_id();
    ai_msg.role = "assistant";
    ai_msg.content = response;
    ai_msg.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    messages_.push_back(ai_msg);
}

std::string AiChat::extract_code(const std::string& text) const {
    size_t start = text.find("```lua");
    if (start == std::string::npos) start = text.find("```");
    if (start == std::string::npos) return "";
    start = text.find('\n', start);
    if (start == std::string::npos) return "";
    start++;
    size_t end = text.find("```", start);
    if (end == std::string::npos) return "";
    return text.substr(start, end - start);
}

bool AiChat::is_generating() const { return is_generating_; }

void AiChat::clear_history() {
    messages_.clear();
    ChatMessage welcome;
    welcome.id = generate_id();
    welcome.role = "assistant";
    welcome.content = "Chat cleared! Ask me to generate any Lua script - I have 20+ templates built in!";
    welcome.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    messages_.push_back(welcome);
}
