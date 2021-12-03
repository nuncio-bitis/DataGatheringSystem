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
 * UITask.h
 *
 * Created on: Feb 21, 2020
 * Author: jparziale
 */

#ifndef UITASK_H_
#define UITASK_H_
// ****************************************************************************

#include <stdint.h>
#include <string>

#include "Logger.h"
#include "AppTask.h"
#include "DataItem.h"

class UITask : public AppTask, public DataItemSubscriber
{
    // ------------------------------------------------------------------------
public:
    UITask(const std::string name, int id, Logger* pLog /* TODO pass in link to data store */);
    virtual ~UITask();

    void DataItemUpdated(int id) override;

    // ------------------------------------------------------------------------
private:
    // Work function of the sensor task.
    void Entry(void) override;

    void UpdateItem(DataItem<uint64_t> *item);
    void UpdateItem(DataItem<double> *item);

    int m_id;
    Logger* m_pLog;

    // Pointers to data store objects
    DataItem<uint64_t> *cpu_mem_free;
    DataItem<double> *cpu_temp;
    DataItem<double> *temp_1;
    DataItem<double> *temp_2;
    DataItem<double> *press_1;
    DataItem<double> *press_2;
    DataItem<double> *flow_1;
    DataItem<double> *flow_2;

    long cpu_mem_free_tok;
    long cpu_temp_tok;
    long temp_1_tok;
    long temp_2_tok;
    long press_1_tok;
    long press_2_tok;
    long flow_1_tok;
    long flow_2_tok;
};

// ****************************************************************************
#endif /* UITASK_H_ */
