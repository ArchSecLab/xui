/*
 * Copyright (c) 2012-2013 ARM Limited
 * All rights reserved
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Copyright (c) 2008 The Hewlett-Packard Development Company
 * All rights reserved.
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
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

#include "arch/x86/interrupts.hh"

#include <list>
#include <memory>

#include "arch/x86/intmessage.hh"
#include "arch/x86/regs/apic.hh"
#include "arch/x86/regs/misc.hh"
#include "cpu/base.hh"
#include "debug/LocalApic.hh"
#include "debug/UserInterrupt.hh"
#include "dev/x86/i82094aa.hh"
#include "dev/x86/pc.hh"
#include "dev/x86/south_bridge.hh"
#include "mem/packet_access.hh"
#include "sim/full_system.hh"
#include "sim/system.hh"
#include "cpu/o3/cpu.hh"
#include "dev/net/load_generator.hh"

namespace gem5
{

  int divideFromConf(uint32_t conf)
  {
    // This figures out what division we want from the division configuration
    // register in the local APIC. The encoding is a little odd but it can
    // be deciphered fairly easily.
    int shift = ((conf & 0x8) >> 1) | (conf & 0x3);
    shift = (shift + 1) % 8;
    return 1 << shift;
  }

  namespace X86ISA
  {

    ApicRegIndex
    decodeAddr(Addr paddr)
    {
      ApicRegIndex regNum;
      paddr &= ~mask(3);
      switch (paddr)
      {
      case 0x20:
        regNum = APIC_ID;
        break;
      case 0x30:
        regNum = APIC_VERSION;
        break;
      case 0x80:
        regNum = APIC_TASK_PRIORITY;
        break;
      case 0x90:
        regNum = APIC_ARBITRATION_PRIORITY;
        break;
      case 0xA0:
        regNum = APIC_PROCESSOR_PRIORITY;
        break;
      case 0xB0:
        regNum = APIC_EOI;
        break;
      case 0xD0:
        regNum = APIC_LOGICAL_DESTINATION;
        break;
      case 0xE0:
        regNum = APIC_DESTINATION_FORMAT;
        break;
      case 0xF0:
        regNum = APIC_SPURIOUS_INTERRUPT_VECTOR;
        break;
      case 0x100:
      case 0x110:
      case 0x120:
      case 0x130:
      case 0x140:
      case 0x150:
      case 0x160:
      case 0x170:
        regNum = APIC_IN_SERVICE((paddr - 0x100) / 0x10);
        break;
      case 0x180:
      case 0x190:
      case 0x1A0:
      case 0x1B0:
      case 0x1C0:
      case 0x1D0:
      case 0x1E0:
      case 0x1F0:
        regNum = APIC_TRIGGER_MODE((paddr - 0x180) / 0x10);
        break;
      case 0x200:
      case 0x210:
      case 0x220:
      case 0x230:
      case 0x240:
      case 0x250:
      case 0x260:
      case 0x270:
        regNum = APIC_INTERRUPT_REQUEST((paddr - 0x200) / 0x10);
        break;
      case 0x280:
        regNum = APIC_ERROR_STATUS;
        break;
      case 0x300:
        regNum = APIC_INTERRUPT_COMMAND_LOW;
        break;
      case 0x310:
        regNum = APIC_INTERRUPT_COMMAND_HIGH;
        break;
      case 0x320:
        regNum = APIC_LVT_TIMER;
        break;
      case 0x330:
        regNum = APIC_LVT_THERMAL_SENSOR;
        break;
      case 0x340:
        regNum = APIC_LVT_PERFORMANCE_MONITORING_COUNTERS;
        break;
      case 0x350:
        regNum = APIC_LVT_LINT0;
        break;
      case 0x360:
        regNum = APIC_LVT_LINT1;
        break;
      case 0x370:
        regNum = APIC_LVT_ERROR;
        break;
      case 0x380:
        regNum = APIC_INITIAL_COUNT;
        break;
      case 0x390:
        regNum = APIC_CURRENT_COUNT;
        break;
      case 0x3E0:
        regNum = APIC_DIVIDE_CONFIGURATION;
        break;
      default:
        // A reserved register field.
        // panic("Accessed reserved register field %#x.\n", paddr);
        break;
      }
      return regNum;
    }
  }

  Tick
  X86ISA::Interrupts::read(PacketPtr pkt)
  {
    Addr offset = pkt->getAddr() - pioAddr;
    // Make sure we're at least only accessing one register.
    if ((offset & ~mask(3)) != ((offset + pkt->getSize()) & ~mask(3)))
      panic("Accessed more than one register at a time in the APIC!\n");
    ApicRegIndex reg = decodeAddr(offset);
    uint32_t val = htole(readReg(reg));
    DPRINTF(LocalApic,
            "Reading Local APIC register %d at offset %#x as %#x.\n",
            reg, offset, val);
    pkt->setData(((uint8_t *)&val) + (offset & mask(3)));
    pkt->makeAtomicResponse();
    return pioDelay;
  }

  Tick
  X86ISA::Interrupts::write(PacketPtr pkt)
  {
    Addr offset = pkt->getAddr() - pioAddr;
    pkt->tick = curTick();

    // assert(pkt->cpu);
    //   Make sure we're at least only accessing one register.
    if ((offset & ~mask(3)) != ((offset + pkt->getSize()) & ~mask(3)))
      panic("Accessed more than one register at a time in the APIC!\n");
    ApicRegIndex reg = decodeAddr(offset);
    uint32_t val = regs[reg];
    pkt->writeData(((uint8_t *)&val) + (offset & mask(3)));
    DPRINTF(LocalApic,
            "Writing Local APIC register %d at offset %#x as %#x.\n",
            reg, offset, letoh(val));
    setReg(reg, letoh(val), pkt->tick);
    pkt->makeAtomicResponse();
    return pioDelay;
  }
  void
  X86ISA::Interrupts::requestInterrupt(uint8_t vector,
                                       uint8_t deliveryMode, bool level)
  {
    /*
     * Fixed and lowest-priority delivery mode interrupts are handled
     * using the IRR/ISR registers, checking against the TPR, etc.
     * The SMI, NMI, ExtInt, INIT, etc interrupts go straight through.
     */
    if (deliveryMode == delivery_mode::Fixed ||
        deliveryMode == delivery_mode::LowestPriority)
    {
      DPRINTF(LocalApic, "Interrupt is an %s.\n",
              delivery_mode::names[deliveryMode]);
      // Queue up the interrupt in the IRR.
      DPRINTF(LocalApic, "IRRV: %d, new vector: %d\n", IRRV, vector);
      if (!tc->getCpuPtr()->isKvm)
      {
        UintrMisc uintrMisc = tc->readMiscRegNoEffect(misc_reg::UintrMisc);
        UintrTimerStatus uintrTimerStatus = tc->readMiscRegNoEffect(misc_reg::UintrTimerStatus);

        UintrPciDisable uintrPciDisable = tc->readMiscRegNoEffect(misc_reg::UintrPciDisable);
        if (vector == 37)
        {
          if (auxPending == 0)
          {
            userPciTimeStart = curTick();
          }
          if (!uintrPciDisable)
          {
            auxPending++;
            if (!tc->getCpuPtr()->isKvm && LoadGenerator::switched && auxPending)
            {
              if (!pendingEvent.scheduled())
              {
                schedule(pendingEvent, curTick() + tc->getCpuPtr()->cyclesToTicks(Cycles(1)));
              }
            }
            if (FullSystem)
              tc->getCpuPtr()->wakeup(0);
            return;
          }
          auxPending++;
          if (FullSystem)
            tc->getCpuPtr()->wakeup(0);
          return;
        }
        if (vector == uintrMisc.uinv || vector == uintrTimerStatus.uinv)
        {
          DPRINTF(UserInterrupt, "IRRVUSER set %d", vector);
          IRRVUSER = vector;
          if (FullSystem)
            tc->getCpuPtr()->wakeup(0);
          return;
        }
      }
      else if (vector == 37 || vector == 36)
      {
        return;
      }
      if (vector > IRRV)
      {
        DPRINTF(LocalApic, "IRRV is being overwritten\n");
        IRRV = vector;
      }
      if (!getRegArrayBit(APIC_INTERRUPT_REQUEST_BASE, vector))
      {
        setRegArrayBit(APIC_INTERRUPT_REQUEST_BASE, vector);
        if (level)
        {
          setRegArrayBit(APIC_TRIGGER_MODE_BASE, vector);
        }
        else
        {
          clearRegArrayBit(APIC_TRIGGER_MODE_BASE, vector);
        }
      }
      else if (vector == 33)
      {
        printInterruptCount++;
      }
    }
    else if (!delivery_mode::isReserved(deliveryMode))
    {
      DPRINTF(LocalApic, "Interrupt is an %s.\n",
              delivery_mode::names[deliveryMode]);
      if (deliveryMode == delivery_mode::SMI && !pendingSmi)
      {
        pendingUnmaskableInt = pendingSmi = true;
        smiVector = vector;
      }
      else if (deliveryMode == delivery_mode::NMI && !pendingNmi)
      {
        pendingUnmaskableInt = pendingNmi = true;
        nmiVector = vector;
      }
      else if (deliveryMode == delivery_mode::ExtInt && !pendingExtInt)
      {
        pendingExtInt = true;
        extIntVector = vector;
      }
      else if (deliveryMode == delivery_mode::INIT && !pendingInit)
      {
        pendingUnmaskableInt = pendingInit = true;
        initVector = vector;
      }
      else if (deliveryMode == delivery_mode::SIPI &&
               !pendingStartup && !startedUp)
      {
        pendingUnmaskableInt = pendingStartup = true;
        startupVector = vector;
      }
    }
    if (FullSystem)
      tc->getCpuPtr()->wakeup(0);
  }

  void
  X86ISA::Interrupts::raiseInterruptPin(int number)
  {
    panic_if(number < 0 || number > 1,
             "Asked to raise unrecognized int pin %d.", number);
    DPRINTF(LocalApic, "Raised wired interrupt pin LINT%d.\n", number);

    const LVTEntry entry =
        regs[(number == 0) ? APIC_LVT_LINT0 : APIC_LVT_LINT1];

    if (entry.masked)
    {
      DPRINTF(LocalApic, "The interrupt was masked.\n");
      return;
    }

    PacketPtr pkt = buildIntAcknowledgePacket();
    auto on_completion = [this, dm = entry.deliveryMode, trigger = entry.trigger](
                             PacketPtr pkt)
    {
      requestInterrupt(pkt->getLE<uint8_t>(), dm, trigger);
      delete pkt;
    };
    intRequestPort.sendMessage(pkt, sys->isTimingMode(), on_completion);
  }

  void
  X86ISA::Interrupts::lowerInterruptPin(int number)
  {
    panic_if(number < 0 || number > 1,
             "Asked to lower unrecognized int pin %d.", number);
    DPRINTF(LocalApic, "Lowered wired interrupt pin LINT%d.\n", number);
  }

  void
  X86ISA::Interrupts::setThreadContext(ThreadContext *_tc)
  {
    assert(_tc);
    panic_if(tc != NULL && tc->cpuId() != _tc->cpuId(),
             "Local APICs can't be moved between CPUs with different IDs.");

    BaseInterrupts::setThreadContext(_tc);

    // Update APIC ID to consider SMT threads
    initialApicId = tc->contextId();
    regs[APIC_ID] = (initialApicId << 24);
    pioAddr = x86LocalAPICAddress(initialApicId, 0);
  }

  void
  X86ISA::Interrupts::init()
  {
    panic_if(!intRequestPort.isConnected(),
             "Int port not connected to anything!");
    panic_if(!pioPort.isConnected(),
             "Pio port of %s not connected to anything!", name());

    intResponsePort.sendRangeChange();
    pioPort.sendRangeChange();
  }

  Tick
  X86ISA::Interrupts::recvMessage(PacketPtr pkt)
  {
    Addr offset = pkt->getAddr() - x86InterruptAddress(initialApicId, 0);
    assert(pkt->cmd == MemCmd::WriteReq);
    switch (offset)
    {
    case 0:
    {
      TriggerIntMessage message = pkt->getRaw<TriggerIntMessage>();
      DPRINTF(LocalApic,
              "Got Trigger Interrupt message with vector %#x.\n",
              message.vector);
      UintrMisc uintrMisc = tc->readMiscRegNoEffect(misc_reg::UintrMisc);
      if (message.vector == uintrMisc.uinv && message.vector)
      {
        // reinterpret_cast<o3::CPU *>(pkt->cpu)->sendUipiSent();
        reinterpret_cast<o3::CPU *>(tc->getCpuPtr())->sendUipiRegister(pkt->tick);
      }
      requestInterrupt(message.vector,
                       message.deliveryMode, message.trigger);
    }
    break;
    default:
      panic("Local apic got unknown interrupt message at offset %#x.\n",
            offset);
      break;
    }
    pkt->makeAtomicResponse();
    return pioDelay;
  }

  void
  X86ISA::Interrupts::completeIPI(PacketPtr pkt)
  {
    if (--pendingIPIs == 0)
    {
      InterruptCommandRegLow low = regs[APIC_INTERRUPT_COMMAND_LOW];
      // Record that the ICR is now idle.
      low.deliveryStatus = 0;
      regs[APIC_INTERRUPT_COMMAND_LOW] = low;
    }
    DPRINTF(LocalApic, "ICR is now idle.\n");
    delete pkt;
  }

  AddrRangeList
  X86ISA::Interrupts::getAddrRanges() const
  {
    assert(tc);
    AddrRangeList ranges;
    ranges.push_back(RangeSize(pioAddr, PageBytes));
    return ranges;
  }

  AddrRangeList
  X86ISA::Interrupts::getIntAddrRange() const
  {
    AddrRangeList ranges;
    ranges.push_back(RangeEx(x86InterruptAddress(initialApicId, 0),
                             x86InterruptAddress(initialApicId, 0) +
                                 PhysAddrAPICRangeSize));
    return ranges;
  }

  uint32_t
  X86ISA::Interrupts::readReg(ApicRegIndex reg)
  {
    if (reg >= APIC_TRIGGER_MODE(0) &&
        reg <= APIC_TRIGGER_MODE(15))
    {
      panic("Local APIC Trigger Mode registers are unimplemented.\n");
    }
    switch (reg)
    {
    case APIC_ARBITRATION_PRIORITY:
      panic("Local APIC Arbitration Priority register unimplemented.\n");
      break;
    case APIC_PROCESSOR_PRIORITY:
      panic("Local APIC Processor Priority register unimplemented.\n");
      break;
    case APIC_ERROR_STATUS:
      regs[APIC_INTERNAL_STATE] &= ~0x1ULL;
      break;
    case APIC_CURRENT_COUNT:
    {
      if (apicTimerEvent.scheduled())
      {
        // Compute how many m5 ticks happen per count.
        uint64_t ticksPerCount = clockPeriod() *
                                 divideFromConf(regs[APIC_DIVIDE_CONFIGURATION]);
        // Compute how many m5 ticks are left.
        uint64_t val = apicTimerEvent.when() - curTick();
        // Turn that into a count.
        val = (val + ticksPerCount - 1) / ticksPerCount;
        return val;
      }
      else
      {
        return 0;
      }
    }
    default:
      break;
    }
    return regs[reg];
  }

  void
  X86ISA::Interrupts::setReg(ApicRegIndex reg, uint32_t val, Tick tick)
  {
    uint32_t newVal = val;
    if (reg >= APIC_IN_SERVICE(0) &&
        reg <= APIC_IN_SERVICE(15))
    {
      panic("Local APIC In-Service registers are unimplemented.\n");
    }
    if (reg >= APIC_TRIGGER_MODE(0) &&
        reg <= APIC_TRIGGER_MODE(15))
    {
      panic("Local APIC Trigger Mode registers are unimplemented.\n");
    }
    if (reg >= APIC_INTERRUPT_REQUEST(0) &&
        reg <= APIC_INTERRUPT_REQUEST(15))
    {
      panic("Local APIC Interrupt Request registers "
            "are unimplemented.\n");
    }
    switch (reg)
    {
    case APIC_ID:
      newVal = val & 0xFF;
      break;
    case APIC_VERSION:
      // The Local APIC Version register is read only.
      return;
    case APIC_TASK_PRIORITY:
      newVal = val & 0xFF;
      break;
    case APIC_ARBITRATION_PRIORITY:
      panic("Local APIC Arbitration Priority register unimplemented.\n");
      break;
    case APIC_PROCESSOR_PRIORITY:
      panic("Local APIC Processor Priority register unimplemented.\n");
      break;
    case APIC_EOI:
      // Remove the interrupt that just completed from the local apic state.
      clearRegArrayBit(APIC_IN_SERVICE_BASE, ISRV);
      updateISRV();
      return;
    case APIC_LOGICAL_DESTINATION:
      newVal = val & 0xFF000000;
      break;
    case APIC_DESTINATION_FORMAT:
      newVal = val | 0x0FFFFFFF;
      break;
    case APIC_SPURIOUS_INTERRUPT_VECTOR:
      regs[APIC_INTERNAL_STATE] &= ~(1ULL << 1);
      regs[APIC_INTERNAL_STATE] |= val & (1 << 8);
      if (val & (1 << 9))
        warn("Focus processor checking not implemented.\n");
      break;
    case APIC_ERROR_STATUS:
    {
      if (regs[APIC_INTERNAL_STATE] & 0x1)
      {
        regs[APIC_INTERNAL_STATE] &= ~0x1ULL;
        newVal = 0;
      }
      else
      {
        regs[APIC_INTERNAL_STATE] |= 0x1ULL;
        return;
      }
    }
    break;
    case APIC_INTERRUPT_COMMAND_LOW:
    {
      InterruptCommandRegLow low = regs[APIC_INTERRUPT_COMMAND_LOW];
      bool userInterrupt = false;
      // Check if we're already sending an IPI.
      if (low.deliveryStatus)
      {
        newVal = low;
        break;
      }
      low = val;
      InterruptCommandRegHigh high = regs[APIC_INTERRUPT_COMMAND_HIGH];
      TriggerIntMessage message = 0;
      message.destination = high.destination;
      message.vector = low.vector;
      message.deliveryMode = low.deliveryMode;
      message.destMode = low.destMode;
      message.level = low.level;
      message.trigger = low.trigger;

      std::list<int> apics;
      int numContexts = sys->threads.size();
      switch (low.destShorthand)
      {
      case 0:
        if (message.deliveryMode == delivery_mode::LowestPriority)
        {
          panic("Lowest priority delivery mode "
                "IPIs aren't implemented.\n");
        }
        if (message.destMode == 1)
        {
          int dest = message.destination;
          hack_once("Assuming logical destinations are 1 << id.\n");
          for (int i = 0; i < numContexts; i++)
          {
            if (dest & 0x1)
              apics.push_back(i);
            dest = dest >> 1;
          }
        }
        else
        {
          if (message.destination == 0xFF)
          {
            for (int i = 0; i < numContexts; i++)
            {
              if (i == initialApicId)
              {
                requestInterrupt(message.vector,
                                 message.deliveryMode, message.trigger);
              }
              else
              {
                apics.push_back(i);
              }
            }
          }
          else
          {
            if (message.destination == initialApicId)
            {
              requestInterrupt(message.vector,
                               message.deliveryMode, message.trigger);
            }
            else
            {
              apics.push_back(message.destination);
            }
          }
        }
        break;
      case 1:
        newVal = val;
        requestInterrupt(message.vector,
                         message.deliveryMode, message.trigger);
        break;
      case 2:
        requestInterrupt(message.vector,
                         message.deliveryMode, message.trigger);
        // Fall through
      case 3:
      {
        for (int i = 0; i < numContexts; i++)
        {
          if (i != initialApicId)
          {
            apics.push_back(i);
          }
        }
      }
      break;
      }
      // Record that an IPI is being sent if one actually is.
      if (apics.size())
      {
        low.deliveryStatus = 1;
        pendingIPIs += apics.size();
      }
      regs[APIC_INTERRUPT_COMMAND_LOW] = low;
      for (auto id : apics)
      {
        PacketPtr pkt = buildIntTriggerPacket(id, message);
        pkt->tick = tick;
        intRequestPort.sendMessage(pkt, sys->isTimingMode(),
                                   [this](PacketPtr pkt)
                                   { completeIPI(pkt); });
      }
      newVal = regs[APIC_INTERRUPT_COMMAND_LOW];
    }
    break;
    case APIC_LVT_TIMER:
    case APIC_LVT_THERMAL_SENSOR:
    case APIC_LVT_PERFORMANCE_MONITORING_COUNTERS:
    case APIC_LVT_LINT0:
    case APIC_LVT_LINT1:
    case APIC_LVT_ERROR:
    {
      uint64_t readOnlyMask = (1 << 12) | (1 << 14);
      newVal = (val & ~readOnlyMask) |
               (regs[reg] & readOnlyMask);
    }
    break;
    case APIC_INITIAL_COUNT:
    {
      newVal = bits(val, 31, 0);
      // Compute how many timer ticks we're being programmed for.
      uint64_t newCount = newVal *
                          (divideFromConf(regs[APIC_DIVIDE_CONFIGURATION]));
      // Schedule on the edge of the next tick plus the new count.
      Tick offset = curTick() % clockPeriod();
      if (offset)
      {
        reschedule(apicTimerEvent,
                   curTick() + (newCount + 1) * clockPeriod() - offset, true);
      }
      else
      {
        if (newCount)
          reschedule(apicTimerEvent,
                     curTick() + newCount *
                                     clockPeriod(),
                     true);
      }
    }
    break;
    case APIC_CURRENT_COUNT:
      // Local APIC Current Count register is read only.
      return;
    case APIC_DIVIDE_CONFIGURATION:
      newVal = val & 0xB;
      break;
    default:
      break;
    }
    regs[reg] = newVal;
    return;
  }

  X86ISA::Interrupts::Interrupts(const Params &p)
      : BaseInterrupts(p), sys(p.system), clockDomain(*p.clk_domain),
        apicTimerEvent([this]
                       { processApicTimerEvent(); },
                       name()),
        pendingEvent([this]
                     { processPendingEvent(); },
                     name()),
        unlockEvent([this]
                    { processUnlockEvent(); },
                    name()),
        intResponsePort(name() + ".int_responder", this, this),
        intRequestPort(name() + ".int_requestor", this, this, p.int_latency),
        lint0Pin(name() + ".lint0", 0, this, 0),
        lint1Pin(name() + ".lint1", 0, this, 1),
        pioPort(this), pioDelay(p.pio_latency)
  {
    memset(regs, 0, sizeof(regs));
    // Set the local apic DFR to the flat model.
    regs[APIC_DESTINATION_FORMAT] = (uint32_t)(-1);

    // At reset, all LVT entries start out zeroed, except for their mask bit.
    LVTEntry masked = 0;
    masked.masked = 1;

    regs[APIC_LVT_TIMER] = masked;
    regs[APIC_LVT_THERMAL_SENSOR] = masked;
    regs[APIC_LVT_PERFORMANCE_MONITORING_COUNTERS] = masked;
    regs[APIC_LVT_LINT0] = masked;
    regs[APIC_LVT_LINT1] = masked;
    regs[APIC_LVT_ERROR] = masked;

    regs[APIC_SPURIOUS_INTERRUPT_VECTOR] = 0xff;

    regs[APIC_VERSION] = (5 << 16) | 0x14;
  }

  bool
  X86ISA::Interrupts::checkInterrupts()
  {
    RFLAGS rflags = tc->readMiscRegNoEffect(misc_reg::Rflags);
    if (pendingUnmaskableInt)
    {
      DPRINTF(LocalApic, "Reported pending unmaskable interrupt.\n");
      if (ISRVUSER)
        return false;
      return true;
    }
    if (rflags.intf)
    {
      if (pendingExtInt)
      {
        DPRINTF(LocalApic, "Reported pending external interrupt.\n");
        if (ISRVUSER)
          return false;
        return true;
      }
      if (ISRVUSER)
      {
        // std::cerr << "SERVICE? at: " << curTick() << "\n";
        return false;
      }
      UintrPciON uintrPciON = tc->readMiscRegNoEffect(misc_reg::UintrPciON);
      UintrPciDisable uintrPciDisable = tc->readMiscRegNoEffect(misc_reg::UintrPciDisable);
      if (!uintrPciON && !uintrPciDisable)
      {
        UintrPciPending_t uintrPciPending = tc->readMiscRegNoEffect(misc_reg::UintrPciPending);
        if (curTick() > userPciTimeStart + userPciTimeout && (uintrPciPending + auxPending) > 0)
        {
          if (!pendingEvent.scheduled())
          {
            schedule(pendingEvent, curTick() + tc->getCpuPtr()->cyclesToTicks(Cycles(1)));
          }
        }
      }
      if (IRRV > ISRV && bits(IRRV, 7, 4) > bits(regs[APIC_TASK_PRIORITY], 7, 4) && ISRV != 33)
      {
        DPRINTF(LocalApic, "Reported pending regular interrupt.\n");
        return true;
      }
      if (IRRVUSER && !ISRV)
      {

        UintrMisc uintrMisc = tc->readMiscRegNoEffect(misc_reg::UintrMisc);
        UintrTimerStatus uintrTimerStatus = tc->readMiscRegNoEffect(misc_reg::UintrTimerStatus);
        X86ISA::HandyM5Reg m5reg = tc->readMiscRegNoEffect(X86ISA::misc_reg::M5Reg);
        if (m5reg.cpl && uintrMisc.uif && uintrTimerStatus.timer_on && uintrTimerStatus.timer_active && IRRVUSER == uintrTimerStatus.uinv) // magic number
        {
          DPRINTF(LocalApic, "Reported pending user timer.\n");
          DPRINTF(UserInterrupt, "user timer");
          return true;
        }
        if (uintrTimerStatus.timer_on && uintrTimerStatus.timer_active && IRRVUSER == uintrTimerStatus.uinv)
        {
          return false;
        }
        if (m5reg.cpl && IRRVUSER == uintrMisc.uinv && uintrMisc.uif)
        {
          // std::cerr << "DO COME\n";
          DPRINTF(LocalApic, "Reported pending user interrupt.\n");
          return true;
        }
        if (IRRVUSER == uintrMisc.uinv)
        {
          // std::cerr << "UIF: " << (uintrMisc.uif ? "true" : "false") << " CPL: " << m5reg.cpl << std::endl;
          return false;
        }
        UintrPciON uintrPciON = tc->readMiscRegNoEffect(misc_reg::UintrPciON);
        UintrPciDisable uintrPciDisable = tc->readMiscRegNoEffect(misc_reg::UintrPciDisable);
        if (m5reg.cpl && IRRVUSER == 37 && uintrMisc.uif && uintrPciON && !uintrPciDisable)
        {
          // std::cerr << "DO COME\n";
          DPRINTF(LocalApic, "Reported pending user interrupt pci.\n");
          DPRINTF(UserInterrupt, "user pci");
          return true;
        }
      }
    }
    return false;
  }

  bool
  X86ISA::Interrupts::checkInterruptsRaw() const
  {
    return pendingUnmaskableInt || pendingExtInt ||
           (IRRV > ISRV && bits(IRRV, 7, 4) >
                               bits(regs[APIC_TASK_PRIORITY], 7, 4));
  }

  Fault
  X86ISA::Interrupts::getInterrupt()
  {
    assert(checkInterrupts());
    // These are all probably fairly uncommon, so we'll make them easier to
    // check for.
    if (pendingUnmaskableInt)
    {
      if (pendingSmi)
      {
        DPRINTF(LocalApic, "Generated SMI fault object.\n");
        return std::make_shared<SystemManagementInterrupt>();
      }
      else if (pendingNmi)
      {
        DPRINTF(LocalApic, "Generated NMI fault object.\n");
        return std::make_shared<NonMaskableInterrupt>(nmiVector);
      }
      else if (pendingInit)
      {
        DPRINTF(LocalApic, "Generated INIT fault object.\n");
        return std::make_shared<InitInterrupt>(initVector);
      }
      else if (pendingStartup)
      {
        DPRINTF(LocalApic, "Generating SIPI fault object.\n");
        return std::make_shared<StartupInterrupt>(startupVector);
      }
      else
      {
        panic("pendingUnmaskableInt set, but no unmaskable "
              "ints were pending.\n");
        return NoFault;
      }
    }
    else if (pendingExtInt)
    {
      DPRINTF(LocalApic, "Generated external interrupt fault object.\n");
      return std::make_shared<ExternalInterrupt>(extIntVector);
    }
    else
    {
      if (IRRV > ISRV && bits(IRRV, 7, 4) >
                             bits(regs[APIC_TASK_PRIORITY], 7, 4))
      {
        DPRINTF(LocalApic, "Generated regular interrupt fault object.\n");
        // The only thing left are fixed and lowest priority interrupts.
        return std::make_shared<ExternalInterrupt>(IRRV);
      }
      UintrMisc uintrMisc = tc->readMiscRegNoEffect(misc_reg::UintrMisc);

      if (IRRVUSER == 36) // magic number
      {
        DPRINTF(LocalApic, "Generated user timer fault object.\n");
        return std::make_shared<UserTimer>(IRRVUSER, this);
      }
      if (IRRVUSER == uintrMisc.uinv)
      {
        DPRINTF(LocalApic, "Generated user interrupt fault object.\n");
        return std::make_shared<UserInterrupt>(IRRVUSER, this);
      }
      if (IRRVUSER == 37)
      {
        DPRINTF(LocalApic, "Generated user pci fault object.\n");
        return std::make_shared<UserPci>(IRRVUSER, this);
      }
    }
  }

  void
  X86ISA::Interrupts::updateIntrInfo(const Fault &interrupt)
  {
    // assert(checkInterrupts()); BAD BAD
    if (pendingUnmaskableInt)
    {
      if (pendingSmi)
      {
        DPRINTF(LocalApic, "SMI sent to core.\n");
        pendingSmi = false;
      }
      else if (pendingNmi)
      {
        DPRINTF(LocalApic, "NMI sent to core.\n");
        pendingNmi = false;
      }
      else if (pendingInit)
      {
        DPRINTF(LocalApic, "Init sent to core.\n");
        pendingInit = false;
        startedUp = false;
      }
      else if (pendingStartup)
      {
        DPRINTF(LocalApic, "SIPI sent to core.\n");
        pendingStartup = false;
        startedUp = true;
      }
      if (!(pendingSmi || pendingNmi || pendingInit || pendingStartup))
        pendingUnmaskableInt = false;
    }
    else if (pendingExtInt)
    {
      pendingExtInt = false;
    }
    else
    {
      if (interrupt->userInt)
      {
        ISRVUSER = IRRVUSER;
        IRRVUSER = 0; // TODO NOT MORE THAN ONE VECTOR CAN BE DONE
        return;
      }
      DPRINTF(LocalApic, "Interrupt %d sent to core.\n", IRRV);
      // Mark the interrupt as "in service".
      ISRV = IRRV;
      setRegArrayBit(APIC_IN_SERVICE_BASE, ISRV);
      // Clear it out of the IRR.
      clearRegArrayBit(APIC_INTERRUPT_REQUEST_BASE, IRRV);
      updateIRRV();
      if (ISRV == 33 && printInterruptCount)
      {
        printInterruptCount--;
        requestInterrupt(33, delivery_mode::Fixed, false);
      }
    }
  }
  void
  X86ISA::Interrupts::updateIntrInfo()
  {
    // assert(checkInterrupts()); BAD BAD
    if (pendingUnmaskableInt)
    {
      if (pendingSmi)
      {
        DPRINTF(LocalApic, "SMI sent to core.\n");
        pendingSmi = false;
      }
      else if (pendingNmi)
      {
        DPRINTF(LocalApic, "NMI sent to core.\n");
        pendingNmi = false;
      }
      else if (pendingInit)
      {
        DPRINTF(LocalApic, "Init sent to core.\n");
        pendingInit = false;
        startedUp = false;
      }
      else if (pendingStartup)
      {
        DPRINTF(LocalApic, "SIPI sent to core.\n");
        pendingStartup = false;
        startedUp = true;
      }
      if (!(pendingSmi || pendingNmi || pendingInit || pendingStartup))
        pendingUnmaskableInt = false;
    }
    else if (pendingExtInt)
    {
      pendingExtInt = false;
    }
    else
    {
      DPRINTF(LocalApic, "Interrupt %d sent to core.\n", IRRV);
      // Mark the interrupt as "in service".
      ISRV = IRRV;
      setRegArrayBit(APIC_IN_SERVICE_BASE, ISRV);
      // Clear it out of the IRR.
      clearRegArrayBit(APIC_INTERRUPT_REQUEST_BASE, IRRV);
      updateIRRV();
      if (ISRV == 33 && printInterruptCount)
      {
        printInterruptCount--;
        requestInterrupt(33, delivery_mode::Fixed, false);
      }
    }
  }
  void
  X86ISA::Interrupts::serialize(CheckpointOut &cp) const
  {
    SERIALIZE_ARRAY(regs, NUM_APIC_REGS);
    SERIALIZE_SCALAR(pendingSmi);
    SERIALIZE_SCALAR(smiVector);
    SERIALIZE_SCALAR(pendingNmi);
    SERIALIZE_SCALAR(nmiVector);
    SERIALIZE_SCALAR(pendingExtInt);
    SERIALIZE_SCALAR(extIntVector);
    SERIALIZE_SCALAR(pendingInit);
    SERIALIZE_SCALAR(initVector);
    SERIALIZE_SCALAR(pendingStartup);
    SERIALIZE_SCALAR(startupVector);
    SERIALIZE_SCALAR(startedUp);
    SERIALIZE_SCALAR(pendingUnmaskableInt);
    SERIALIZE_SCALAR(pendingIPIs);
    SERIALIZE_SCALAR(IRRV);
    SERIALIZE_SCALAR(ISRV);
    bool apicTimerEventScheduled = apicTimerEvent.scheduled();
    SERIALIZE_SCALAR(apicTimerEventScheduled);
    Tick apicTimerEventTick = apicTimerEvent.when();
    SERIALIZE_SCALAR(apicTimerEventTick);
  }

  void
  X86ISA::Interrupts::unserialize(CheckpointIn &cp)
  {
    UNSERIALIZE_ARRAY(regs, NUM_APIC_REGS);
    UNSERIALIZE_SCALAR(pendingSmi);
    UNSERIALIZE_SCALAR(smiVector);
    UNSERIALIZE_SCALAR(pendingNmi);
    UNSERIALIZE_SCALAR(nmiVector);
    UNSERIALIZE_SCALAR(pendingExtInt);
    UNSERIALIZE_SCALAR(extIntVector);
    UNSERIALIZE_SCALAR(pendingInit);
    UNSERIALIZE_SCALAR(initVector);
    UNSERIALIZE_SCALAR(pendingStartup);
    UNSERIALIZE_SCALAR(startupVector);
    UNSERIALIZE_SCALAR(startedUp);
    UNSERIALIZE_SCALAR(pendingUnmaskableInt);
    UNSERIALIZE_SCALAR(pendingIPIs);
    UNSERIALIZE_SCALAR(IRRV);
    UNSERIALIZE_SCALAR(ISRV);
    bool apicTimerEventScheduled;
    UNSERIALIZE_SCALAR(apicTimerEventScheduled);
    if (apicTimerEventScheduled)
    {
      Tick apicTimerEventTick;
      UNSERIALIZE_SCALAR(apicTimerEventTick);
      if (apicTimerEvent.scheduled())
      {
        reschedule(apicTimerEvent, apicTimerEventTick, true);
      }
      else
      {
        schedule(apicTimerEvent, apicTimerEventTick);
      }
    }
  }

  void
  X86ISA::Interrupts::processApicTimerEvent()
  {
    if (triggerTimerInterrupt())
      setReg(APIC_INITIAL_COUNT, readReg(APIC_INITIAL_COUNT), 0);
  }

  void
  X86ISA::Interrupts::processPendingEvent()
  {
    UintrPciLock uintrPciLock = tc->readMiscRegNoEffect(misc_reg::UintrPciLock);
    if (!uintrPciLock && !unlockEvent.scheduled())
    {
      reinterpret_cast<o3::CPU *>(tc->getCpuPtr())->setMiscRegNoEffect(misc_reg::UintrPciLock, 2, tc->threadId()); // acquire lock
      UintrPciPending_t uintrPciPending = tc->readMiscRegNoEffect(misc_reg::UintrPciPending);
      UintrPciON uintrPciON = tc->readMiscRegNoEffect(misc_reg::UintrPciON);
      int64_t totalPending = auxPending + uintrPciPending;
      if (!uintrPciON && ((curTick() > userPciTimeStart + userPciTimeout && totalPending > 0) || totalPending >= userPciThreshold))
      {
        uintrPciPending = totalPending;
        auxPending = 0;
        DPRINTF(UserInterrupt, "IRRVUSER set PCI");
        IRRVUSER = 37;
        if (FullSystem)
          tc->getCpuPtr()->wakeup(0);
        reinterpret_cast<o3::CPU *>(tc->getCpuPtr())->setMiscRegNoEffect(misc_reg::UintrPciON, 1, tc->threadId());
        userPciTimeStart = curTick();
      }
      reinterpret_cast<o3::CPU *>(tc->getCpuPtr())->setMiscRegNoEffect(misc_reg::UintrPciPending, uintrPciPending, tc->threadId());
      schedule(unlockEvent, curTick() + tc->getCpuPtr()->cyclesToTicks(Cycles(2)));
    }
    else
    {
      schedule(pendingEvent, curTick() + tc->getCpuPtr()->cyclesToTicks(Cycles(1)));
    }
  }
  void
  X86ISA::Interrupts::processUnlockEvent()
  {
    UintrPciLock uintrPciLock = tc->readMiscRegNoEffect(misc_reg::UintrPciLock);
    if (uintrPciLock == 1)
    {
      return;
    }
    assert(uintrPciLock);
    reinterpret_cast<o3::CPU *>(tc->getCpuPtr())->setMiscRegNoEffect(misc_reg::UintrPciLock, 0, tc->threadId());
  }

} // namespace gem5
