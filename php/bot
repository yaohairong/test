#!/usr/bin/env php
<?php
function socketDie($socket) {
    debug_print_backtrace();
    die(socket_strerror(socket_last_error($socket)));
}

abstract class EventLooper {
    protected $writeEventList = [];

    protected $readEventList = [];

    protected $writeFdList = [];

    protected $readFdList = [];

    public function AddWriteEvent($event) {
        $fd = $event->fd;
        $this->writeFdList[] = $fd;
        $this->writeEventList[(int)$fd] = $event;
    }

    public function RemoveWriteEvent($event) {
        $fd = $event->fd;
        unset($this->writeFdList[array_search($fd, $this->writeFdList)]);
        unset($this->writeEventList[(int)$fd]);
    }

    public function AddReadEvent($event) {
        $fd = $event->fd;
        $this->readFdList[] = $fd;
        $this->readEventList[(int)$fd] = $event;
    }

    public function RemoveReadEvent($event) {
        $fd = $event->fd;
        unset($this->readFdList[array_search($fd, $this->readFdList)]);
        unset($this->readEventList[(int)$fd]);
    }

    public function Loop($sleepTime) {
    }
}

class SelectLooper extends EventLooper {
    public function Loop($sleepTime) {
        $readList = $this->readFdList;
        $writeList = $this->writeFdList;
        $exceptList = $this->writeFdList;

        if (socket_select($readList, $writeList, $exceptList, 0)) {
            foreach ($readList as $fd) {
                $event = $this->readEventList[(int)$fd];
                $event->onRead();
            }

            foreach ($writeList as $fd) {
                $event = $this->writeEventList[(int)$fd];
                $event->onWrite();
            }

            foreach ($exceptList as $fd) {
                $event = $this->writeEventList[(int)$fd];
                $event->onWrite();
            }
        }
        usleep($sleepTime);
    }
}

abstract class Event {
    private $eventLooper;

    public $fd;

    public function __construct($eventLooper) {
        $this->eventLooper = $eventLooper;
    }

    public function onWrite() {
    }

    public function onRead() {
    }

    protected function AddWrite() {
        $this->eventLooper->AddWriteEvent($this);
    }

    protected function RemoveWrite() {
        $this->eventLooper->RemoveWriteEvent($this);
    }

    protected function AddRead() {
        $this->eventLooper->AddReadEvent($this);
    }

    protected function RemoveRead() {
        $this->eventLooper->RemoveReadEvent($this);
    }
}
abstract class TcpConnection extends Event {
    const STATE_DISCONNECTED = 0;

    const STATE_CONNECTING = 1;

    const STATE_CONNECTED = 2;

    private $state = self::STATE_DISCONNECTED;

    private $ip;

    private $port;

    private $socket;

    public function isConnected() {
        return $this->state == self::STATE_CONNECTED;
    }

    public function connect($ip, $port) {
        if ($this->socket != null) {
            socket_close($this->socket);
            $this->socket = null;
        }
        $this->ip = $ip;
        $this->port = $port;

        $socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP) or socketDie();
        socket_set_nonblock($socket) or socketDie($socket);
        socket_set_option($socket, SOL_SOCKET, SO_KEEPALIVE, 1) or socketDie($socket);
        socket_set_option($socket, SOL_TCP, TCP_NODELAY, 1) or socketDie($socket);

        if (!socket_connect($socket, $ip, $port)) {
            $error = socket_last_error($socket);
            if ($error == SOCKET_EINPROGRESS || $error == SOCKET_EWOULDBLOCK) {
                $this->fd = $socket;
                $this->AddWrite();
                $this->state = self::STATE_CONNECTING;
                return true;
            }
            else {
                socket_close($socket);
                return false;
            }
        }
        else {
            $this->fd = $socket;
            $this->AddRead();
            $this->onConnectedSuccess();
            $this->state = self::STATE_CONNECTED;
            return true;
        }
    }

    public function onRead () {
        $bytes = socket_recv($this->fd, $buffer, 8192, 0);
        if ($bytes !== false) {
            if ($bytes > 0) {
                $this->onMessage($buffer);
            }
            else {
                $this->RemoveRead();
                $this->state = self::STATE_DISCONNECTED;
                $this->onDisconnected(0);
                socket_close($this->fd);
            }
        }
        else {
            $this->RemoveRead();
            $this->state = self::STATE_DISCONNECTED;
            $this->onDisconnected(socket_last_error($this->fd));
            socket_close($this->fd);
        }
    }

    public function onWrite() {
        if ($this->state == self::STATE_CONNECTING) {
            $error = (socket_get_option($this->fd,SOL_SOCKET,SO_ERROR));
            if ($error == 0) {
                $this->RemoveWrite();
                $this->AddRead();
                $this->state = self::STATE_CONNECTED;
                $this->onConnectedSuccess();
            }
            else {
                $this->RemoveWrite();
                $this->state = self::STATE_DISCONNECTED;
                $this->onConnectedFailed($error);
                socket_close($this->fd);
            }
        }
    }

    public function onConnectedSuccess() {
    }

    public function onConnectedFailed($error) {
    }

    public function onDisconnected($error) {
    }

    public function onMessage($message) {
    }

    public function sendMessage($message) {
        if ($this->state == self::STATE_CONNECTED) {
            if (socket_write($this->fd, $message) === false) {
                $this->RemoveRead();
                $this->state = self::STATE_DISCONNECTED;
                $this->onDisconnected(socket_last_error($this->fd));
                socket_close($this->fd);
            }
        }
    }
}

class Bot extends TcpConnection {
    public static $activeCount = 0;

    public $name;

    public function __construct($name, $eventLooper) {
        $this->name = $name;
        parent::__construct($eventLooper);
    }

    public function getName() {
        return $this->name;
    }

    public function onConnectedSuccess() {
        echo "$this->name connect to server success\n";
    }

    public function onConnectedFailed($error) {
        echo "$this->name failed to connect to server: ", socket_strerror($error), "\n";
        self::$activeCount--;
    }

    public function onDisconnected($error) {
        if ($error == 0) {
            echo "$this->name disconnected to server: ", "closed by peer", "\n";
        }
        else {
            echo "$this->name disconnected to server: ", socket_strerror($error), "\n";
        }
        self::$activeCount--;
    }

    public function onMessage($message) {
        echo "$this->name recv: $message";
    }
}

function Main() {
    $eventLooper = new SelectLooper();

    $ip = "localhost";
    $port = 7;
    $botList = [];
    for ($i = 0; $i < 10; $i++) {
        $bot = new Bot("bot" . $i, $eventLooper);

        if ($bot->connect($ip, $port)) {
            Bot::$activeCount++;
            echo "$bot->name connecting to $ip:$port ...\n";
            $botList[] = $bot;
        }
    }

    $sendNumer = 1;
    while (Bot::$activeCount > 0) {
        $eventLooper->loop(0);
        foreach($botList as $bot) {
            if ($bot->isConnected()) {
                echo "$bot->name send: $sendNumer\n";
                $bot->sendMessage($sendNumer . "\n");
                $sendNumer++;
            }
        }
    }
}
Main();
?>
