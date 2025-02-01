#include "dev/net/accelerator.hh"
#include "dev/net/i8254xGBe.hh"
#include "dev/x86/pc.hh"
#include "cpu/o3/cpu.hh"
#include <inttypes.h>
#include "sim/sim_exit.hh"
#include <netinet/in.h>
#include "base/trace.hh"
#include "debug/AccelDebug.hh"
#include "accelerator.hh"
namespace gem5
{
    Accelerator *Accelerator::self = nullptr;
    std::ofstream Accelerator::latency_file("accel_latency");
    Accelerator::AcceleratorStats::AcceleratorStats(statistics::Group *parent)
        : statistics::Group(parent, "Accelerator"),
          ADD_STAT(sentPackets, statistics::units::Count::get(), "Number of Generated Packets"),
          ADD_STAT(recvPackets, statistics::units::Count::get(), "Number of Recieved Packets"),
          ADD_STAT(latency, statistics::units::Second::get(), "Distribution of Latency in ms")
    {
        sentPackets.precision(0);
        recvPackets.precision(0);
        latency.init(100);
    }

    Accelerator::Accelerator(const AcceleratorParams &p) : SimObject(p), accelId(p.accel_id), packetSize(p.packet_size),
                                                           lastRxCount(0), lastTxCount(0),
                                                           accelGen(AccelGen::FullTyper(), 0, 0),
                                                           AcceleratorStats(this)
    {
        if (p.mode == "Static")
            accelMode = Mode::Static;
        else if (p.mode == "Normal")
            accelMode = Mode::Normal;
        else if (p.mode == "Half")
            accelMode = Mode::Normal;
        Accelerator::interface = new AccelInt("interface", this);
        Accelerator::self = this;
        if (p.mode == "Normal")
        {
            AccelGen::FullTyper typer = {};
            accelGen = AccelGen(typer, p.vmr, p.max_error);
        }
        else if (p.mode == "Half")
        {
            AccelGen::HalfTyper typer = {};
            double min_value = p.max_error; // alias this name I dont want to create a new variable all the way up to configuration.
            accelGen = AccelGen(typer, p.vmr, min_value);
        }
    }

    Port &Accelerator::getPort(const std::string &if_name, PortID idx)
    {
        return *interface;
    }

    void Accelerator::buildPacket(EthPacketPtr ethpacket, uint64_t sendTick, uint64_t req_type)
    {
        // Build Packet header
        // DSTMAC 6 | SRCMAC 6 | LENGTH 2 | DATA
        uint8_t dst_mac[6] = {0x00, 0x90, 0x00, 0x00, 0x00, 0x01 + accelId}; // Use paired NIC's MAC
        uint8_t src_mac[6] = {0x00, 0x80, 0x00, 0x00, 0x00, 0x01 + accelId};

        uint16_t size = ethpacket->length;

        if (1 != htons(1))
            size = htons(size);

        uint8_t head[MACHeaderSize];
        memcpy(head, dst_mac, 6);
        memcpy(head + 6, src_mac, 6);
        memcpy(head + 12, &size, 2);
        memcpy(ethpacket->data, head, MACHeaderSize);
        memcpy(&(ethpacket->data[MACHeaderSize]), &sendTick, sizeof(uint64_t));
        memcpy(&(ethpacket->data[MACHeaderSize + sizeof(uint64_t)]), &req_type, sizeof(uint64_t));
    }

    void Accelerator::sendPacket(uint64_t sendTick)
    {
        AcceleratorStats.sentPackets++;
        lastTxCount++;
        EthPacketPtr txPacket = std::make_shared<EthPacketData>(packetSize);
        txPacket->length = packetSize;
        buildPacket(txPacket, sendTick);
        interface->sendPacket(txPacket);
    }
    static inline Tick usToTicks(double us)
    {
        return std::round(us * 1e6);
    }
    bool Accelerator::processRxPkt(EthPacketPtr pkt)
    {

        uint64_t sendTick;
        uint64_t packetType;
        memcpy(&sendTick, &(pkt->data[14]), sizeof(uint64_t));
        memcpy(&packetType, &(pkt->data[14 + sizeof(uint64_t)]), sizeof(uint64_t));
        AcceleratorStats.recvPackets++;
        lastRxCount++;
        // std::open(); packets
        EventFunctionWrapper *sendPacketEventPtr = new EventFunctionWrapper([sendTick, this]()
                                                                            { sendPacket(sendTick); }, name(), true);
        // Hack to calculate the mean of distributions
        // stddev = [0.2,2.0,5.0, 10.0,50.0]
        // request_lengths = {1:[2,10,100],10:[20,50,100]}
        // min_errors = [1,10]
        /*double stddevs[] = {0.2, 2.0, 5.0, 10.0, 15.0};
        double request_lengths[2][1] = {{2}, {20}};
        double min_errors[] = {1, 10};
        for (auto stddev : stddevs)
        {
            for (auto min_error : min_errors)
            {
                AccelGen::HalfTyper typer_half = {};
                accelGen = AccelGen(typer_half, stddev, min_error);
                for (auto request_length : request_lengths[(min_error == 1) ? 0 : 1])
                {
                    double total_us = 0;
                    for (int i = 0; i < 3000; i++)
                    {
                        uint64_t next = findNext(request_length);
                        if (next > usToTicks(1000))
                        {
                            next = usToTicks(1000);
                        }
                        total_us += (double)next / 1000000.0;
                    }
                    Accelerator::latency_file << "Stddev:" << stddev << " Request Length:" << request_length << " Min Error:" << min_error << " Total:" << total_us << std::endl;
                }
            }
        }
        fatal("Done");
        */
        uint64_t next = findNext(packetType);
        if (next > usToTicks(1000))
        {
            next = usToTicks(1000);
        }
        schedule(sendPacketEventPtr, curTick() + next);
        return true;
    }

