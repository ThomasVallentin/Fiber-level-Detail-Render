#ifndef BCC_H
#define BCC_H

#include "Logging.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <vector>
#include <iostream>

struct BCCHeader
{
    char sign[3];
    unsigned char byteCount;
    char curveType[2],
         dimensions,
         upDimension;
    uint64_t curveCount,
             totalControlPointCount;
    char fileInfo[40];
};

void readBCC(const std::string& filename, std::vector<std::vector<glm::vec3>>& closedFibersCP, std::vector<std::vector<glm::vec3>>& openFibersCP)
{
    BCCHeader header;
    closedFibersCP.clear();
    openFibersCP.clear();
    bool openIsEmpty = true, closedIsEmpty = true;
    FILE *pFile = fopen(filename.c_str(),"rb");
    if( pFile == NULL )
    {
        LOG_ERROR("Impossible to open the file %s !", filename.c_str());
    }

    fread(&header, sizeof(header), 1, pFile);
    if (header.sign[0] != 'B' ||
        header.sign[1] != 'C' ||
        header.sign[2] != 'C' ||
        header.byteCount != 0x44 )   { LOG_ERROR("Invalid BCC format !"); }
    if (header.curveType[0] != 'C' ) { LOG_ERROR("Invalid Curve type !"); }
    if (header.curveType[1] != '0' ) { LOG_ERROR("Invalid Curve parametrisation!"); }
    if (header.dimensions != 3 )     { LOG_ERROR("Invalid number of dimensions !"); }

    for (uint64_t id = 0 ; id < header.curveCount ; id++)
    {
        int nbCP;
        fread(&nbCP,sizeof(int),1,pFile);
        if (nbCP < 0)
        {
            nbCP = -nbCP;
            std::vector<glm::vec3> curveData(nbCP);
            for (uint64_t cpid = 0 ; cpid < nbCP ; cpid ++ )
            {
                glm::vec3 point;
                fread(&point,sizeof(float),3,pFile);
                curveData[cpid]=point;
            }
            closedFibersCP.emplace_back(curveData);
        }
        else
        {
            std::vector<glm::vec3> curveData(nbCP);
            for (uint64_t cpid = 0 ; cpid < nbCP ; cpid ++ )
            {
                glm::vec3 point;
                fread(&point,sizeof(float),3,pFile);
                curveData[cpid]=point;
            }
            openFibersCP.emplace_back(curveData);
        }
    }
    
    LOG_INFO("Successfully loaded %d open curves and %d closed curves", openFibersCP.size(), closedFibersCP.size());
}

#endif  // BCC_H