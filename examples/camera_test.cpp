#include <cstdlib>
#include <string>
#include <iostream>



int main(){
	std::string command = "rpicam-vid -t 600000 -o video"; //Uncomment For Flight
	//std::string command = "rpicam-vid -t 2000 -o test"; //Comment For Flight
	std::string filetype = ".mp4";
	std::system("mkdir -p videos");  // This will create the 'videos' directory if it doesn't exist
	for (int i = 0; i < 18; i++){ //Change to 18 before flight
		std::system((command + std::to_string(i) + filetype).c_str());
	}
	return 0;
}

