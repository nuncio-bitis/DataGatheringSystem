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
 * DataStore.h
 *
 *  Created on: Dec 31, 2020
 *      Author: jparz
 */

#ifndef DATASTORE_H_
#define DATASTORE_H_
// ****************************************************************************

#include "Logger.h"
#include "DataItem.h"

#include <vector>

// ****************************************************************************

// IDs of all data items in the system.
enum DataItemId {
    CPU_MEM_FREE,   // /proc/meminfo (MemFree:)
    CPU_TEMP,       // Pi: /opt/vc/bin/vcgencmd measure_temp ; Linux: /sys/class/thermal/thermal_zone0/temp
    TEMP_SENSE_1,   // /dev/i2c-1, address 0x48 (ADS1115)
    TEMP_SENSE_2,
    PRESSURE1,
    PRESSURE2,
    FLOW1,
    FLOW2,
    NUM_DATA_ITEMS
};

// ****************************************************************************

class DataStore : public std::enable_shared_from_this<DataStore>
{
    // ------------------------------------------------------------------------
public:
    DataStore();
    virtual ~DataStore();

    static DataItemPublisher* GetDataItem(DataItemId id) { return m_DataItemList[id]; };

    static DataStore* getInstance() { return s_pInstance; };

    // ------------------------------------------------------------------------
private:
    static DataStore* s_pInstance;

    // List of all data items
    static std::vector<DataItemPublisher*> m_DataItemList;
};

// ****************************************************************************
#endif /* DATASTORE_H_ */