    Tick Accelerator::findNext(uint64_t packetType)
    {
        switch (accelMode)
        {
        case Mode::Static:
            return usToTicks(packetType);
        case Mode::Normal:
            return accelGen.findNext(packetType);
        }
    }
    AccelGen::AccelGen(FullTyper typer, double vmr, double max_error) : vmr(vmr), type(AccelGen::Type::Full), max_error(max_error), generator(0), dist()
    {
        std::cout << "Full Typer" << std::endl;
    }
    AccelGen::AccelGen(HalfTyper typer, double vmr, double min_value) : vmr(vmr), type(AccelGen::Type::Half), min_value(min_value), generator(0), dist()
    {
        std::cout << "Half Typer" << std::endl;
    }
    Tick AccelGen::findNext(uint64_t packet_type)
    {
        switch (type)
        {
        case AccelGen::Type::Full:
            return findNextFull(packet_type);
        case AccelGen::Type::Half:
            return findNextHalf(packet_type);
        }
    }
    Tick AccelGen::findNextFull(uint64_t packet_type)
    {
        if (vmr == 0.0 || vmr == -0.0){
            return usToTicks((double)packet_type);
        }
        double mean = packet_type;
        double variance = vmr * mean;
        double stddev = std::sqrt(variance);
        double beta = max_error / stddev;
        double alpha = -beta;
        double phi_alpha = NormalCDF(alpha);
        double phi_beta = NormalCDF(beta);
        double U = dist(generator);
        double val = phi_alpha + U * (phi_beta - phi_alpha);
        if (val <= 0.0 && val >= -1e-3)
        {
            val = 1e-7;
        }
        if (val >= 1.0 && val <= 1 + 1e-3)
        {
            val = 1.0 - 1e-7;
        }
        if (val < 0.0 || val > 1.0)
        {
            std::cerr << "we are about to error, phi_alpha:" << phi_alpha << " phi_beta:" << phi_beta << std::endl;
        }
        double next = NormalCDFInverse(phi_alpha + U * (phi_beta - phi_alpha)) * stddev + mean;
        return usToTicks(next);
    }
    Tick AccelGen::findNextHalf(uint64_t packet_type)
    {
        if (vmr == 0.0 || vmr == -0.0){
            return usToTicks((double)packet_type);
        }
        static int sample_counter = 0;
        constexpr double strata_multiplier = 1.0 / 300.0;
        static int first_run = 1;
        double mean = packet_type;
        double variance = vmr * mean;
        double stddev = std::sqrt(variance);
        double alpha = (min_value - mean) / stddev;
        if(alpha <= 0.0 && alpha > -1e-4){
            alpha = 0.0;
        }
        double phi_alpha = NormalCDF(alpha);
        double phi_beta = 1;
        double U;
        if (first_run)
        {
            first_run = 0;
            constexpr float factor = 0.3989422804014327;
            double pdf_alpha = factor * std::exp(-0.5f * alpha * alpha);
            double err;
            if (phi_alpha <= 1.0 - 1e-7)
            {
                err = stddev * (pdf_alpha / (1.0 - phi_alpha));
            }
            else
            {
                err = 0.0;
            }
            assert(Accelerator::latency_file.is_open());
            Accelerator::latency_file << std::flush;
            Accelerator::latency_file << "Expected Value:" << mean + err << std::endl;
            Accelerator::latency_file << std::flush;
        }
        if (sample_counter != 299)
        {
            U = (dist(generator) + sample_counter) * strata_multiplier;
            sample_counter++;
        }
        else
        {
            U = dist(generator) * 0.003 + 0.997;
            sample_counter = 0;
        }
        double val = phi_alpha + U * (phi_beta - phi_alpha);
        if (val <= 0.0 && val >= -1e-3)
        {
            val = 1e-7;
        }
        if (val >= 1.0 && val <= 1 + 1e-3)
        {
            val = 1.0 - 1e-7;
        }
        if (val < 0.0 || val > 1.0)
        {
            std::cerr << "we are about to error, phi_alpha:" << phi_alpha << " phi_beta:" << phi_beta << std::endl;
        }
        double next = NormalCDFInverse(phi_alpha + U * (phi_beta - phi_alpha)) * stddev + mean;
        return usToTicks(next);
    }
}
