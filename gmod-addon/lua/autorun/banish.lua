AddCSLuaFile("banish/gui.lua")

if CLIENT then include("banish/gui.lua") end
if SERVER then include("banish/server.lua") end