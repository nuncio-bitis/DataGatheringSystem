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
 * SensorTask.cpp
 *
 * Created on: Feb 20, 2020
 * Author: jparziale
 */
// ****************************************************************************

#include "DataStore.h"
#include "SensorTask.h"

#include <iostream>

// XXX
#include <ctime>
#include <sys/time.h>
// XXX

// ****************************************************************************

// TODO Each sensor should have specifics for its own HW interface
// e.g. Type ("I2C", "SPI", "Serial", Mem-Mapped Addr, etc), Location ("/dev/...", "0x########", etc)

// The id is the DataStore ID, which can be used for multiple purposes.
// For example, PRESSURE1 and FLOW1 are being handled here using the same sensor class,
// but instantiated as separate objects. (see main in DataMonitor.cpp)
SensorTask::SensorTask(const std::string name, DataItemId id, Logger* pLog,
                       const std::string type, double sampleFreq, double reportPeriod) :
    AppTask(name, pLog),
    m_id(id),
    m_pLog(pLog),
    m_type(type),
    m_sampleFreq(sampleFreq),
    m_reportPeriod(reportPeriod),
    m_samplesPerReport(sampleFreq * reportPeriod),
    m_sampleCount(0),
    pDataItem(nullptr)
{
    m_pLog->log(eLOG_DEBUG, "%s : CREATED", GetName().c_str());
}

SensorTask::~SensorTask()
{
    m_pLog->log(eLOG_DEBUG, "%s.%s : DONE", GetName().c_str(), __FUNCTION__);
}

// ****************************************************************************

void SensorTask::Entry()
{
    waitForBeginOperation();

    // ------------------------------------------------
    // TODO Task initialization

    m_pLog->log(eLOG_DEBUG, "%s: BEGIN + Initialization", GetName().c_str());

    double current = 0.0;
    double cumulative = 0.0;
    double newValue = 0.0;

    pDataItem = dynamic_cast<DataItem<double> *>(DataStore::getInstance()->GetDataItem(m_id));

    // Set stale time = 2 x reportPeriod
    pDataItem->setStaleTime(2 * m_reportPeriod * 1000 /* ms */);

    // ------------------------------------------------

    TaskState prevState = GetState();

    while (!isStopped())
    {
        if (prevState != GetState())
        {
            m_pLog->log(eLOG_DEBUG, "%s: %s", GetName().c_str(), GetStateName().c_str());
        }

        // --------------------------------------------
        // TODO Task work - gather sensor data

        // Get sample at sample frequency
        Sleep(1000 / m_sampleFreq);

        // TODO cumulative += value read from hardware

        struct timeval detail_time; // For seconds and microseconds
        gettimeofday(&detail_time, NULL);
        double delta;

        if (m_id == PRESSURE1) {
            delta = (500.0 - (detail_time.tv_usec % 1000)) / 500.0; // rand[-1.0, 1.0]
            current = 15.0 + delta;
        } else if (m_id == FLOW1) {
            delta = (500.0 - (detail_time.tv_usec % 1000)) / 100.0; // rand[-5.0, 5.0]
            current = 25.0 + delta;
        } else if (m_id == TEMP_SENSE_1) {
            delta = (500.0 - (detail_time.tv_usec % 1000)) / 100.0; // rand[-5.0, 5.0]
            current = 37.0 + delta;
        }
        cumulative += current;

        if (++m_sampleCount >= m_samplesPerReport) {
            newValue = cumulative / m_sampleCount;
            pDataItem->setValue(newValue);
            //m_pLog->log(eLOG_DEBUG, "[%d] %s: Report %s - %lg %s",
            //        m_id, GetName().c_str(), m_type.c_str(), newValue, pDataItem->getUnits().c_str() );

            m_sampleCount = 0;
            cumulative = 0.0;
        }
        // --------------------------------------------

        // PAUSED: Must wait to be told to continue.
        if (isPaused())
        {
            // TODO Pause data-gathering timer

            m_pLog->log(eLOG_DEBUG, "--- %s Paused", GetName().c_str());
            waitForContinue();
            m_pLog->log(eLOG_DEBUG, "--- %s Continuing", GetName().c_str());
        }

        prevState = GetState();
    } // end while running

    // ------------------------------------------------
    // TODO Task cleanup before exit
    m_pLog->log(eLOG_DEBUG, "%s.%s : CLEANUP", GetName().c_str(), __FUNCTION__);
    // ------------------------------------------------
}

// ****************************************************************************
