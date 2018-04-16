/* CONNECTION_HZ should be less than 1000
 * The total HZ is (CONNECTION_HZ * THREADS_NUM)
 */
unsigned int CONNECTION_HZ = 20;
unsigned int THREADS_NUM = 32;

unsigned int TIMEOUTSECONDS = 60;

unsigned short PORTS[] = {80, 8000, 8080};
unsigned int PORTS_NUM = sizeof(PORTS) / sizeof(unsigned short); 

unsigned int MAXRETRY = 2;

unsigned int MINNET = 1;
unsigned int MAXNET = 224;

unsigned int MINADDRINDEX = 0;
unsigned int MAXADDRINDEX  = (1 << 24);
