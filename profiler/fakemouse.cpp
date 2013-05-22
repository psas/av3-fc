#include <cstdio>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/random_device.hpp>

using namespace std;
namespace ba = boost::asio;
namespace bru = boost::random;
namespace bsl = boost::asio::local;

#define ERROR cerr << "\033[31;1mERROR:\t\t"
#define SUCCESS cerr << "\033[32mSUCCESS:\t"
#define WARN cerr << "\033[33mWARNING:\t"
#define INFO cout << "\033[36mINFO:\t\t"
#define CRITICAL cerr << "\033[5;41;37mCRITICAL:\t"
#define DEBUG cout << "\033[37mDEBUG:\t"
#define ENDL "\033[0m" << endl

const unsigned int TIMEOUT = 0;  // Default timeout.
const unsigned int _ARGC = 3;    // Needed number of arguments.

const short PORT_I = 2; // Port index for argv.
const short ADDR_I = 1; // Address index for argv.
const int MIN_CHARS = 0;
const int MAX_CHARS = 30;
const int MIN_CHAR_RANGE = 32;
const int MAX_CHAR_RANGE = 126;

string gen_buf(){
	bru::uniform_int_distribution<> dist1(MIN_CHARS, MAX_CHARS);
	bru::uniform_int_distribution<> dist2(MIN_CHAR_RANGE, MAX_CHAR_RANGE);

	boost::random::random_device rdev;
	int numChars = dist1(rdev);
	string str;
	for (int i = 0; i < numChars; ++i){
		//DEBUG << "i=" << i << " and numChars=" << numChars << ENDL;
		str = str + (char) dist2(rdev);
	}

	INFO << "Generated \"" << str << "\"" << ENDL;
	return str;
}

int main(int argc, char **argv){

    if (argc != _ARGC){
        ERROR << "You provided " << argc << " arguments. Expected " <<
            _ARGC << "." << ENDL;
        exit(EXIT_FAILURE);
    }

	int sockfd = 0;
	const char *cp = argv[1];

	// Construct the sockaddr_in structure needed for bind()
	in_addr ia;
	char * cp2 = new char[BUFSIZ];
	inet_pton(AF_INET, cp, &(ia.s_addr));
	unsigned short port = htons(atoi(argv[2]));
	const struct sockaddr_in addr_in { AF_INET, port, ia, 0 };

	DEBUG << "const struc sockaddr_in {" << ENDL;
	DEBUG << "\tsin_family=" << AF_INET << "," << ENDL;
	DEBUG << "\tsin_port=" << port << "," << ENDL;
	DEBUG << "\tsin_addr={" << ia.s_addr << "}," << ENDL;
	DEBUG << "\tsin_zero[8]=0," << ENDL;
	DEBUG << "}" << ENDL;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		ERROR << "Opening socket." << strerror(errno) << ENDL;

	// Finaly, bind the socket.
	int res = connect(sockfd, (sockaddr *) &addr_in, sizeof(addr_in));
	if (res == -1)
		ERROR << "Connecting socket to fd " << sockfd << ": "
			<< strerror(errno) << ENDL;

	string word;
	while(1){
		word = gen_buf();
		const char *cstr = word.c_str();
		if (send(sockfd, cstr, sizeof(cstr), MSG_NOSIGNAL) == -1){
			WARN << strerror(errno) << ENDL;
			// If we've recieved a "BROKEN PIPE" error, try to reconnect.
			if (EPIPE == errno || EBADF == errno){
				close(sockfd);
				sockfd = socket(AF_INET, SOCK_STREAM, 0);
				if (connect(sockfd, (sockaddr *) &addr_in,
						sizeof(addr_in)) < 0){
					ERROR << strerror(errno) << ENDL;
					ERROR << "Reconnect failed...trying again in " << TIMEOUT <<
							" seconds..." << ENDL;
				}
				sleep(TIMEOUT);
			}
		}else{
			SUCCESS << "Sending data to socket." << ENDL;
		}
	}
}
