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
 * SensorTask.h
 *
 * Created on: Feb 20, 2020
 * Author: jparziale
 */

#ifndef SENSORTASK_H_
#define SENSORTASK_H_
// ****************************************************************************

#include "Logger.h"
#include "AppTask.h"
#include "DataItem.h"

class SensorTask : public AppTask
{
    // ------------------------------------------------------------------------
public:
    SensorTask(const std::string name,
               DataItemId id,
               Logger* pLog,
               const std::string type,
               double sampleFreq,
               double reportPeriod);
    virtual ~SensorTask();

    // ------------------------------------------------------------------------
private:
    DataItemId m_id;
    Logger* m_pLog;
    std::string m_type;
    double m_sampleFreq;
    double m_reportPeriod;
    int m_samplesPerReport;
    int m_sampleCount;

    DataItem<double> *pDataItem; // XXX

    // Work function of the sensor task.
    void Entry(void) override;
};

// ****************************************************************************
#endif /* SENSORTASK_H_ */
