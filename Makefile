all: Http Socks4 Socks5

Http:
	g++ -Wall -lrt -lpthread -o ProxyHunter ProxyHunter.cpp Task.cpp TaskList.cpp HttpRequest.cpp Config.cpp /usr/local/lib/libevent.a

Socks5:
	g++ -DSOCKS5 -Wall -lrt -lpthread -o Socks5Proxy ProxyHunter.cpp Socks5Task.cpp TaskList.cpp Config.cpp /usr/local/lib/libevent.a

Socks4:
	g++ -DSOCKS4 -Wall -lrt -lpthread -o Socks4Proxy ProxyHunter.cpp Socks4Task.cpp TaskList.cpp Config.cpp /usr/local/lib/libevent.a

clean:
	rm -f ProxyHunter Socks4Proxy Socks5Proxy
