scriptTitle = "Modo 240p CRT (Toggle)"
scriptAuthor = "Antigravity"
scriptVersion = 2
scriptDescription = "Activa o desactiva el modo 240p Progresivo (15.734 kHz) localmente en la consola, sin necesidad de PC."
scriptPermissions = { "filesystem", "content" }

local BASE_DIR = "Hdd1:\\AuroraLaunch\\User\\Scripts\\Utility\\Toggle_240p\\"
local CMD_PATH = BASE_DIR .. "cmd.txt"
local STATE_PATH = BASE_DIR .. "state.txt"

local function readState()
    if not FileSystem.FileExists(STATE_PATH) then
        return nil
    end
    local ok, data = pcall(FileSystem.ReadFile, STATE_PATH)
    if not ok or not data then
        return nil
    end
    local s = tostring(data):gsub("[%s%z]+", "")
    if s == "240p" or s == "480i" then
        return s
    end
    return nil
end

local function waitForCommandClear(maxSeconds)
    if os and os.clock then
        local start = os.clock()
        while FileSystem.FileExists(CMD_PATH) do
            if (os.clock() - start) >= maxSeconds then
                return false
            end
        end
        return true
    end
    -- Fallback if os.clock is sandboxed: bounded FileExists checks
    for i = 1, 2000 do
        if not FileSystem.FileExists(CMD_PATH) then
            return true
        end
    end
    return not FileSystem.FileExists(CMD_PATH)
end

local function sendCommand(ch)
    local ok, err = pcall(FileSystem.WriteFile, CMD_PATH, ch)
    if not ok then
        return false, nil, "No se pudo escribir cmd.txt: " .. tostring(err)
    end
    local cleared = waitForCommandClear(2.0)
    if not cleared then
        return false, nil, "El plugin t240.xex no respondió a tiempo.\nVerifica que esté cargado en launch.ini."
    end
    local newState = readState()
    return true, newState, nil
end

function main()
    local currentLabel = readState()
    if not currentLabel then
        -- Ask plugin to refresh state.txt once
        local ok, st, err = sendCommand("s")
        if ok and st then
            currentLabel = st
        else
            Script.ShowMessageBox(
                "Modo 240p CRT",
                err or "t240.xex no está activo en memoria.\nAsegúrate de tenerlo en Hdd:\\launch.ini y reiniciar.",
                "OK"
            )
            return
        end
    end

    local targetAction = (currentLabel == "240p") and "Desactivar 240p (volver a 480i)" or "Activar 240p Progresivo"
    local menu = {
        targetAction,
        "Forzar 240p Progresivo",
        "Restaurar 480i Entrelazado",
        "Ver estado actual (" .. currentLabel .. ")",
    }
    local actions = {
        [targetAction] = "t",
        ["Forzar 240p Progresivo"] = "1",
        ["Restaurar 480i Entrelazado"] = "0",
        ["Ver estado actual (" .. currentLabel .. ")"] = "s",
    }

    local ret = Script.ShowPopupList("Modo 240p CRT [" .. currentLabel .. "]", "Sin opciones", menu)
    if not ret or ret.Canceled or not ret.Selected then
        return
    end

    local cmd = actions[ret.Selected.Value]
    if not cmd then
        return
    end

    local ok, mode, err = sendCommand(cmd)
    if not ok then
        Script.ShowMessageBox("Error Modo 240p", tostring(err), "OK")
        return
    end

    Script.ShowNotification("Modo actual: " .. tostring(mode or "listo"), 0)
end
