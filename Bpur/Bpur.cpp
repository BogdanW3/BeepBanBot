#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
namespace fs = std::filesystem;
SOCKET sock = INVALID_SOCKET;

char* getStringVar(const char* name)
{
	size_t varsize;
	char *var;
	getenv_s(&varsize, NULL, 0, name);
	if (varsize == 0)
	{
		throw std::string(name) + " not set!\n";
	}
	var = (char*)malloc(varsize * sizeof(char));
	if (!var)
	{
		throw "Failed to allocate memory for " + std::string(name) + "\n";
	}
	getenv_s(&varsize, var, varsize, name);
	return var;
}

int main(int argc, char** argv)
{
	struct addrinfo *ainfo = NULL, hints; int temp = 0;
	WSADATA data;
	if (WSAStartup(MAKEWORD(2, 2), &data))
	{
		std::cout << "Socket failure.\n";
		return 1;
	}
	std::cout << "Socket started\n";
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	temp = getaddrinfo("irc.chat.twitch.tv", "6667", &hints, &ainfo);
	if (temp != 0)
	{
		printf("getaddrinfo failed: %d\n", temp);
		WSACleanup();
		return 1;
	}
	for (ainfo; ainfo != NULL; ainfo = ainfo->ai_next)
	{
		sock = socket(ainfo->ai_family, ainfo->ai_socktype, ainfo->ai_protocol);
		if (sock == INVALID_SOCKET)
		{
			printf("socket failed with error: %ld\n", WSAGetLastError());
			return 1;
		}

		temp = connect(sock, ainfo->ai_addr, (int)ainfo->ai_addrlen);
		if (temp == SOCKET_ERROR)
		{
			closesocket(sock);
			sock = INVALID_SOCKET;
			continue;
		}
		break;
	}
	freeaddrinfo(ainfo);
	if (sock == INVALID_SOCKET)
	{
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	char* password;
	char* username;
	char* channel;
	try
	{
		password = getStringVar("BPUR_PASSWORD");
		username = getStringVar("BPUR_USERNAME");
		channel  = getStringVar("BPUR_CHANNEL");
	}
	catch (std::string e)
	{
		std::cout << e;
		return 1;
	}
	std::string message = "PASS ";
	message += password;
	message += "\r\n";
	send(sock, message.c_str(), (int)message.length(), 0);

	message = "NICK ";
	message += username;
	message += "\r\n";
	send(sock, message.c_str(), (int)message.length(), 0);

	message = "JOIN #";
	message += channel;
	message += "\r\n";
	send(sock, message.c_str(), (int)message.length(), 0);

	//banlist code
	if (argv[1] && fs::exists(fs::path(argv[1])))
	{
		std::ifstream list(argv[1]);
		for (std::string line; std::getline(list, line); )
		{
			message = "/ban " + line + "\r\n";
			message = std::string(":") + username + std::string("!") + username + std::string("@") + username +
				std::string(".tmi.twitch.tv PRIVMSG #") + channel + std::string(" :") + message;
			send(sock, message.c_str(), (int)message.length(), 0);
			Sleep(340);
		}
	}
	else
		std::cout << "Invalid list file.\n";

	free(password);
	free(username);
	free(channel);
	std::cout << "beep!\n";
	std::cin.get();
}