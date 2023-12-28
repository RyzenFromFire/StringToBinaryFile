#include <iostream>
#include <fstream>
#include <ctype.h>
#include <regex>

// Command format: 
//   rwbin [-h | --help]
//   rwbin [-r | --read] <filename>
//   rwbin [-w | --write] <value> [filename]
// Examples:
//   rwbin -h                   -- shows help page
//   rwbin -r dec.bin           -- reads file and prints contents
//   rwbin -w 255 dec.bin       -- writes decimal value '255' as binary to file 'dec.bin'
//   rwbin 255 dec.bin          -- same as above. if no specifier is provided, decimal is assumed.
//   rwbin 0xFAB0 hex.bin       -- writes hex value 0xFAB0 as binary to file 'hex.bin'
//   rwbin b'01101010 byte.bin  -- writes '01101010' as binary to file 'byte.bin'
//   rwbin h'00FF hex.bin       -- writes hex value 0x0FF as binary to file 'hex.bin'
//   rwbin d'10 dec.bin         -- writes decimal value '10' as binary to file 'dec.bin'

enum DataType {
  NONE,
  DECIMAL,
  HEX,
  BINARY,
  TEXT // TODO: implement ASCII conversion
};

bool isWriteFlag(char* arg) {
  std::string str(arg);
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  if (str == "-w" || str == "--write") return true;
  else return false;
}

bool isReadFlag(char* arg) {
  std::string str(arg);
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  if (str == "-r" || str == "--read") return true;
  else return false;
}

bool isNumber(std::string val) {
  // iterate across value and detect if it is a number.
  std::string::const_iterator it = val.begin();
  while (it != val.end() && std::isdigit(*it)) ++it;

  return (!val.empty() && it == val.end());
}

bool isHexDigit(char c) {
  return  
    toupper(c) == 'A' ||
    toupper(c) == 'B' ||
    toupper(c) == 'C' ||
    toupper(c) == 'D' ||
    toupper(c) == 'E' ||
    toupper(c) == 'F' ||
    std::isdigit(c);
}

DataType getDataType(char* arg) {

  if (strlen(arg) < 1) return NONE;

  // X'<value> format
  if (strlen(arg) > 2 && (arg[0] == 'b' || arg[0] == 'h' || arg[0] == 'd') && arg[1] == '\'') {
    if (arg[0] == 'b') {
      for (int i = 2; i < strlen(arg); i++) {
        if (arg[i] != '0' || arg[i] != '1') return NONE;
      }
      return BINARY;
    } else if (arg[0] == 'd') {
      if (isNumber(arg)) return DECIMAL;
      else return NONE;
    } else if (arg[0] == 'h') {
      for (int i = 2; i < strlen(arg); i++) {
        if (!isHexDigit(arg[i])) return NONE;
      }
      return HEX;
    }
  // 0x<value> (hex) format
  } else if (strlen(arg) > 2 && arg[0] == '0' && tolower(arg[1]) == 'x') {
    for (int i = 2; i < strlen(arg); i++) {
      if (!isHexDigit(arg[i])) return NONE;
    }
    return HEX;   
  // decimal
  } else if (strlen(arg) > 0) {
    for (int i = 0; i < strlen(arg); i++) {
      if (!isdigit(arg[i])) return NONE;
    }
    return DECIMAL;
  }

  return NONE;
}

// arg is assumed to be valid, checking should already be done by getDataType when this is called
std::string getValueString(char* arg) {

  if (strlen(arg) < 1) return "";

  std::string str(arg);

  // X'<value> format
  if (strlen(arg) > 2 && (arg[0] == 'b' || arg[0] == 'h' || arg[0] == 'd') && arg[1] == '\'') {
    return str.substr(2, str.length() - 2);
  // 0x<value> (hex) format
  } else if (strlen(arg) > 2 && arg[0] == '0' && tolower(arg[1]) == 'x') {
    return str.substr(2, str.length() - 2); 
  // decimal
  } else if (strlen(arg) > 0) {
    return str;
  }

  return "";
}

// from https://stackoverflow.com/questions/22746429/c-decimal-to-binary-converting
// n is assumed to be valid, checking should already be done by getDataType when this is called
std::string decimalToBinary(int n) {
  std::string r;
  while (n != 0) {
    r = (n % 2 == 0 ? "0" : "1" ) + r; 
    n /= 2;
  }
  return r;
}

// from https://stackoverflow.com/a/18311086
// char is assumed to be valid, checking should already be done by getDataType when this is called
std::string hexToBinary(char c) {
  switch (toupper(c)) {
    case '0': return "0000";
    case '1': return "0001";
    case '2': return "0010";
    case '3': return "0011";
    case '4': return "0100";
    case '5': return "0101";
    case '6': return "0110";
    case '7': return "0111";
    case '8': return "1000";
    case '9': return "1001";
    case 'A': return "1010";
    case 'B': return "1011";
    case 'C': return "1100";
    case 'D': return "1101";
    case 'E': return "1110";
    case 'F': return "1111";
  }
  return "0000";
}

// modified from https://stackoverflow.com/a/18311086
// str is assumed to be valid, checking should already be done by getDataType when this is called
std::string hexToBinary(std::string hexStr) {
  std::string binStr;

  // if there is an odd number of hex characters, need to zero pad by 4 (1 hex char) so there is an even number of bytes
  if (hexStr.length() % 2 == 1) {
    binStr += "0000";
  }

  // iterate through string and convert each character to binary
  char c = '\0';
  for (unsigned i = 0; i != hexStr.length(); i++) {
    c = hexStr[i];
    binStr += hexToBinary(c);
  }
  return binStr;
}

