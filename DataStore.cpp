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
 * DataStore.cpp
 *
 *  Created on: Dec 31, 2020
 *      Author: jparz
 */
// ****************************************************************************

#include "DataStore.h"
#include <iostream>
#include <cstdint>

// ****************************************************************************

DataStore* DataStore::s_pInstance = nullptr;

// List of data items. Initialize its size with the number of sensors.
std::vector<DataItemPublisher*> DataStore::m_DataItemList(NUM_DATA_ITEMS);

// ****************************************************************************

DataStore::DataStore() {
    s_pInstance = this;

    // Create data items. They will be referenced by index, so create them by index.
    m_DataItemList[CPU_MEM_FREE] =
        new DataItem<uint64_t>("CPU_Mem_Free", "bytes", CPU_MEM_FREE, 0, 0, ((uint64_t)8*1024*1024*1024), 5000);
    m_DataItemList[CPU_TEMP]     = new DataItem<double>("CPU_Temp", "︒C", CPU_TEMP, 0, -40.0, 80.0, 5000);

    m_DataItemList[TEMP_SENSE_1] = new DataItem<double>("Fluid_Temp", "︒C", TEMP_SENSE_1, 0, -40.0, 80.0, 5000);
    m_DataItemList[TEMP_SENSE_2] = new DataItem<double>("Temp_2", "︒C", TEMP_SENSE_2, 0, -40.0, 80.0, 5000);
    m_DataItemList[PRESSURE1]    = new DataItem<double>("Gas_Pressure", "PSI", PRESSURE1, 0, 0.0, 250.0, 3000);
    m_DataItemList[PRESSURE2]    = new DataItem<double>("Pressure_2", "mmHg", PRESSURE2, 0, 0.0, 250.0, 3000);
    m_DataItemList[FLOW1]        = new DataItem<double>("Gas_Flow_Rate", "l/hr", FLOW1, 0, 0.0, 1000.0, 2000);
    m_DataItemList[FLOW2]        = new DataItem<double>("Flow_2", "ml/hr", FLOW2, 0, 0.0, 1000.0, 2000);
}

DataStore::~DataStore() {

    // Delete all created data items
    for (auto p : m_DataItemList) {
        delete p;
    }
}

//******************************************************************************
