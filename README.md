#Proxy Hunter

This is HTTP/SOCKS4/SOCKS5 proxy hunter

Before compile this program, you need install libevent.

You can run command "./init.sh" to install libevent into the directory "/usr/local/lib".

Then you can run command "make" to get three binary files: ProxyHunter, Socks4Proxy, Socks5Proxy. The binary file ProxyHunter is used to search HTTP proxies on the internet. Socks4Proxy is used to search SOCKS4 proxies on the internet. Socks5Proxy is used to search SOCKS5 proxies on the internet.

You can change the default configuration in the file "Config.cpp":

unsigned int CONNECTION_HZ = 20;
unsigned int THREADS_NUM = 32;

CONNECTION_HZ is the number of IP/port pairs that the proxy hunter will try to check per second per thread. THREADS_NUM will be the number of worker threads that proxy hunter will start and it is better to set THREADS_NUM to the CPU numbers that your machine has. The total connection HZ should be (CONNECTION_HZ * THREADS_NUM).

You should set CONNECTION_HZ carefully. Small value will cause slow search speed and big value may cause network bandwidth issues.

unsigned int TIMEOUTSECONDS = 60;

TIMEOUTSECONDS will be the network timeout seconds.
 
unsigned short PORTS[] = {80, 8000, 8080};
unsigned int PORTS_NUM = sizeof(PORTS) / sizeof(unsigned short);

PORTS defined all TCP ports that will be checked.

unsigned int MAXRETRY = 2;

unsigned int MINNET = 1;
unsigned int MAXNET = 224;

MINNET and MAXNET defined the minimal (including) network address and maximal (excluding) network address. 1 means 1.0.0.0 (including) and 224 means 224.0.0.0 (excluding). So the default configuration will be used to search net 1.x.x.x - 223.x.x.x

unsigned int MINADDRINDEX = 0;
unsigned int MAXADDRINDEX  = (1 << 24);

MINADDRINDEX and MINADDRINDEX defined the minimal (including) host address and maximal (excluding) host address. The default configuration will be used to search host address x.0.0.0 - x.255.255.255 

For HTTP proxy, there is a file "HttpRequest.cpp":

HttpRequest request[] =
{
   {"g-ecx.images-amazon.com",
    "images/G/01/x-locale/common/transparent-pixel._CB386942464_.gif",
    "image",
    43},

   {"static.licdn.com",
    "scds/common/u/img/pic/pic_profile_strength_mask_90x90_v2.png",
    "image",
    1085},
};

This is a HTTP request array. You can add/change this array for the HTTP destinations that you want to verify.  
"g-ecx.images-amazon.com" and "static.licdn.com" are the destination hostnames;
"images/G/01/x-locale/common/transparent-pixel._CB386942464_.gif" and "scds/common/u/img/pic/pic_profile_strength_mask_90x90_v2.png" are the URL paths;
"image" and "image" are the "Content-Type";
43 and 1085 are the "Content-Length".
