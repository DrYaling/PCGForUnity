#include "TCPSocket.h"

#include "Proto/punch.pb.h"
TCPSocket::TCPSocket()
{
	SkyDream::Person person;
	person.set_ip("111");
	person.set_port(1000);
}
