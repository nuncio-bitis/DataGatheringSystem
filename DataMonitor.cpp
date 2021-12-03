/* 
 * This file is part of the DataGatheringSystem distribution
 *   (https://github.com/nuncio-bitis/DataGatheringSystem
 * Copyright (c) 2021 James P. Parziale.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <ctime>
#include <cstring>

#include <signal.h>
#include <execinfo.h>

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <ucontext.h>


#include <iostream>
#include <fstream>
#include <memory>
#include <fstream>
#include <string>
#include <sstream>
#include <iterator>

#include "Logger.h"
#include "Timer.h"

#include "MsgQueue.h"
#include "DataItem.h"
#include "DataStore.h"

#include "SystemClock.h"

#include "MasterTask.h"
#include "UITask.h"

#include "FluidTempSensor.h"
#include "SensorTask.h"

//-----------------------------------------------------------------------------

//  Exception handling
void SetupExceptionHandler();

static Logger *pLogger = nullptr;

//-----------------------------------------------------------------------------

// /proc/meminfo (MemFree:)
int64_t getCPUFreeMem()
{
    int64_t ret = -1;

#if defined(_POSIX_C_SOURCE) && !defined(_DARWIN_C_SOURCE)
    const std::string meminfo("/proc/meminfo");
    std::ifstream infofile;
    infofile.open (meminfo, std::ios::in);

    if (infofile.is_open()) {
        std::string line;
        while (std::getline(infofile,line)) {
            // Read lines until find "MemFree:"
            std::string::size_type n = line.find("MemFree:");
            if (n != std::string::npos) {
                break;
            }
        }

        // Split string into words (vector of strings)
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::copy(std::istream_iterator<std::string>(iss),
                std::istream_iterator<std::string>(),
                back_inserter(tokens));

        ret = std::stoll(tokens[1]);
        if ("kB" == tokens[2]) {
            ret *= 1024;
        } else if ("MB" == tokens[2]) {
            ret *= 1024 * 1024;
        }

        infofile.close();
    }
#else
    // TODO
#endif

    return ret;
}

//-----------------------------------------------------------------------------

// /opt/vc/bin/vcgencmd measure_temp
double getCPUTemp()
{
    double systemp = -1.0;

#if defined(_POSIX_C_SOURCE) && !defined(_DARWIN_C_SOURCE)
    double millideg = 0.0;
    FILE *thermal;

    thermal = fopen("/sys/class/thermal/thermal_zone0/temp","r");
    if (thermal != NULL)
    {
        if (fscanf(thermal,"%lf",&millideg) < 1) {
            millideg = -273000.0;
        }
        fclose(thermal);
        systemp = millideg / 1000;
    }
#else
    // TODO
#endif

    return systemp;
}

//-----------------------------------------------------------------------------

// Timer to read internal sensor data (CPU and system board items)
class StatTimer : public Timer
{
public:
    StatTimer(const std::string &name,
            const IntervalMs &start,
            const IntervalMs &interval,
            Logger *plogger) :
       Timer(name, start, interval),
       m_plogger(plogger)
    {
        cpu_mem_free = dynamic_cast<DataItem<uint64_t> *>(DataStore::getInstance()->GetDataItem(CPU_MEM_FREE));
        cpu_temp = dynamic_cast<DataItem<double> *>(DataStore::getInstance()->GetDataItem(CPU_TEMP));
        m_plogger->log(eLOG_INFO, "StatTimer(%s) Start:%d, Intv:%d", name.c_str(), start, interval);
    };

    void Start()
    {
        Timer::Start();
    }

    void Reset(const IntervalMs &interval)
    {
        Timer::Reset(interval);
    }

    void ExpirationRoutine() override
    {
        // CPU_MEM_FREE
        int64_t mfree = getCPUFreeMem();
        if (mfree > 0)
        {
            cpu_mem_free->setValue(mfree);
        }

        // CPU_TEMP
        double ctmp = getCPUTemp();
        if (ctmp > 0.0)
        {
            cpu_temp->setValue(ctmp);
        }
    }

private:
    Logger *m_plogger;
    DataItem<uint64_t> *cpu_mem_free;
    DataItem<double> *cpu_temp;
};

//-----------------------------------------------------------------------------

int main(int argc, char **argv)
{
    //  Create the system signal/exception handlers.
    SetupExceptionHandler();

    // Create system logger object
    Logger logger("DataMonitor", eLOG_DEBUG);
    pLogger = &logger;

    logger.MirrorToStdOut();

    //-------------------------------------------------------------------------

    // Create DataStore that owns all data objects in the system.
    // This includes data that sensor tasks write to and the UI task reads.
    // This is a singleton so system objects can get access to it by calling getInstance.

    DataStore systemData;

    //-------------------------------------------------------------------------

    // Create all system task objects
    // General task parameters:
    //   name, id, *Logger
    UITask ui{"User Interface", 104, &logger};

    // Sensor tasks (Type = flow, pressure, temperature, light, humidity, ...)
    //   type (string), sample freq (double), report period (samples in this period are averaged before reported)
    FluidTempSensor sensor1{"Fluid Temp",      TEMP_SENSE_1, &logger, "Temperature", 20.0, 2.0}; // 20Hz, update every 2 seconds
//    SensorTask      sensor1{"Fluid Temp",      TEMP_SENSE_1, &logger, "Temperature", 20.0, 2.0}; // 20Hz, update every 2 seconds
    SensorTask      sensor2{"Gas Flow Rate",   FLOW1,        &logger, "Flow",         5.0, 1.0}; // 5Hz, update every 1 seconds
    SensorTask      sensor3{"Gas Pressure",    PRESSURE1,    &logger, "Pressure",    10.0, 3.0}; // 10Hz, update every 3 seconds

    // NOTE: Sensor tasks have their own timers to read and average sensor data, then write to DataStore objects.

    // Timer to update internal HW stats
    StatTimer statTimer("IntStatTimer", (Timer::IntervalMs)2000, (Timer::IntervalMs)1000, &logger);

    //-------------------------------------------------------------------------

    // Create MasterTask object (which owns all other task objects)
    MasterTask masterTask("Master Task", &logger);
    // Don't start it yet...

    // Add tasks
    masterTask.AddAppTask(&ui);
    masterTask.AddAppTask(&sensor1);
    masterTask.AddAppTask(&sensor2);
    masterTask.AddAppTask(&sensor3);

    //-------------------------------------------------------------------------

    // TODO Create CLI object

    //-------------------------------------------------------------------------

    logger.log(eLOG_INFO, "main() Current OS uptime: %.03f seconds", SystemClock::GetSeconds());

    // Start the master task, which will bring up all other tasks.
    masterTask.Start();
    // MasterTask will terminate if no tasks were added.
    if (masterTask.isTerminated()) {
        logger.log(eLOG_CRIT, "main() MasterTask has terminated.");
        return EXIT_FAILURE;
    }

    // Start internal HW stats-gathering timer
    statTimer.Start();

    //-------------------------------------------------------------------------

    // XXX DEBUG - sample system lifecycle
    int count = 0;

    Task::Sleep(1 * 1000);

    // This would be the application background in a normal system...
    while (!masterTask.isTerminated()) {


        // ------------------------------------------------
        switch (count) {
        case 1:
            count++;
            logger.log(eLOG_INFO, "----------------------------------------");
            logger.log(eLOG_INFO, "main(%d) Pausing system...", count);
            logger.log(eLOG_INFO, "----------------------------------------");
            statTimer.Pause();
            masterTask.PauseSystem();
            Task::Sleep(5 * 1000);
            break;

        case 3:
            count++;
            logger.log(eLOG_INFO, "----------------------------------------");
            logger.log(eLOG_INFO, "main(%d) Resuming system...", count);
            logger.log(eLOG_INFO, "----------------------------------------");
            statTimer.Resume();
            masterTask.ContinueSystem();
            Task::Sleep(5 * 1000);
            break;

        case 5:
            count++;
            logger.log(eLOG_INFO, "----------------------------------------");
            logger.log(eLOG_INFO, "main(%d) Terminating system...", count);
            logger.log(eLOG_INFO, "----------------------------------------");
            masterTask.TerminateSystem();
            Task::Sleep(1 * 1000);
            break;

        default:
            count++;
            // TODO - Background processing, maybe including CLI...

            logger.log(eLOG_INFO, "----------------------------------------");
            logger.log(eLOG_INFO, "main(%d) Doing work...", count);

            logger.log(eLOG_INFO, "----------------------------------------");
            logger.log(eLOG_INFO, masterTask.GetSystemInfo());
            logger.log(eLOG_INFO, "----------------------------------------");
            Task::Sleep(5 * 1000);
            break;
        }
        // ------------------------------------------------

    }
    logger.log(eLOG_INFO, "----------------------------------------");
    logger.log(eLOG_INFO, masterTask.GetSystemInfo());
    logger.log(eLOG_INFO, "----------------------------------------");
    // XXX DEBUG

    statTimer.Stop();

    //-------------------------------------------------------------------------

    logger.log(eLOG_INFO, "main() Ending");

    logger.WriteToFile();
    std::cout << "Log file '" << logger.name() << "' " << logger.Size() << " bytes" << std::endl;

    //-------------------------------------------------------------------------

    return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------


//! Save system device state
void SaveSystemState(void)
{
    //  Shut down all active scripts.  This will terminate BIT and freeze
    //  the state of the DST.
//    gObjMgr->ServerConsole()->Display(1, "Stopping all scripts");
//    gObjMgr->ScriptManager()->StopAllScripts();

    pLogger->log(eLOG_CRIT, "Saving system state");
    pLogger->WriteToFile();
    std::cerr << std::endl;
}

/*! \brief  Get fault description.
 *
 *  This function converts the signal/code combo into descriptive text.
 *
 *  \param signal   Signal ID
 *  \param code     signal subcode
 *  \return         string with description
 */
