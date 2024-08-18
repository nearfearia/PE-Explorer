
is_pe = (os.info('isWinPE') == 1)  -- Windows Preinstallation Environment
is_wes = App:HasOption('-wes')         -- Windows Embedded Standard
is_win = App:HasOption('-windows')  -- Normal Windows

-- 'auto', 'ui_systemInfo', 'system', '' or nil
handle_system_property = 'auto'

--[[ add one more '-' to be '---', will enable this function
function do_ocf(lnkfile, realfile) -- handle open containing folder menu
  -- local path = realfile:match('(.+)\\')
  -- App:Run('cmd', '/k echo ' .. path)

  -- totalcmd
  App:Run('X:\\Progra~1\\TotalCommander\\TOTALCMD64.exe', '/O /T /A \"' .. realfile .. '\"')
  -- XYplorer
  App:Run('X:\\Progra~1\\XYplorer\\XYplorer.exe', '/select=\"' .. realfile .. '\"')
end
--]]

function App:onLoad()
  -- App:Run('notepad.exe')
  print('WinXShell.exe loading...')
  print('CommandLine:' .. App.CmdLine)
  print('WINPE:'.. tostring(is_pe), 123, 'test', App)
  print('WES:' .. tostring(is_wes))
end

function App:onDaemon()
end

function App:onShell()
  -- wxsUI('UI_WIFI', 'main.jcfg', '-notrayicon -hidewindow')
  -- wxsUI('UI_Volume', 'main.jcfg', '-notrayicon -hidewindow')
end

-- �������ʹ������Զ����¼�����,
-- �뽫������������Ϊondisplaychanged()��
function ondisplaychanged_sample()
  local cur_res_x = Screen:GetX()
  if last_res_x == cur_res_x then return end
  last_res_x = cur_res_x
  if last_res_x >= 3840 then
    Screen:DPI(150)
  elseif last_res_x >= 1440 then
    Screen:DPI(125)
  elseif last_res_x >= 800 then
    Screen:DPI(100)
  end
end

-- return the resource id for startmenu logo
function startmenu_logoid()
  local map = {
    ["none"] = 0, ["windows"] = 1, ["winpe"] = 2,
    ["custom1"] = 11, ["custom2"] = 12, ["custom3"] = 13,
    ["default"] = 1
  }
  -- use next line for custom (remove "--" and change "none" to what you like)
  -- if true then return map["none"] end
  if is_pe then return map["winpe"] end
  return map["windows"]
end

-- ��������Զ���ʱ���������ʾ��Ϣ,
-- �뽫������������Ϊupdate_clock_text()��
-- �Զ�����ʾʾ��:
--[[
    |  22:00 ������  |
    |   2019-9-14    |
]]
-- FYI:https://www.lua.org/pil/22.1.html
function update_clock_text_sample()
  local wd_name = {'��', 'һ', '��', '��', '��', '��', '��'}
  local now_time = os.time()
  local wd_disname =  ' ����' .. wd_name[os.date('%w', now_time) + 1]
  local clocktext = os.date('%H:%M' .. wd_disname .. '\r\n%Y-%m-%d', now_time)
  app:call('SetVar', 'ClockText', clocktext)
end

function App:onFirstShellRun()
end

function Shell:onClick(ctrl)
  if ctrl == 'startmenu_reboot' then
    return onclick_startmenu_reboot()
  elseif ctrl == 'startmenu_shutdown' then
    return onclick_startmenu_shutdown()
  elseif ctrl == 'startmenu_controlpanel' then
    return onclick_startmenu_controlpanel()
  elseif ctrl == 'tray_clockarea' then
    return onclick_tray_clockarea()
  elseif ctrl == 'tray_clockarea(double)' then
    return onclick_tray_clockarea(true)
  end
  return 1 -- continue shell action
end

function onclick_startmenu_reboot()
  -- restart computer directly
  -- System:Reboot()
  wxsUI('UI_Shutdown', 'full.jcfg')
  return 0
  -- return 1 -- for call system dialog
end

function onclick_startmenu_shutdown()
  -- shutdown computer directly
  -- System::Shutdown()
  wxsUI('UI_Shutdown', 'full.jcfg')
  return 0
  -- return 1 -- for call system dialog
end

function onclick_startmenu_controlpanel()
  if is_wes then
    App:Run('control.exe')
    return 0
  end
  return 1
end

function onclick_tray_clockarea(isdouble)
  if isdouble then
    App:Run('control.exe', 'timedate.cpl')
  else
    wxsUI('UI_Calendar', 'main.jcfg')
  end
  return 0
end


function App:onTimer(tid)
end


