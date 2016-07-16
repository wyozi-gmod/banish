concommand.Add("banish", function()
	local fr = vgui.Create("DFrame")
	fr:SetTitle("Banish")

	fr:SetSize(800, 600)
	fr:Center()
	fr:MakePopup()

	net.Receive("banish", function()
		if not IsValid(fr) then return end

		local tbl = net.ReadTable()

	end)
end)