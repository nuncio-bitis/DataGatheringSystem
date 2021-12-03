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
/*
 * MasterTask.h
 *
 * Created on: Feb 20, 2020
 * Author: jparziale
 */

#ifndef MASTERTASK_H_
#define MASTERTASK_H_
// ****************************************************************************

#include "Task.h"
#include "Event.h"
#include "Logger.h"

#include <vector>
#include "AppTask.h"

class MasterTask : public Task
{
    // ------------------------------------------------------------------------
public:
    enum class SystemState {
        Running,
        Paused,
        Terminating,
        Terminated,
        NUM_STATES
    };

    MasterTask(const std::string name, Logger* pLog);
    virtual ~MasterTask();

    // Method to add app tasks
    void AddAppTask(AppTask* pAppTask);

    void Start();  // Start Master task and all registered app tasks.
    void PauseSystem() { m_state = SystemState::Paused; };
    void ContinueSystem() { m_state = SystemState::Running; };
    void TerminateSystem() {
        if (isTerminated()) return;
        m_state = SystemState::Terminating;
        // Wait for all tasks to be told to stop.
        while (!isTerminated()) {
            Task::Sleep(500);
        }
    };

    bool isTerminated() { return (m_state == SystemState::Terminated); };

    SystemState GetState(void) { return m_state; };
    std::string GetStateName(void);

    std::string GetSystemInfo() {
        std::stringstream out;
        out << "State of System Tasks:" << std::endl;
        out << "  Task '" << GetName() << "' : " << GetStateName() << std::endl;
        for (auto t : m_TaskList) {
            t->GetInfo(out);
        }
        return out.str();
    }

    void GetSystemInfo(std::ostream& out) {
        out << GetSystemInfo();
    }

    // ------------------------------------------------------------------------
private:
    void Entry();

    static const std::string StateNames[(int)SystemState::NUM_STATES];

    SystemState m_state;
    Logger* m_pLog;

    // List of all system tasks
    std::vector<AppTask*> m_TaskList;

    // ------------------------------------------------------------------------
private:

};

// ****************************************************************************
#endif /* MASTERTASK_H_ */
