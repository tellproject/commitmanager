#include "ServerConfig.hpp"
#include "ServerSocket.hpp"

#include <crossbow/infinio/InfinibandService.hpp>
#include <crossbow/logger.hpp>
#include <crossbow/program_options.hpp>

#include <iostream>

int main(int argc, const char** argv) {
    tell::commitmanager::ServerConfig serverConfig;
    bool help = false;
    crossbow::string logLevel("DEBUG");

    auto opts = crossbow::program_options::create_options(argv[0],
            crossbow::program_options::value<'h'>("help", &help),
            crossbow::program_options::value<'l'>("log-level", &logLevel),
            crossbow::program_options::value<'p'>("port", &serverConfig.port));

    try {
        crossbow::program_options::parse(opts, argc, argv);
    } catch (crossbow::program_options::argument_not_found e) {
        std::cerr << e.what() << std::endl << std::endl;
        crossbow::program_options::print_help(std::cout, opts);
        return 1;
    }

    if (help) {
        crossbow::program_options::print_help(std::cout, opts);
        return 0;
    }

    crossbow::infinio::InfinibandLimits infinibandLimits;
    infinibandLimits.receiveBufferCount = 128;
    infinibandLimits.sendBufferCount = 128;
    infinibandLimits.bufferLength = 32 * 1024;
    infinibandLimits.sendQueueLength = 128;
    infinibandLimits.maxScatterGather = 1;

    crossbow::logger::logger->config.level = crossbow::logger::logLevelFromString(logLevel);

    LOG_INFO("Starting CommitManager on port %1%", serverConfig.port);

    // Initialize network server
    crossbow::infinio::InfinibandService service(infinibandLimits);
    tell::commitmanager::ServerManager server(service, serverConfig);
    service.run();

    LOG_INFO("Exiting CommitManager server");
    return 0;
}