const char* FaultString(int signal, int code)
{
    if(SIGILL==signal)
    {
        switch(code)
        {
        case ILL_ILLOPC:
            return "illegal opcode";
        case ILL_ILLOPN:
            return "illegal operand";
        case ILL_ILLADR:
            return "illegal addressing mode";
        case ILL_ILLTRP:
            return "illegal trap";
        case ILL_PRVOPC:
            return "privileged register";
        case ILL_COPROC:
            return "coprocessor error";
        case ILL_BADSTK:
            return "internal stack error";
        case 0:
            return "SIGILL";
        default:
            return "unknown SIGILL code";
        }
    }

    if(SIGFPE==signal)
    {
        switch(code)
        {
        case FPE_INTDIV:
            return "integer divide by zero";
        case FPE_INTOVF:
            return "integer overflow";
        case FPE_FLTDIV:
            return "floating point divide by zero";
        case FPE_FLTOVF:
            return "floating point overflow";
        case FPE_FLTUND:
            return "floating point underflow";
        case FPE_FLTRES:
            return "floating point inexact result";
        case FPE_FLTINV:
            return "floating point invalid operation";
        case FPE_FLTSUB:
            return "subscript out of range";
        case 0:
            return "SIGFPE";
        default:
            return "unknown SIGFPE code";
        }
    }

    if(SIGSEGV==signal)
    {
        switch(code)
        {
        case SEGV_MAPERR:
            return "address not mapped to object";
        case SEGV_ACCERR:
            return "invalid permissions for mapped object";
        case 0:
            return "SIGSEGV";
        default:
            return "unknown SIGSEGV code";
        }
    }

    if(SIGBUS==signal)
    {
        switch(code)
        {
        case BUS_ADRALN:
            return "invalid address alignment";
        case BUS_ADRERR:
            return "non-existent physical address";
        case BUS_OBJERR:
            return "object specific hardware error";
        case 0:
            return "SIGBUS";
        default:
            return "unknown SIGBUS code";
        }
    }

    if (SIGINT==signal)
    {
        return "SIGINT; Terminated by user";
    };

    if (SIGABRT==signal)
    {
        if (code > 0) {
            return "Kernel-generated signal";
        }
#if defined(_POSIX_C_SOURCE) && !defined(_DARWIN_C_SOURCE)
        switch(code)
        {
        case SI_TKILL:
            return "Self-abort; tkill()";

        case SI_SIGIO:
            return "IO";

        case SI_ASYNCIO:
            return "Async IO";

        case SI_MESGQ:
            return "Msg Queue state change";

        case SI_TIMER:
            return "Timer expiration";

        case SI_QUEUE:
            return "SIGQUEUE";

        case SI_USER:
            return "SIGABRT; User-generated kill, sigend, raise";

        default:
            return "External abort";
        }
#else
        switch(code)
        {
        case SI_USER:
            return "Self-abort";

        case SI_QUEUE:
            return "SIGQUEUE";

        case SI_TIMER:
            return "Timer expiration";

        case SI_ASYNCIO:
            return "Async IO";

        case SI_MESGQ:
            return "Msg Queue state change";

        default:
            return "External abort";
        }
#endif
    };

    return "Unhandled signal handler";
}

