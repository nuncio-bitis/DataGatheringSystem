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
 * MasterTask.cpp
 *
 * Created on: Feb 20, 2020
 * Author: jparziale
 */

// ****************************************************************************

#include <iostream>
#include "MasterTask.h"
#include "SystemClock.h"

// ****************************************************************************

const std::string MasterTask::StateNames[(int)SystemState::NUM_STATES] = {
        "Running",
        "Paused",
        "Terminating",
        "Terminated"
};

// ****************************************************************************

MasterTask::MasterTask(const std::string name, Logger* pLog) :
    Task(name),
    m_state(SystemState::Running),
    m_pLog(pLog)
{
    m_pLog->log(eLOG_DEBUG, "%s : CREATED", GetName().c_str());
}

MasterTask::~MasterTask()
{
    m_pLog->log(eLOG_DEBUG, "%s.%s : DONE", GetName().c_str(), __FUNCTION__);
}

// ****************************************************************************

std::string MasterTask::GetStateName(void)
{
    return StateNames[(int)m_state];
}

// ****************************************************************************

void MasterTask::AddAppTask(AppTask* pAppTask)
{
    m_TaskList.push_back(pAppTask);
}

// ****************************************************************************

// Start Master task and all registered app tasks.
void MasterTask::Start()
{
    if (m_TaskList.size() == 0) {
        m_pLog->log(eLOG_CRIT, "%s : There are no system tasks! Quitting.", GetName().c_str());
        m_state = SystemState::Terminated;
        return;
    }

    // Start all system tasks
    for (auto t : m_TaskList) {
        t->Start();
    }

    Task::Start();
};

// ****************************************************************************

void MasterTask::Entry()
{
    m_pLog->log(eLOG_DEBUG, "%s: START + Initialization", GetName().c_str());

    //-------------------------------------------------------------------------

    // Begin all system tasks
    for (auto t : m_TaskList) {
        t->BeginOperation();
    }

    //-------------------------------------------------------------------------

    SystemState prevState = m_state;

    while (m_state != SystemState::Terminating) {
        Sleep(200); // Check system state periodically.

        if ((m_state == SystemState::Paused) && (prevState == SystemState::Running)) {
            // Transition: RUNNING -> PAUSED

            m_pLog->log(eLOG_DEBUG, "%s: Pause tasks...", GetName().c_str());
            // Pause all tasks
            for (auto t : m_TaskList) {
                t->Pause();
            }

        } else if ((m_state == SystemState::Running) && (prevState == SystemState::Paused)) {
            // Transition: PAUSED -> RUNNING

            m_pLog->log(eLOG_DEBUG, "%s: Unpause tasks...", GetName().c_str());
            // Continue all tasks
            for (auto t : m_TaskList) {
                t->Continue();
            }

        }

        prevState = m_state;

    } // end while

    m_pLog->log(eLOG_DEBUG, "%s: Stop tasks...", GetName().c_str());
    // Stop all tasks
    for (auto t : m_TaskList) {
        t->Stop();
    }

    //-------------------------------------------------------------------------

    m_pLog->log(eLOG_DEBUG, "%s.%s : CLEANUP", GetName().c_str(), __FUNCTION__);

    m_state = SystemState::Terminated;
}

// ****************************************************************************
