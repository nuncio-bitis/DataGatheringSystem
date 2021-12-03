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
 * UITask.cpp
 *
 * Created on: Feb 20, 2020
 * Author: jparziale
 */
// ****************************************************************************

#include "UITask.h"
#include "DataStore.h"

#include <iostream>

// ****************************************************************************

UITask::UITask(const std::string name, int id, Logger* pLog) :
    AppTask(name, pLog),
    m_id(id),
    m_pLog(pLog),
    cpu_mem_free_tok(0),
    cpu_temp_tok(0),
    temp_1_tok(0),
    temp_2_tok(0),
    press_1_tok(0),
    press_2_tok(0),
    flow_1_tok(0),
    flow_2_tok(0)
{
    cpu_mem_free = dynamic_cast<DataItem<uint64_t> *>(DataStore::getInstance()->GetDataItem(CPU_MEM_FREE));
    cpu_temp = dynamic_cast<DataItem<double> *>(DataStore::getInstance()->GetDataItem(CPU_TEMP));
    temp_1   = dynamic_cast<DataItem<double> *>(DataStore::getInstance()->GetDataItem(TEMP_SENSE_1));
    temp_2   = dynamic_cast<DataItem<double> *>(DataStore::getInstance()->GetDataItem(TEMP_SENSE_2));
    press_1  = dynamic_cast<DataItem<double> *>(DataStore::getInstance()->GetDataItem(PRESSURE1));
    press_2  = dynamic_cast<DataItem<double> *>(DataStore::getInstance()->GetDataItem(PRESSURE2));
    flow_1   = dynamic_cast<DataItem<double> *>(DataStore::getInstance()->GetDataItem(FLOW1));
    flow_2   = dynamic_cast<DataItem<double> *>(DataStore::getInstance()->GetDataItem(FLOW2));

    m_pLog->log(eLOG_DEBUG, "%s(%d) : CREATED", GetName().c_str(), m_id);
}

UITask::~UITask()
{
    m_pLog->log(eLOG_DEBUG, "%s.%s : DONE", GetName().c_str(), __FUNCTION__);
}

// ****************************************************************************

void UITask::UpdateItem(DataItem<uint64_t> *item)
{
    uint64_t value;
    DataItemState state = item->getValue(value);

    if (state == DataItemState::Invalid) {
        return;
    }

    std::string sstate;
    if (state == DataItemState::OutOfRangeLow) {
        sstate = "OORL";
        m_pLog->log(eLOG_MED, "[UI] (%s) %s %lld %s *LOW*",
                sstate.c_str(), item->name().c_str(), value, item->getUnits().c_str());
    } else if (state == DataItemState::OutOfRangeHigh) {
        sstate = "OORH";
        m_pLog->log(eLOG_MED, "[UI] (%s) %s %lld %s *HIGH*",
                sstate.c_str(), item->name().c_str(), value, item->getUnits().c_str());
    } else if (state == DataItemState::Stale) {
        sstate = "STALE";
        m_pLog->log(eLOG_MED, "[UI] (%s) %s %lld %s *STALE*",
                sstate.c_str(), item->name().c_str(), value, item->getUnits().c_str());
    } else if (state == DataItemState::Valid) {
        sstate = "OK";
        m_pLog->log(eLOG_DEBUG, "[UI] (%s) %s %lld %s",
                sstate.c_str(), item->name().c_str(), value, item->getUnits().c_str());
    }

    // TODO Update value on UI
}

void UITask::UpdateItem(DataItem<double> *item)
{
    double value;
    DataItemState state = item->getValue(value);

    if (state == DataItemState::Invalid) {
        return;
    }

    std::string sstate;
    if (state == DataItemState::OutOfRangeLow) {
        sstate = "OORL";
        m_pLog->log(eLOG_MED, "[UI] (%s) %s %lld %s *LOW*",
                sstate.c_str(), item->name().c_str(), value, item->getUnits().c_str());
    } else if (state == DataItemState::OutOfRangeHigh) {
        sstate = "OORH";
        m_pLog->log(eLOG_MED, "[UI] (%s) %s %lld %s *HIGH*",
                sstate.c_str(), item->name().c_str(), value, item->getUnits().c_str());
    } else if (state == DataItemState::Stale) {
        sstate = "STALE";
        m_pLog->log(eLOG_MED, "[UI] (%s) %s %lld %s *STALE*",
                sstate.c_str(), item->name().c_str(), value, item->getUnits().c_str());
    } else if (state == DataItemState::Valid) {
        sstate = "OK";
        m_pLog->log(eLOG_DEBUG, "[UI] (%s) %s %lg %s",
                sstate.c_str(), item->name().c_str(), value, item->getUnits().c_str());
    }

    // TODO Update value on UI
}