//-----------------------------------------------------------------------------

/*! \brief Process processor exception signals
 *
 *  This function collects the exception information and prints a standard
 *  diagnostic message via qDebug
 *
 *  Note: Linux signal handling on some archs works by the kernel replacing the
 *  return address of the faulting function (within the stack) with the signla
 *  "unwinding" function, which would later restore it. What this means is that
 *  the backtrace we get is missing the single most important bit of information:
 *  the addres of the faulting function.  We can fix this by going (if necessary)
 *  to the signal structure and extracting the saved function address.
 *
 *  \param signal   The signal being trapped.
 *  \param siginfo  The signal information
 *  \param context  The process context at the time of the exception
 */
void FaultHandler(int signal, siginfo_t * siginfo, void *context)
{
    //  Register the server cleanup function
    atexit(SaveSystemState);

    //  ^C is not really a fault...
    if (signal == SIGINT)
    {
        std::cerr << "[terminated by user]" << std::endl;
        exit(signal);
    };

    //  ...neither is a system terminate signal
    if (signal == SIGTERM)
    {
        std::cerr << "[terminated by system]" << std::endl;
        exit(signal);
    };

    //  Real fault handling
    const int maxbtsize = 128; // display up to this many back calls
    const int max_line_size = 256; // max no. of chars in each backtrace line

    void*  stack[maxbtsize];
    ucontext_t * cntxt = (ucontext_t *)context;

    // Output buffers
    char  exAddr[max_line_size];
    char  exText[max_line_size];
    char  exError[max_line_size];
    char  exBackTrace[maxbtsize * max_line_size + 1];

    memset(stack, 0, sizeof(stack));

    //  Get exception address
    snprintf(exAddr, sizeof(exAddr), "0x%p", (void *)siginfo->si_addr);

    //  Get exception codes & description
    snprintf(exText, sizeof(exText), "{%d, %d} %s", signal, siginfo->si_code, FaultString(signal, siginfo->si_code));

    //  Get any error code associated with exception
    snprintf(exError, sizeof(exError), "%d", siginfo->si_errno);

    //  Get backtrace of stack.  For some architectures, replace missing address.
    int btSz  = backtrace((void**)stack, maxbtsize);
#if defined(_POSIX_C_SOURCE) && !defined(_DARWIN_C_SOURCE)
    stack[3] = (void *)(cntxt->uc_mcontext.gregs[REG_RIP]);     // x86_64
//    stack[3] = (void *)(cntxt->uc_mcontext.gregs[REG_EIP]);   // i386
//    stack[3] = (void *)(cntxt->uc_mcontext.regs->nip);        // PPC
#else
//    stack[3] = (void *)(cntxt->uc_mcontext->__ss.__rip);        // x86_64 MacOs
    stack[3] = (void *)(cntxt->uc_mcontext->__ss.__pc);        // M1 Pro MacOs
#endif

    //  Get backtrace info
    if (btSz <= 0)
    {
        // None available
        strcpy(exBackTrace, "not available");
    }
    else
    {
        //  Try to get strings with symbolic info
        char ** symbols = backtrace_symbols((void**)stack, btSz);
        if (symbols)
        {
            char tmpStr[max_line_size];
            for (int t=0; t < btSz; t++)
            {
                snprintf(tmpStr, sizeof(tmpStr), "\n%2d [%p] %s", t, stack[t], symbols[t]);
                strncat(exBackTrace, tmpStr, (maxbtsize * max_line_size));
            }
        }
        else
        {
            // Send symbolic info to a text file.  (This backtrace funcion takes a file
            // descriptor, which we have to sneak out of the FILE structure via "_file".)
            std::string timestamp = pLogger->TimeStamp();
            strcpy(exBackTrace, (timestamp + std::string(".backtrace")).c_str());
            FILE* crashFile = fopen(exBackTrace, "w");
#if defined(_POSIX_C_SOURCE) && !defined(_DARWIN_C_SOURCE)
            backtrace_symbols_fd(stack, btSz, crashFile->_fileno);
#else
            backtrace_symbols_fd(stack, btSz, crashFile->_file);
#endif
            fclose(crashFile);
        }
        free(symbols);
    }

    //  Build the message
    std::cerr << std::endl << "*** SYSTEM_EXCEPTION FAIL ::FaultHandler(): " << std::endl
            << "  Error: " << exError << std::endl
            << "  Addr: " << exAddr << std::endl
            << "  Text: " << exText << std::endl
            << "  Backtrace: " << exBackTrace
            << std::endl << std::endl;

    exit(signal);
}

