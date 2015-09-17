#include <commitmanager/ClientSocket.hpp>

#include <crossbow/infinio/Endpoint.hpp>
#include <crossbow/infinio/InfinibandService.hpp>
#include <crossbow/logger.hpp>
#include <crossbow/program_options.hpp>

int main(int argc, const char** argv) {
    crossbow::string commitManagerHost;
    bool help = false;
    crossbow::string logLevel("DEBUG");

    auto opts = crossbow::program_options::create_options(argv[0],
            crossbow::program_options::value<'h'>("help", &help),
            crossbow::program_options::value<'l'>("log-level", &logLevel),
            crossbow::program_options::value<'c'>("commit-manager", &commitManagerHost));

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

    crossbow::logger::logger->config.level = crossbow::logger::logLevelFromString(logLevel);

    crossbow::infinio::InfinibandLimits infinibandLimits;
    infinibandLimits.receiveBufferCount = 128;
    infinibandLimits.sendBufferCount = 128;
    infinibandLimits.bufferLength = 32 * 1024;
    infinibandLimits.sendQueueLength = 128;
    infinibandLimits.maxScatterGather = 1;

    crossbow::infinio::InfinibandService service(infinibandLimits);

    auto processor = service.createProcessor();
    tell::commitmanager::ClientSocket client(service.createSocket(*processor));
    client.connect(crossbow::infinio::Endpoint(crossbow::infinio::Endpoint::ipv4(), commitManagerHost));

    processor->executeFiber([&client] (crossbow::infinio::Fiber& fiber) {
        LOG_INFO("Starting transaction");
        auto startResponse = client.startTransaction(fiber, false);
        if (!startResponse->waitForResult()) {
            auto& ec = startResponse->error();
            LOG_INFO("Error while starting transaction [error = %1% %2%]", ec, ec.message());
            return;
        }
        auto snapshot = startResponse->get();
        LOG_INFO("Started transaction [snapshot = %1%]", *snapshot);


        LOG_INFO("Committing transaction");
        auto commitResponse = client.commitTransaction(fiber, snapshot->version());
        if (!commitResponse->waitForResult()) {
            auto& ec = commitResponse->error();
            LOG_INFO("Error while committing transaction [error = %1% %2%]", ec, ec.message());
            return;
        }
        auto succeeded = commitResponse->get();
        LOG_INFO("Committed transaction [succeeded = %1%]", succeeded);
    });

    service.run();

    return 0;
}
