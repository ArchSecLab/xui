/*
 * Copyright (c) 2008 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "dev/x86/i82094aa.hh"

#include <list>

#include "arch/x86/interrupts.hh"
#include "arch/x86/intmessage.hh"
#include "cpu/base.hh"
#include "debug/I82094AA.hh"
#include "dev/x86/i8259.hh"
#include "mem/packet.hh"
#include "mem/packet_access.hh"
#include "sim/system.hh"

namespace gem5
{

    X86ISA::I82094AA::I82094AA(const Params &p)
        : BasicPioDevice(p, 20), lowestPriorityOffset(0),
          intRequestPort(name() + ".int_request", this, this, p.int_latency)
    {
        // This assumes there's only one I/O APIC in the system and since the apic
        // id is stored in a 8-bit field with 0xff meaning broadcast, the id must
        // be less than 0xff

        assert(p.apic_id < 0xff);
        initialApicId = id = p.apic_id;
        arbId = id;
        regSel = 0;
        RedirTableEntry entry = 0;
        entry.mask = 1;
        for (int i = 0; i < TableSize; i++)
        {
            redirTable[i] = entry;
            pinStates[i] = false;
        }

        for (int i = 0; i < p.port_inputs_connection_count; i++)
            inputs.push_back(new IntSinkPin<I82094AA>(
                csprintf("%s.inputs[%d]", name(), i), i, this));
    }

    void
    X86ISA::I82094AA::init()
    {
        // The io apic must register its address range with its pio port via
        // the piodevice init() function.
        BasicPioDevice::init();

        // If the request port isn't connected, we can't send interrupts anywhere.
        panic_if(!intRequestPort.isConnected(),
                 "Int port not connected to anything!");
    }

    Port &
    X86ISA::I82094AA::getPort(const std::string &if_name, PortID idx)
    {
        if (if_name == "int_requestor")
            return intRequestPort;
        if (if_name == "inputs")
            return *inputs.at(idx);
        else
            return BasicPioDevice::getPort(if_name, idx);
    }

    Tick
    X86ISA::I82094AA::read(PacketPtr pkt)
    {
        assert(pkt->getSize() == 4);
        Addr offset = pkt->getAddr() - pioAddr;
        switch (offset)
        {
        case 0:
            pkt->setLE<uint32_t>(regSel);
            break;
        case 16:
            pkt->setLE<uint32_t>(readReg(regSel));
            break;
        default:
            panic("Illegal read from I/O APIC.\n");
        }
        pkt->makeAtomicResponse();
        return pioDelay;
    }

    Tick
    X86ISA::I82094AA::write(PacketPtr pkt)
    {
        assert(pkt->getSize() == 4);
        Addr offset = pkt->getAddr() - pioAddr;
        switch (offset)
        {
        case 0:
            regSel = pkt->getLE<uint32_t>();
            break;
        case 16:
            writeReg(regSel, pkt->getLE<uint32_t>());
            break;
        default:
            panic("Illegal write to I/O APIC.\n");
        }
        pkt->makeAtomicResponse();
        return pioDelay;
    }

    void
    X86ISA::I82094AA::writeReg(uint8_t offset, uint32_t value)
    {
        if (offset == 0x0)
        {
            id = bits(value, 31, 24);
        }
        else if (offset == 0x1)
        {
            // The IOAPICVER register is read only.
        }
        else if (offset == 0x2)
        {
            arbId = bits(value, 31, 24);
        }
        else if (offset >= 0x10 && offset <= (0x10 + TableSize * 2))
        {
            int index = (offset - 0x10) / 2;
            if (offset % 2)
            {
                redirTable[index].topDW = value;
                redirTable[index].topReserved = 0;
            }
            else
            {
                redirTable[index].bottomDW = value;
                redirTable[index].bottomReserved = 0;
            }
        }
        else
        {
            warn("Access to undefined I/O APIC register %#x.\n", offset);
        }
        DPRINTF(I82094AA,
                "Wrote %#x to I/O APIC register %#x .\n", value, offset);
    }

    uint32_t
    X86ISA::I82094AA::readReg(uint8_t offset)
    {
        uint32_t result = 0;
        if (offset == 0x0)
        {
            result = id << 24;
        }
        else if (offset == 0x1)
        {
            result = ((TableSize - 1) << 16) | APICVersion;
        }
        else if (offset == 0x2)
        {
            result = arbId << 24;
        }
        else if (offset >= 0x10 && offset <= (0x10 + TableSize * 2))
        {
            int index = (offset - 0x10) / 2;
            if (offset % 2)
            {
                result = redirTable[index].topDW;
            }
            else
            {
                result = redirTable[index].bottomDW;
            }
        }
        else
        {
            warn("Access to undefined I/O APIC register %#x.\n", offset);
        }
        DPRINTF(I82094AA,
                "Read %#x from I/O APIC register %#x.\n", result, offset);
        return result;
    }

    void
    X86ISA::I82094AA::requestInterrupt(int line)
    {
        if (line == -1)
        {

            TriggerIntMessage message = 0;

            message.destination = 0xFF;
            message.deliveryMode = delivery_mode::Fixed;
            message.destMode = 0;
            message.level = 0;
            message.trigger = 0;
            message.vector = 37;
            signalInterrupt(message);
            return;
        }
        DPRINTF(I82094AA, "Received interrupt %d.\n", line);
        assert(line < TableSize);
        RedirTableEntry entry = redirTable[line];
        if (entry.mask)
        {
            DPRINTF(I82094AA, "Entry was masked.\n");
            return;
        }

        TriggerIntMessage message = 0;

        message.destination = entry.dest;
        message.deliveryMode = entry.deliveryMode;
        message.destMode = entry.destMode;
        message.level = entry.polarity;
        message.trigger = entry.trigger;

        if (entry.deliveryMode == delivery_mode::ExtInt)
        {
            // We need to ask the I8259 for the vector.
            PacketPtr pkt = buildIntAcknowledgePacket();
            auto on_completion = [this, message](PacketPtr pkt)
            {
                auto msg_copy = message;
                msg_copy.vector = pkt->getLE<uint8_t>();
                signalInterrupt(msg_copy);
                delete pkt;
            };
            intRequestPort.sendMessage(pkt, sys->isTimingMode(),
                                       on_completion);
        }
        else
        {
            message.vector = entry.vector;
            signalInterrupt(message);
        }
    }

    void
    X86ISA::I82094AA::signalInterrupt(TriggerIntMessage message)
    {
        std::list<int> apics;
        int numContexts = sys->threads.size();
        if (message.destMode == 0)
        {
            if (message.deliveryMode == delivery_mode::LowestPriority)
            {
                panic("Lowest priority delivery mode from the "
                      "IO APIC aren't supported in physical "
                      "destination mode.\n");
            }
            if (message.destination == 0xFF)
            {
                for (int i = 0; i < numContexts; i++)
                {
                    apics.push_back(i);
                }
            }
            else
            {
                apics.push_back(message.destination);
            }
        }
        else
        {
            for (int i = 0; i < numContexts; i++)
            {
                BaseInterrupts *base_int = sys->threads[i]->getCpuPtr()->getInterruptController(0);
                auto *localApic = dynamic_cast<Interrupts *>(base_int);
                if ((localApic->readReg(APIC_LOGICAL_DESTINATION) >> 24) &
                    message.destination)
                {
                    apics.push_back(localApic->getInitialApicId());
                }
            }
            if (message.deliveryMode == delivery_mode::LowestPriority &&
                apics.size())
            {
                // The manual seems to suggest that the chipset just does
                // something reasonable for these instead of actually using
                // state from the local APIC. We'll just rotate an offset
                // through the set of APICs selected above.
                uint64_t modOffset = lowestPriorityOffset % apics.size();
                lowestPriorityOffset++;
                auto apicIt = apics.begin();
                while (modOffset--)
                {
                    apicIt++;
                    assert(apicIt != apics.end());
                }
                int selected = *apicIt;
                apics.clear();
                apics.push_back(selected);
            }
        }
        for (auto id : apics)
        {
            PacketPtr pkt = buildIntTriggerPacket(id, message);
            intRequestPort.sendMessage(pkt, sys->isTimingMode());
        }
    }

    void
    X86ISA::I82094AA::raiseInterruptPin(int number)
    {
        assert(number < TableSize);
        if (!pinStates[number])
            requestInterrupt(number);
        pinStates[number] = true;
    }

    void
    X86ISA::I82094AA::lowerInterruptPin(int number)
    {
        assert(number < TableSize);
        pinStates[number] = false;
    }

    void
    X86ISA::I82094AA::serialize(CheckpointOut &cp) const
    {
        uint64_t *redirTableArray = (uint64_t *)redirTable;
        SERIALIZE_SCALAR(regSel);
        SERIALIZE_SCALAR(initialApicId);
        SERIALIZE_SCALAR(id);
        SERIALIZE_SCALAR(arbId);
        SERIALIZE_SCALAR(lowestPriorityOffset);
        SERIALIZE_ARRAY(redirTableArray, TableSize);
        SERIALIZE_ARRAY(pinStates, TableSize);
    }

    void
    X86ISA::I82094AA::unserialize(CheckpointIn &cp)
    {
        uint64_t redirTableArray[TableSize];
        UNSERIALIZE_SCALAR(regSel);
        UNSERIALIZE_SCALAR(initialApicId);
        UNSERIALIZE_SCALAR(id);
        UNSERIALIZE_SCALAR(arbId);
        UNSERIALIZE_SCALAR(lowestPriorityOffset);
        UNSERIALIZE_ARRAY(redirTableArray, TableSize);
        UNSERIALIZE_ARRAY(pinStates, TableSize);
        for (int i = 0; i < TableSize; i++)
        {
            redirTable[i] = (RedirTableEntry)redirTableArray[i];
        }
    }

} // namespace gem5