// ****************************************************************************

// Update the UI when a data item has been updated.
void UITask::DataItemUpdated(int id)
{
    DataItemPublisher* pdi = DataStore::GetDataItem((DataItemId)id);
    m_pLog->log(eLOG_DEBUG, "[UI] Data Item '%s' update detected.", pdi->name().c_str());
/*
    switch (id) {
    case CPU_MEM_FREE:
    {
        UpdateItem(cpu_mem_free);
        break;
    }

    // These are all the same type, so it's easy to make generic code.
    case CPU_TEMP:
    case TEMP_SENSE_1:
    case TEMP_SENSE_2:
    case PRESSURE1:
    case PRESSURE2:
    case FLOW1:
    case FLOW2:
    {
        DataItem<double> *pdi = dynamic_cast<DataItem<double> *>(DataStore::getInstance()->GetDataItem((DataItemId)id));
        UpdateItem(pdi);
        break;
    }

    default:
        break;
    }
*/
}

// ****************************************************************************

void UITask::Entry()
{
    waitForBeginOperation();

    // ------------------------------------------------
    // Task init... subscribe to data items
    // NOTE: This isn't necessary if the UI will simply poll the data items.
    cpu_mem_free_tok = cpu_mem_free->subscribe(this);
    cpu_temp_tok     = cpu_temp->subscribe(this);
    temp_1_tok       = temp_1->subscribe(this);
    temp_2_tok       = temp_2->subscribe(this);
    press_1_tok      = press_1->subscribe(this);
    press_2_tok      = press_2->subscribe(this);
    flow_1_tok       = flow_1->subscribe(this);
    flow_2_tok       = flow_2->subscribe(this);

    m_pLog->log(eLOG_DEBUG, "%s: BEGIN + Initialization", GetName().c_str());
    // ------------------------------------------------

    TaskState prevState = GetState();

    while (!isStopped())
    {
        if (prevState != GetState())
        {
            m_pLog->log(eLOG_DEBUG, "%s: %s", GetName().c_str(), GetStateName().c_str());
        }

        // --------------------------------------------
        // TODO Task work - pull values from data store to update display.

        static int count = 0;

        Sleep(100);
        if (++count >= 10) {
            // About every 1 second...
            count = 0;

            // Unconditionally report data every update period.
            // Note this is separate from DataItemUpdated() being called when data is updated.
            UpdateItem(cpu_mem_free);
            UpdateItem(cpu_temp);
            UpdateItem(temp_1);
            UpdateItem(temp_2);
            UpdateItem(press_1);
            UpdateItem(press_2);
            UpdateItem(flow_1);
            UpdateItem(flow_2);
        }
        // --------------------------------------------

        // PAUSED: Must wait to be told to continue.
        if (isPaused())
        {
            // TODO Blank & power off screens,LEDs, etc.

            m_pLog->log(eLOG_DEBUG, "--- %s Paused", GetName().c_str());
            waitForContinue();
            m_pLog->log(eLOG_DEBUG, "--- %s Continuing", GetName().c_str());
        }

        prevState = GetState();
    } // end while running

    // ------------------------------------------------
    // Task cleanup... unsubscribe from data items
    cpu_mem_free->unsubscribe(cpu_mem_free_tok);
    cpu_temp->unsubscribe(cpu_temp_tok);
    temp_1->unsubscribe  (temp_1_tok);
    temp_2->unsubscribe  (temp_2_tok);
    press_1->unsubscribe (press_1_tok);
    press_2->unsubscribe (press_2_tok);
    flow_1->unsubscribe  (flow_1_tok);
    flow_2->unsubscribe  (flow_2_tok);
    m_pLog->log(eLOG_DEBUG, "%s.%s : CLEANUP", GetName().c_str(), __FUNCTION__);
    // ------------------------------------------------
}

// ****************************************************************************
