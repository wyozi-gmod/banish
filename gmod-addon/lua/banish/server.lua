require("banish")

-- only one instance of banishg should exist per Lua session
banishg = banishg or {}
banishg.localRepository = banish.CreateRepository()
banishg.remotes = banishg.remotes or {}

function banish.ServerOn()
	if not banishg.server then
		banishg.server = banish.CreateServer(banishg.localRepository, 8099)
	end
end
function banish.ServerOff()
	if banishg.server then
		banishg.server:Close()
		banishg.server = nil
	end
end

function banish.AddRemote(name, url)
	banishg.remotes[name] = {
		url = url
	}
end

function banish.Pull()
	local function parseData(data)
		local hdr = data:sub(1, 4)
		if hdr ~= "BANI" then
			return false, "Invalid header"
		end

		local vers = data:sub(5, 6)

		return true, util.JSONToTable(data:sub(7))
	end

	for name,remote in pairs(banishg.remotes) do
		print("Pulling from remote ", name)

		local repo = banish.CreateRepository()
		repo:PullFromRemoteURL(remote.url, function()
			repo:GetHeadData(function(data)
				local parsed, pdata = parseData(data)
				if not parsed then
					print("Parsing data from remote ", name, " failed: ", pdata)
					return
				end

				PrintTable(pdata)
			end)
		end)
	end
end

function banish.UpdateLocalData(data)
	local fdata = {
		-- header
		"BANI",

		-- version
		string.char(0x00),
		string.char(0x01),

		-- data
		util.TableToJSON(data)
	}
	fdata = table.concat(fdata, "")

	banishg.localRepository:SetHeadData(fdata)
end

banish.UpdateLocalData {
	{ name = "minge", reason = "proppush", duration = 1000 },
	{ name = "1337haker", reason = "hacking", duration = 909999 }
}

util.AddNetworkString("banish")
net.Receive("banish", function(len, cl)
	if not cl:IsSuperAdmin() then return end
end)

concommand.Add("banish", function(ply, cmd, args)
	if IsValid(ply) and not ply:IsSuperAdmin() then return end

	local act = args[1]
	local op = args[2]

	if act == "server" then
		if op == "on" then
			print("Turning server on")
			banish.ServerOn()
		elseif op == "off" then
			print("Turning server off")

			print("Due to a bug in underlying HTTP library, turning server off is currently disabled.")
			--banish.ServerOff()
		else
			print("Server is " .. (banishg.server and "on" or "off"))
		end
	elseif act == "remote" then
		local subAct = args[2]
		if subAct == "add" then
			print("Adding remote")

			local name, url = args[3], args[4]
			assert(name and url)
			banish.AddRemote(name, url)
		end
	elseif act == "pull" then
		print("Pulling")
		banish.Pull()
	end
end)

function Banish(libname)
	require(libname or "banish")

	local localRepo = banish.CreateRepository()
	localRepo:SetHeadData("ayy lmao")
	local server = banish.CreateServer(localRepo, 8099)

	timer.Simple(1, function()
		local repo = banish.CreateRepository()
		print("pulling")
		repo:PullFromRemoteURL("http://localhost:8099", function()
			print("pull done")
			repo:GetHeadData(function(data)
				print("DATA: ", data)

				server:Close()
			end)
		end)
	end)
end