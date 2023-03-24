#ifndef BCC_H
#define BCC_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <vector>
#include <iostream>

struct BCCHeader{
    char sign[3];
    unsigned char byteCount;
    char    curveType[2],
            dimensions,
            upDimension;
    uint64_t curveCount,
             totalControlPointCount;
    char fileInfo[40];
};

void readBCC(const std::string& filename,std::vector<std::vector<glm::vec3>>& closedFibersCP,std::vector<std::vector<glm::vec3>>& openFibersCP){
    BCCHeader header;
    closedFibersCP.clear();
    openFibersCP.clear();
    bool openIsEmpty = true, closedIsEmpty = true;
    FILE *pFile = fopen(filename.c_str(),"rb");
    if( pFile == NULL ){
        printf("Impossible to open the file !\n");
    }
    fread(&header, sizeof(header), 1, pFile);
    if (header.sign[0] != 'B' ||
        header.sign[1] != 'C' ||
        header.sign[2] != 'C' ||
        header.byteCount != 0x44 ){std::cout<<"Invalid BCC format !"<<std::endl;}
    if (header.curveType[0] != 'C' ){std::cout<<"Invalid Curve type !"<<std::endl;}
    if (header.curveType[1] != '0' ){std::cout<<"Invalid Curve parametrisation!"<<std::endl;}
    if (header.dimensions != 3 ){std::cout<<"Invalid number of dimensions !"<<std::endl;}
    std::cout<<header.curveCount<<std::endl;
    for (uint64_t id = 0 ; id < header.curveCount ; id++){
        int nbCP;
        fread(&nbCP,sizeof(int),1,pFile);
        if (nbCP < 0){
            nbCP = -nbCP;
            std::vector<glm::vec3> curveData(nbCP);
            for (uint64_t cpid = 0 ; cpid < nbCP ; cpid ++ ){
                glm::vec3 point;
                fread(&point,sizeof(float),3,pFile);
                //std::cout<<point.x<<" "<<point.y<<" "<<point.z<<std::endl;
                curveData[cpid]=point;
            }
            closedFibersCP.emplace_back(curveData);
            std::cout << "Curve nb "<<id<<" should have "<<nbCP<<" control points and has "<<curveData.size()<<" control points."<<std::endl;
        }else{
            std::vector<glm::vec3> curveData(nbCP);
            for (uint64_t cpid = 0 ; cpid < nbCP ; cpid ++ ){
                glm::vec3 point;
                fread(&point,sizeof(float),3,pFile);
                //std::cout<<point.x<<" "<<point.y<<" "<<point.z<<std::endl;
                curveData[cpid]=point;
            }
            openFibersCP.emplace_back(curveData);
            std::cout << "Curve nb "<<id<<" should have "<<nbCP<<" control points and has "<<curveData.size()<<" control points."<<std::endl;
        }
    }
    std::cout << "Curves count - open : "<<openFibersCP.size()<<" , closed : "<<closedFibersCP.size()<<std::endl;
}

#endif