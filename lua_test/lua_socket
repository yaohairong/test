local socket = require("socket")
local scheduler = require("framework/scheduler")
local HOST_PORT = 8080
local TCP_STATE_DISCONNECTED = 1
local TCP_STATE_CONNECTING = 2
local TCP_STATE_CONNECTED = 3
local TCP_STATE_LISTENING = 4
local MAX_CONNECT_TIME = 5

local MainScene = class("MainScene", function()
	return display.newScene("MainScene")
end)
function MainScene:ctor()
	self.tcp_state = TCP_STATE_DISCONNECTED
	self.is_host = true
end

function MainScene:onEnter()
	self:Connect("192.168.1.3")
	--self:StartServer()
	scheduler.scheduleUpdateGlobal(function(dt)
		if self.tcp_state == TCP_STATE_LISTENING then
			local rt = socket.select({self.tcp}, nil, 0)
			if rt[1] then
				local client_tcp, err = self.tcp:accept()
				if client_tcp then
					self:OnClientConnected(client_tcp)
				else
					print(err)
				end
			end
		elseif self.tcp_state == TCP_STATE_CONNECTING then
			local _, state = self.tcp:connect(self.host_ip, HOST_PORT)
			if state == "already connected" then
				self:OnConnected()
			else
				self.connect_time = self.connect_time + dt
				if self.connect_time > MAX_CONNECT_TIME then
					self:OnConnectFailed("timeout")
				end
			end
		elseif self.tcp_state == TCP_STATE_CONNECTED then
			self.tcp:send("hello\n")
			local rt, _, se = socket.select({self.tcp}, nil, 0)
			if se then
				assert(se == "timeout")
			end
			if rt[1] then
				local s, err, p = rt[1]:receive()
				if err == "closed" then
					if self.is_host then
						self:OnClientDisconnected()
					else
						self:OnDisconnected()
					end
				end
				if s then
					print("s:", s)
				end
				if p then
					assert(#p == 0)
					if #p == 0 then
						self:OnDisconnected()
					end
				end
			end
		end
	end)
end

function MainScene:onExit()
end

function MainScene:Connect(host_ip)
	self.is_host = false
	self.host_ip = host_ip
	local tcp, err = socket.tcp()
	if err then
		self:OnConnectFailed(err)	
		return
	end
	tcp:settimeout(0)
	self.tcp = tcp
	tcp:connect(self.host_ip, HOST_PORT)
	self.connect_time = 0
	self:OnConnecting()
end

function MainScene:StartServer()
	self.is_host = true
	local tcp, err = socket.tcp()
	if not err then
		self.tcp = tcp
		tcp:settimeout(0)
		tcp:setoption("reuseaddr", true)
		_, err = tcp:bind("*", HOST_PORT)
		if not err then
			_, err = tcp:listen(1)
			if not err then
				local client_tcp, err = tcp:accept()
				if client_tcp then
					self:OnClientConnected(client_tcp)
					return
				elseif err == "timeout" then
					self:OnListening()
					return	
				end
			end
		end
	end
	if err then
		self:OnStartServerFailed(err)
	end
end

function MainScene:OnConnecting()
	self.tcp_state = TCP_STATE_CONNECTING
	printf("connecting to server...")
end

function MainScene:OnConnected()
	self.tcp_state = TCP_STATE_CONNECTED
	printf("connect to host: %s success", self.host_ip)
end

function MainScene:OnConnectFailed(err)
	self.tcp_state = TCP_STATE_DISCONNECTED
	printf("cannot connect to host: %s, %s", self.host_ip, err)
	self:Connect(self.host_ip)
end

function MainScene:OnDisconnected()
	self.tcp_state = TCP_STATE_DISCONNECTED
	printf("server disconect")
	self:Connect(self.host_ip)
end

function MainScene:OnStartServerFailed(err)
	self.tcp_state = TCP_STATE_DISCONNECTED
	printf("start server failed: %s", err)
	self:Connect(self.host_ip)
end

function MainScene:OnListening()
	self.tcp_state = TCP_STATE_LISTENING
	printf("start listening...")
end

function MainScene:OnClientConnected(client_tcp)
	self.tcp_state = TCP_STATE_CONNECTED
	self.tcp:close()
	self.tcp = client_tcp
	printf("client connected")
end

function MainScene:OnClientDisconnected()
	self.tcp_state = TCP_STATE_DISCONNECTED
	printf("client disconnected")
	self.tcp:close()
	self:StartServer()
end
return MainScene
