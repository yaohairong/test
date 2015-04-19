module("Tcp", package.seeall)
local STATE_DISCONNECTED = 1
local STATE_CONNECTING = 2
local STATE_CONNECTED = 3
local STATE_LISTENING = 4
local MAX_CONNECT_TIME = 5
local socket = require("socket")
local tcp = nil
local server_ip = "127.0.0.1"
local server_port = 1221
local connect_time = 0
local state = STATE_DISCONNECTED

local connecting_callback = nil
local connected_callback = nil
local connect_failed_callback = nil
local disconnected_callback = nil

local listening_callback = nil
local listen_failed_callback = nil
local listen_closed_callback = nil
local client_connected_callback = nil

local message_callback = nil
local is_server = false
function Sleep(t)
	socket.sleep(t)
end
function IsServer()
	return is_server
end
local function OnMessage(msg)
	--print(IsServer() and "client msg" or "server msg", msg)
	if message_callback then
		message_callback(msg)
	end
end
local function Close()
	if tcp then
		tcp:close()
		tcp = nil
	end
end
local function OnConnecting()
	state = STATE_CONNECTING
	if connecting_callback then
		connecting_callback()
	end
end

local function OnConnected()
	state = STATE_CONNECTED
	if connected_callback then
		connected_callback()
	end
end

local function OnConnectFailed(err)
	state = STATE_DISCONNECTED
	Close()
	if connect_failed_callback then
		connect_failed_callback()
	end
end

local function OnDisconnected()
	state = STATE_DISCONNECTED
	Close()
	if disconnected_callback then
		disconnected_callback()
	end
end

local function OnListenFailed(err)
	state = STATE_DISCONNECTED
	Close()
	if listen_failed_callback then
		listen_failed_callback()
	end
end

local function OnListenClosed()
	state = STATE_DISCONNECTED
	Close()
	if listen_closed_callback then
		listen_closed_callback()
	end
end
local function OnListening()
	state = STATE_LISTENING
	if listening_callback then
		listening_callback()
	end
end

local function OnClientConnected(client_tcp)
	state = STATE_CONNECTED
	Close()
	tcp = client_tcp
	if client_connected_callback then
		client_connected_callback(client_tcp)
	end
end

function SetConnectingCallback(callback)
	connecting_callback = callback
end
function SetConnectedCallback(callback)
	connected_callback = callback
end
function SetConnectFailedCallback(callback)
	connect_failed_callback = callback
end
function SetDisconnectedCallback(callback)
	disconnected_callback = callback
end

function SetListeningCallback(callback)
	listening_callback = callback
end
function SetListenFailedCallback(callback)
	listen_failed_callback = callback
end
function SetListenClosedCallback(callback)
	listen_closed_callback = callback
end
function SetClientConnectedCallback(callback)
	client_connected_callback = callback
end
function SetMessageCallback(callback)
	message_callback = callback
end
function Connect(ip, port)
	server_ip = ip
	server_port = port
	Close()
	is_server = false
	local err = nil
	tcp, err = socket.tcp()
	if err then
		OnConnectFailed(err)	
		return
	end
	tcp:settimeout(0)
	tcp:setoption("tcp-nodelay", true)
	tcp:connect(ip, port)
	connect_time = 0
	OnConnecting()
end
function Listen(port)
	Close()
	is_server = true
	local err = nil
	tcp, err = socket.tcp()
	if not err then
		tcp:settimeout(0)
		tcp:setoption("tcp-nodelay", true)
		tcp:setoption("reuseaddr", true)
		_, err = tcp:bind("*", port)
		if not err then
			_, err = tcp:listen(1)
			if not err then
				local client_tcp, err = tcp:accept()
				if client_tcp then
					OnClientConnected(client_tcp)
				elseif err == "timeout" then
					OnListening()
				end
			end
		end
	end
	if err then
		OnListenFailed(err)
	end
end
function Disconnect()
	state = STATE_DISCONNECTED
	Close()
end

function Send(buf)
	local _, err = tcp:send(buf)
	if err then
		OnDisconnected()
	end
end
function Update(frame_time)
	if state == STATE_LISTENING then
		local rt = socket.select({tcp}, nil, 0)
		if rt[1] then
			local client_tcp, err = tcp:accept()
			if client_tcp then
				OnClientConnected(client_tcp)
			else
				OnListenClosed()
			end
		end
	elseif state == STATE_CONNECTING then
		local _, state = tcp:connect(server_ip, server_port)
		if state == "already connected" then
			OnConnected()
		else
			connect_time = connect_time + frame_time
			if connect_time > MAX_CONNECT_TIME then
				OnConnectFailed("timeout")
			end
		end
	elseif state == STATE_CONNECTED then
		local rt = socket.select({tcp}, nil, 0)
		if rt[1] then
			local s, err, p = rt[1]:receive()
			if err == "closed" then
				OnDisconnected()
			elseif s then
				OnMessage(s)
			elseif p then
				print("receive incomplete message")
				Disconnect()
			end
		end
	end
end
