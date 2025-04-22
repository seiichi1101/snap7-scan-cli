#include "snap7.h"
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

enum class ExitCode {
  Ok = 0,
  InvalidArgs = 1,
  ConnectionFailed = 2,
  ReadFailed = 3
};

void hexDump(const void *data, std::size_t length, std::size_t indexWidth) {
  const auto *bytes = static_cast<const std::uint8_t *>(data);

  for (std::size_t i = 0; i < length; ++i) {
    if (i % 10 == 0)
      std::cout << std::dec << std::setfill('0') << std::setw(indexWidth) << i
                << "| ";

    std::cout << std::uppercase << std::setfill('0') << std::setw(2) << std::hex
              << static_cast<int>(bytes[i]) << ' ';

    if ((i + 1) % 5 == 0 && (i + 1) % 10 != 0)
      std::cout << ' ';
    if ((i + 1) % 10 == 0 || i + 1 == length)
      std::cout << "|\n";
  }
  std::cout << std::dec;
}

int main(int argc, char *argv[]) {
  if (argc != 7) {
    std::cerr << "Usage: " << argv[0]
              << " <ip> <rack> <slot> <dbnum> <start> <size>\n";
    return static_cast<int>(ExitCode::InvalidArgs);
  }

  try {
    const std::string ip = argv[1];
    const std::uint32_t rack = std::stoul(argv[2]);
    const std::uint32_t slot = std::stoul(argv[3]);
    const std::uint32_t dbNum = std::stoul(argv[4]);
    const std::uint32_t start = std::stoul(argv[5]);
    const std::size_t bufSize = std::stoul(argv[6]);

    std::vector<std::uint8_t> buffer(bufSize);

    std::cout << "===== Configuration =====\n"
              << "IP Address      : " << ip << '\n'
              << "Rack / Slot     : " << rack << " / " << slot << '\n'
              << "DB Number       : " << dbNum << '\n'
              << "Start Offset    : " << start << '\n'
              << "Read Buffer Size: " << bufSize << "\n\n";

    TS7Client client;
    if (client.ConnectTo(ip.c_str(), rack, slot) != 0) {
      std::cerr << "Failed to connect\n";
      return static_cast<int>(ExitCode::ConnectionFailed);
    }

    if (client.DBRead(dbNum, start, bufSize, buffer.data()) != 0) {
      std::cerr << "DBRead failed: " << CliErrorText(client.LastError())
                << '\n';
      client.Disconnect();
      return static_cast<int>(ExitCode::ReadFailed);
    }

    std::cout << "===== Scan Data =====\n";
    hexDump(buffer.data(), buffer.size(), std::to_string(bufSize).length());
    std::cout << '\n';

    client.Disconnect();
    return static_cast<int>(ExitCode::Ok);
  } catch (const std::exception &ex) {
    std::cerr << "Argument error: " << ex.what() << '\n';
    return static_cast<int>(ExitCode::InvalidArgs);
  }
}
