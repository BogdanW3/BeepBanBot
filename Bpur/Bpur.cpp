#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#define fs std::filesystem
SOCKET sock = INVALID_SOCKET;

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

		// Create a SOCKET for connecting to server
		sock = socket(ainfo->ai_family, ainfo->ai_socktype, ainfo->ai_protocol);
		if (sock == INVALID_SOCKET)
		{
			printf("socket failed with error: %ld\n", WSAGetLastError());
			return 1;
		}

		// Connect to server.
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

	/*std::string message = "PASS ";
	message += std::getenv("BPUR_PASSWORD");
	send(sock, message.c_str(), message.length(), 0);
	message = "NICK ";
	message += std::getenv("BPUR_USERNAME");
	send(sock, message.c_str(), message.length(), 0);*/

	char* password;
	size_t envsize;
	getenv_s(&envsize, NULL, 0, "BPUR_PASSWORD");
	if (envsize == 0)
	{
		printf("BPUR_PASSWORD not set!\n");
		return 1;
	}
	password = (char*)malloc(envsize * sizeof(char));
	if (!password)
	{
		printf("Failed to allocate memory!\n");
		return 1;
	}
	getenv_s(&envsize, password, envsize, "BPUR_PASSWORD");
	std::string message = "PASS ";
	message += password;
	send(sock, message.c_str(), message.length(), MSG_OOB);

	char* username;
	getenv_s(&envsize, NULL, 0, "BPUR_USERNAME");
	if (envsize == 0)
	{
		printf("BPUR_USERNAME not set!\n");
		return 1;
	}
	username = (char*)malloc(envsize * sizeof(char));
	if (!username)
	{
		printf("Failed to allocate memory!\n");
		return 1;
	}
	getenv_s(&envsize, username, envsize, "BPUR_USERNAME");
	message = "NICK ";
	message += username;
	send(sock, message.c_str(), message.length(), MSG_OOB);

	message = "USER ";
	message += username;
	message += " 8 * :";
	message += username;
	send(sock, message.c_str(), message.length(), MSG_OOB);

	char* channel;
	getenv_s(&envsize, NULL, 0, "BPUR_CHANNEL");
	if (envsize == 0)
	{
		printf("BPUR_CHANNEL not set!\n");
		return 1;
	}
	channel = (char*)malloc(envsize * sizeof(char));
	if (!channel)
	{
		printf("Failed to allocate memory!\n");
		return 1;
	}
	getenv_s(&envsize, channel, envsize, "BPUR_CHANNEL");
	message = "JOIN #";
	message += channel;
	send(sock, message.c_str(), message.length(), MSG_OOB);

	char* buff = new char[1024];
	while (recv(sock, buff, 1024, 0))
		std::cout << &buff[0] << std::endl;



	//fileread code
	if (argv[1] && fs::exists(fs::path(argv[1])))
	{
		std::ifstream list(argv[1]);
		for (std::string line; std::getline(list, line); )
		{
			std::cout << line << std::endl;
		}
	}
	else
		std::cout << "Invalid list file.\n";
	free(password);
	free(username);
	free(channel);
	delete [] buff;
	std::cin.get();
}