bool isFileName(std::string arg) {
  // filename regex: ^[^<>:;,?"*|/]+$
  if (std::regex_match(arg, std::regex("^[^<>:;,?\"*|/]+$"))) return true;
  else return false;
}

void throwInputErrorMsg() {
  std::cout << "Incorrect input command" << std::endl;
}

void throwFilenameErrorMsg() {
  std::cout << "Incorrect filename" << std::endl;
}

void throwHelpMsg() {
  std::cout <<
  "Command format: " << std::endl <<
  "  rwbin [-h | --help]" << std::endl <<
  "  rwbin [-r | --read] <filename>" << std::endl <<
  "  rwbin [-w | --write] <value> [filename]" << std::endl <<
  "Examples:" << std::endl <<
  "  rwbin -h                   -- shows help page" << std::endl <<
  "  rwbin -r dec.bin           -- reads file and prints contents" << std::endl <<
  "  rwbin -w 255 dec.bin       -- writes decimal value '255' as binary to file 'dec.bin'" << std::endl <<
  "  rwbin 255 dec.bin          -- same as above. if no specifier is provided, decimal is assumed." << std::endl <<
  "  rwbin 0xFAB0 hex.bin       -- writes hex value 0xFAB0 as binary to file 'hex.bin'" << std::endl <<
  "  rwbin b'01101010 byte.bin  -- writes '01101010' as binary to file 'byte.bin'" << std::endl <<
  "  rwbin h'00FF hex.bin       -- writes hex value 0x0FF as binary to file 'hex.bin'" << std::endl <<
  "  rwbin d'10 dec.bin         -- writes decimal value '10' as binary to file 'dec.bin'" << std::endl;
}

void writeBinaryFile(std::string content, DataType type, std::string filename) {
  // Modified from: https://www.tutorialspoint.com/reading-and-writing-binary-file-in-c-cplusplus
  std::ofstream of(filename, std::ios::out | std::ios::binary);
  if (!of) {
    std::cout << "Cannot open file for writing!" << std::endl;
  }

  if (type == NONE) {
    throwInputErrorMsg();
    return;
  }

  std::cout << "content = " << content << std::endl;
  

  // convert content to binary
  std::string binStr;

  if (type == DECIMAL) {
    binStr = decimalToBinary(stoi(content));
  } else if (type == HEX) {
    binStr = hexToBinary(content); 
  } else if (type == BINARY) {
    binStr = content;
  }

  std::cout << "binStr = " << binStr << std::endl;
  std::cout << "padding = " << 8 - (binStr.length() % 8) << std::endl;

  // if the length of the resulting binary string is not evenly divisible into bytes, zero pad until it is
  if (binStr.length() % 8 != 0) {
    int lim = 8 - (binStr.length() % 8);
    for (int i = 0; i < lim; i++) {
      binStr = "0" + binStr;
    }
  }

  std::cout << "padded binStr = " << binStr << std::endl;

  // at this point, the binary string is evenly divisible into bytes
  // package into bytes and write to file
  for (int i = 0; i < binStr.length(); i += 8) {
    char byte = static_cast<char>(std::stoi(binStr.substr(i, 8), 0, 2));
    of.write((char*) &byte, sizeof(char));
  }

  if (!of.good()) {
    std::cout << "Error occurred at writing time!" << std::endl;
  }
}

void printBinaryFile(std::string filename) {
  // Modified from: https://www.tutorialspoint.com/reading-and-writing-binary-file-in-c-cplusplus
  std::ifstream rf("student.dat", std::ios::out | std::ios::binary);
  if (!rf) {
    std::cout << "Cannot open file!" << std::endl;
  }
}

int main(int argc, char** argv) {
  // debug argc, argv
  std::cout << "argc = " << argc << "  argv = [ ";
  for (int i = 0; i < argc; i++) {
    std::cout << argv[i];
    if (i < argc - 1) std::cout << ", ";
  }
  std::cout << " ]" << std::endl;

  // if no args, throw help msg and return 1
  if (argc < 2) {
    throwInputErrorMsg();
    throwHelpMsg();
    return 1;
  }

  // handle flags first
  if (
    (argc >= 3 && isWriteFlag(argv[1]) && getDataType(argv[2]) != NONE) ||
    (argc >= 2 && getDataType(argv[1]) != NONE)
  ) {
    // write data if passed write flag and convertible value
    // assume write if no flag passed but arg is convertible

    int offset = 0;
    
    if (argc >= 2 && getDataType(argv[1]) != NONE) {
      offset = 1;
    }

    if (argc >= 4 - offset) {
      std::string str(argv[3 - offset]);
      if (isFileName(str)) {
        writeBinaryFile(getValueString(argv[2 - offset]), getDataType(argv[2 - offset]), str);
      } else {
        str += ".bin";
        if (isFileName(str)) {
          writeBinaryFile(getValueString(argv[2 - offset]), getDataType(argv[2 - offset]), str);
        } else {
          throwFilenameErrorMsg();
        }
      }
    }
  } else if (argc >= 3 && isReadFlag(argv[1]) && isFileName(argv[2])) {
    // read file if read flag is passed
    printBinaryFile(argv[2]);
  } else {
    throwInputErrorMsg();
    throwHelpMsg();
    return 1;
  }

  return 0;
}