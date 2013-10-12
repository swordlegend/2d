--[[
Author: Joel Van Eenwyk, Ryan Monday
Purpose: Controls the player
--]]

function math.clamp(n, low, high)
	return math.min(math.max(n, low), high)
end

function FireWeapon(self)
	if self.fireDelay <= 0 then
		local default = Vision.hkvVec3(0, 0, 0)
		local layer = 7
		local scale = 0.2
		
		local offset1 = self:GetPoint(171, 97)
		offset1.z = layer
		
		local offset2 = self:GetPoint(84, 97)
		offset2.z = layer
				
		local missileLeft = Game:CreateEntity(default, "Sprite", "", "Missile")
		missileLeft:UpdateProperty("TextureFilename", "Textures/missile.png")
		missileLeft:SetScaling(scale)
		missileLeft:SetCenterPosition(offset1)
		table.insert(self.missiles, missileLeft)
		
		local missileRight = Game:CreateEntity(default, "Sprite", "", "Missile")
		missileRight:SetScaling(scale)
		missileRight:SetCenterPosition(offset2)	
		missileRight:UpdateProperty("TextureFilename", "Textures/missile.png")	
		table.insert(self.missiles, missileRight)
		
		self.fireDelay = 0.1
	else
		self.fireDelay = self.fireDelay - Timer:GetTimeDiff()
	end
end

function UpdateMissiles(self)
	local dt = Timer:GetTimeDiff()
	local toDelete = {}

	for key, missile in ipairs(self.missiles) do
		local pos =	missile:GetPosition()
		pos.y = pos.y - (dt * 500)

		if pos.y < 0 then
			table.insert(toDelete, missile)
		else
			missile:SetPosition(pos)
		end
	end

	for _, missileToDelete in ipairs(toDelete) do
		for index, missile in ipairs(self.missiles) do
			if missile == missileToDelete then
				missile:Remove()
				table.remove(self.missiles, index)
				break
			end
		end
	end
end

function OnSpriteCollision(self, sprite)
	Debug:PrintLine("Collision!")
end

function OnAfterSceneLoaded(self)
	local kDeadzone = {deadzone = 0.1}
 
	self.playerInputMap = Input:CreateMap("InputMap")
 
	self.w, self.h = Screen:GetViewportSize()
	Debug:PrintLine("width: " .. self.w .. " height: " .. self.h)
	
	-- Setup the WASD keyboard playerInputMap
	self.playerInputMap:MapTrigger("KeyLeft", "KEYBOARD", "CT_KB_A")
	self.playerInputMap:MapTrigger("KeyRight", "KEYBOARD", "CT_KB_D")
	self.playerInputMap:MapTrigger("KeyUp", "KEYBOARD", "CT_KB_W")
	self.playerInputMap:MapTrigger("KeyDown", "KEYBOARD", "CT_KB_S")
	self.playerInputMap:MapTrigger("keyFire", "KEYBOARD", "CT_KB_SPACE")

	-- Create a virtual thumbstick then setup playerInputMap for it
	Input:CreateVirtualThumbStick()
	self.playerInputMap:MapTriggerAxis("TouchLeft", "VirtualThumbStick", "CT_PAD_LEFT_THUMB_STICK_LEFT", kDeadzone)
	self.playerInputMap:MapTriggerAxis("TouchRight", "VirtualThumbStick", "CT_PAD_LEFT_THUMB_STICK_RIGHT", kDeadzone)
	self.playerInputMap:MapTriggerAxis("TouchUp", "VirtualThumbStick", "CT_PAD_LEFT_THUMB_STICK_UP", kDeadzone)
	self.playerInputMap:MapTriggerAxis("TouchDown", "VirtualThumbStick", "CT_PAD_LEFT_THUMB_STICK_DOWN", kDeadzone)
	self.playerInputMap:MapTrigger("touchFire", {(self.w*.5), (self.h*.5), self.w, self.h}, "CT_TOUCH_ANY")

	-- Calculate the starting position of the ship
	self.x = self.w * 0.5
	self.y = self.h * 0.8
	self.roll = 0
	self:SetPosition(self.x, self.y, 0) 
	
	self.missiles = {}
	self.fireDelay = 0
end

function OnBeforeSceneUnloaded(self)
	Input:DestroyVirtualThumbStick()
	Input:DestroyMap(self.playerInputMap)
end

function OnThink(self)
	-- Constants based off delta time
	local kTimeDifference = Timer:GetTimeDiff()
	local kMoveSpeed = 300 * kTimeDifference
	local kInvMoveSpeed = kMoveSpeed * -1
	local kRollSpeed = 5 * kTimeDifference
	local kRollRecoverSpeed = 4 * kTimeDifference
	
	-- Get keyboard state
	local keyLeft = self.playerInputMap:GetTrigger("KeyLeft")>0
	local keyRight = self.playerInputMap:GetTrigger("KeyRight")>0
	local keyUp = self.playerInputMap:GetTrigger("KeyUp")>0
	local keyDown = self.playerInputMap:GetTrigger("KeyDown")>0
	local keyFire = self.playerInputMap:GetTrigger("keyFire")>0

	-- Get touch control state
	local touchLeft = self.playerInputMap:GetTrigger("TouchLeft")>0
	local touchRight = self.playerInputMap:GetTrigger("TouchRight")>0
	local touchUp = self.playerInputMap:GetTrigger("TouchUp")>0
	local touchDown = self.playerInputMap:GetTrigger("TouchDown")>0
	local touchFire = self.playerInputMap:GetTrigger("touchFire")>0
	
	local hasMovementY = false
	local hasMovementX = false
	
	if keyUp or touchUp then
		self:IncPosition(0, kInvMoveSpeed, 0)
		hasMovementY = true
	end

	if keyDown or touchDown then
		self:IncPosition(0, kMoveSpeed, 0)
		hasMovementY = true
	end
	
	if keyLeft or touchLeft then
		self:IncPosition(kInvMoveSpeed, 0, 0)
		self.roll = self.roll - kRollSpeed
		hasMovementX = true
	end

	if keyRight or touchRight then
		self:IncPosition(kMoveSpeed, 0, 0)
		self.roll = self.roll + kRollSpeed
		hasMovementX = true
	end
	
	self.roll = math.clamp(self.roll, -1, 1)
	
	-- Adjust the roll back to flat over time if we're not moving
	if not hasMovementX then
		local absoluteRoll = math.abs(self.roll)
		if absoluteRoll > 0 and (not hasMovementX) then
			local rollCorrection = (self.roll / absoluteRoll) * kRollRecoverSpeed
			
			-- Make sure we don't over-correct as that would cause wobbling
			if math.abs(rollCorrection) > absoluteRoll then
				self.roll = 0
			else
				self.roll = self.roll - rollCorrection	
			end
		end
	end
	
	-- Set the current state by which direction we're going
	if self.roll < 0 then
		self:SetState("rollLeft")
	else
		self:SetState("rollRight")
	end
 
	self:SetFramePercent(math.abs(self.roll))
	
	if keyFire or touchFire and touchUp == false then
		FireWeapon(self)
	end
	
	UpdateMissiles(self)
end