//-----------------------------------------------------------------------------

/*! \brief Create the exception handling system.
 *
 *  This function registers the handler for specified system exceptions.
 */
void SetupExceptionHandler()
{
    struct sigaction action;

    memset(&action, 0, sizeof (action));
    action.sa_sigaction = FaultHandler;
    sigemptyset (&action.sa_mask);
    action.sa_flags = SA_SIGINFO | SA_RESETHAND;

    if (-1 == sigaction (SIGSEGV, &action, NULL))
    {
        std::cerr << "SYSTEM_CALL_FAILED FAIL cExceptionHandler() sigaction(SIGSEGV...)" << std::endl;
    };

    if (-1 == sigaction (SIGILL, &action, NULL))
    {
        std::cerr << "SYSTEM_CALL_FAILED FAIL cExceptionHandler() sigaction(SIGILL...)" << std::endl;
    };

    if (-1 == sigaction (SIGINT, &action, NULL))
    {
        std::cerr << "SYSTEM_CALL_FAILED FAIL cExceptionHandler() sigaction(SIGINT...)" << std::endl;
    };

    if (-1 == sigaction (SIGFPE, &action, NULL))
    {
        std::cerr << "SYSTEM_CALL_FAILED FAIL cExceptionHandler() sigaction(SIGFPE...)" << std::endl;
    };

    if (-1 == sigaction (SIGBUS, &action, NULL))
    {
        std::cerr << "SYSTEM_CALL_FAILED FAIL cExceptionHandler() sigaction(SIGBUS...)" << std::endl;
    };

    if (-1 == sigaction (SIGQUIT, &action, NULL))
    {
        std::cerr << "SYSTEM_CALL_FAILED FAIL cExceptionHandler() sigaction(SIGQUIT...)" << std::endl;
    };

    if (-1 == sigaction (SIGABRT, &action, NULL))
    {
        std::cerr << "SYSTEM_CALL_FAILED FAIL cExceptionHandler() sigaction(SIGABRT...)" << std::endl;
    };

    if (-1 == sigaction (SIGTERM, &action, NULL))
    {
        std::cerr << "SYSTEM_CALL_FAILED FAIL cExceptionHandler() sigaction(SIGTERM...)" << std::endl;
    };

#if defined(_POSIX_C_SOURCE) && !defined(_DARWIN_C_SOURCE)
    if (-1 == sigaction (SIGSTKFLT, &action, NULL))
    {
        std::cerr << "SYSTEM_CALL_FAILED FAIL cExceptionHandler() sigaction(SIGSTKFLT...)" << std::endl;
    };
#endif

    if (-1 == sigaction (SIGSYS, &action, NULL))
    {
        std::cerr << "SYSTEM_CALL_FAILED FAIL cExceptionHandler() sigaction(SIGSSYS...)" << std::endl;
    };
};

//-----------------------------------------------------------------------------
