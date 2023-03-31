#ifndef BCC_H
#define BCC_H

#include "Logging.h"
#include "VertexArray.h"
#include "VertexBuffer.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <vector>
#include <iostream>
#include <filesystem>


namespace fs = std::filesystem;


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


void LoadBCCFile(const std::string& filePath, std::vector<glm::vec3>& controlPoints, std::vector<uint32_t>& indices)
{
    std::vector<std::vector<glm::vec3>> closedFibersCP;
    std::vector<std::vector<glm::vec3>> openFibersCP;
    readBCC(filePath, closedFibersCP, openFibersCP);

    controlPoints.clear();
    indices.clear();

    // Merge all the curves into a single vector to draw all of them in a single drawcall
    // This need to be replaced by the proper loading of the fiber data
    for (const auto& fiber : closedFibersCP)
    {
        for (const auto& cPoints : fiber)
            controlPoints.push_back(cPoints);
        controlPoints.push_back(fiber.front());
    }
    for (const auto& fiber : openFibersCP)
        for (const auto& cPoints : fiber)
            controlPoints.push_back(cPoints);

    uint32_t vertexCount = controlPoints.size();

    for (size_t i = 0 ; i < vertexCount - 3 ; i++)
    {
        indices.push_back(i);
        indices.push_back(i+1);
        indices.push_back(i+2);
        indices.push_back(i+3);
    }
}


VertexArrayPtr LoadBCCToOpenGL(const std::vector<glm::vec3>& controlPoints, const std::vector<uint32_t>& indices)
{
    // Send the fibers data to OpenGL
    auto vertexBuffer = VertexBuffer::Create(controlPoints.data(), 
                                             controlPoints.size() * sizeof(glm::vec3));
    vertexBuffer->SetLayout({{"Position",  3, GL_FLOAT, false}});
    auto indexBuffer = IndexBuffer::Create(indices.data(), 
                                                 indices.size());
    auto vertexArray = VertexArray::Create();
    vertexArray->Bind();
    vertexArray->AddVertexBuffer(vertexBuffer);
    vertexArray->SetIndexBuffer(indexBuffer);
    vertexArray->Unbind();

    return vertexArray;
}


std::vector<fs::path> ListBCCFiles(const fs::path& directory)
{
    std::vector<fs::path> result; 
    for (const auto& entry : fs::directory_iterator(directory))
    {
        if (entry.path().extension() == ".bcc")
        {
            result.push_back(entry.path());
        }
    }

    return result;
}




#endif  // BCC_H