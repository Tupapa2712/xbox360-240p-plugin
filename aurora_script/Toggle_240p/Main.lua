scriptTitle = "Selector 240p CRT (Nativos / Arcade)"
scriptAuthor = "240p Dream"
scriptVersion = 3
scriptDescription = "Configura la resolucion 240p (320x240, 1280x240 o 1920x240) sin activar en Aurora. Dentro del juego nativo/arcade usa LT + RT + L3 + R3 para alternar."
scriptPermissions = { "filesystem", "content" }

local BASE_DIR    = "Hdd1:\\AuroraLaunch\\User\\Scripts\\Utility\\Toggle_240p\\"
local CMD_PATH    = BASE_DIR .. "cmd.txt"
local PRESET_PATH = BASE_DIR .. "preset.txt"
local STATE_PATH  = BASE_DIR .. "state.txt"

local function readTrimmed(path)
    if not FileSystem.FileExists(path) then
        return nil
    end
    local ok, data = pcall(FileSystem.ReadFile, path)
    if not ok or not data then
        return nil
    end
    return tostring(data):gsub("[%s%z]+", "")
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
        return false, "No se pudo escribir cmd.txt: " .. tostring(err)
    end
    if ch == "p" then
        return true, nil
    end
    local cleared = waitForCommandClear(2.0)
    if not cleared then
        return false, "El plugin t240.xex no respondio a tiempo.\nVerifica que este en Hdd:\\launch.ini."
    end
    return true, nil
end

local function presetLabel(code)
    if code == "3" then return "1280x240p (Super-Res)" end
    if code == "4" then return "1920x240p (Super-Res)" end
    return "320x240p (Nativo)"
end

function main()
    local curPreset = readTrimmed(PRESET_PATH) or "1"
    local curMode = readTrimmed(STATE_PATH) or "480i"

    local menu = {
        "Elegir 320x240p (Nativo 15kHz)",
        "Elegir 1280x240p (Super-Resolucion)",
        "Elegir 1920x240p (Super-Resolucion)",
        "Forzar Apagado Seguro de Consola",
        "Restaurar 480i Entrelazado",
    }

    local title = "240p Nativo/Arcade [" .. presetLabel(curPreset) .. " | Actual: " .. curMode .. "]"
    local ret = Script.ShowPopupList(title, "Sin opciones", menu)
    if not ret or ret.Canceled or not ret.Selected then
        return
    end

    local sel = ret.Selected.Value
    if sel == "Elegir 320x240p (Nativo 15kHz)" then
        pcall(FileSystem.WriteFile, PRESET_PATH, "1")
        local ok, err = sendCommand("r")
        if not ok then Script.ShowMessageBox("Error 240p", tostring(err), "OK"); return end
        Script.ShowNotification("Listo: 320x240p. Usa LT+RT+L3+R3 dentro del juego.", 0)
    elseif sel == "Elegir 1280x240p (Super-Resolucion)" then
        pcall(FileSystem.WriteFile, PRESET_PATH, "3")
        local ok, err = sendCommand("r")
        if not ok then Script.ShowMessageBox("Error 240p", tostring(err), "OK"); return end
        Script.ShowNotification("Listo: 1280x240p. Usa LT+RT+L3+R3 dentro del juego.", 0)
    elseif sel == "Elegir 1920x240p (Super-Resolucion)" then
        pcall(FileSystem.WriteFile, PRESET_PATH, "4")
        local ok, err = sendCommand("r")
        if not ok then Script.ShowMessageBox("Error 240p", tostring(err), "OK"); return end
        Script.ShowNotification("Listo: 1920x240p. Usa LT+RT+L3+R3 dentro del juego.", 0)
    elseif sel == "Restaurar 480i Entrelazado" then
        local ok, err = sendCommand("0")
        if not ok then Script.ShowMessageBox("Error 240p", tostring(err), "OK"); return end
        Script.ShowNotification("Salida restaurada a 480i (640x480)", 0)
    elseif sel == "Forzar Apagado Seguro de Consola" then
        sendCommand("p")
        Script.ShowNotification("Restaurando 480i y forzando apagado...", 0)
    end
end
