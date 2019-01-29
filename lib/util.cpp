#include "util.hpp"
#include <sstream>

namespace util {

    void minmax(float * array, int len, int stride, float * outMin, float * outMax) {
        float min = std::numeric_limits<float>::max();
        float max = - std::numeric_limits<float>::max();
        for(int i = 0; i < len; i += stride) {  
            if( array[i] < min ) {
                min = array[i];
            }
            else if( array[i] > max ) {
                max = array[i];
            }
        } 
        *outMin = min;
        *outMax = max;
    }

    void normalize(float * array, int len, int stride) {
        float min, max;
        minmax(array, len, stride, &min, &max);
        float range = max - min;
        for(int i = 0; i < len; i += stride) {
            array[i] = ( array[i] - min ) / range; // normalize to 0...1
            array[i] = 2*array[i] - 1; // normalize to -1...1
        }
    }

    void normalizeModel(Model * model) {
        normalize(model->vertexArray, 3 * model->numVertices, 3);
        normalize(model->vertexArray + 1, 3 * model->numVertices, 3);
        normalize(model->vertexArray + 2, 3 * model->numVertices, 3);
    }

    Model * quad() {
        GLfloat quadVerts[] = {
            -1,-1,0,
            1,-1, 0,
            1,1, 0,
            -1,1, 0};
        GLfloat quadTexCoords[] = {
            0, 0,
            1, 0,
            1, 1,
            0, 1};
        GLuint quadInds[] = {0, 1, 2, 0, 2, 3};
        Model * quad;
        quad = LoadDataToModel(
                quadVerts, NULL, quadTexCoords, NULL,
                quadInds, 4, 6);
        return quad;
    }

    Model * cube() {
        Model * model = LoadModelPlus("models/cube.obj");
        CenterModel(model);
        ReloadModelData(model);
        return model;
    }

    Model * ball(float scale, int number) {
        std::stringstream ss;
        ss << "models/sphere" << number << ".obj";
        Model * model = LoadModelPlus(ss.str().c_str());
        CenterModel(model);
        ScaleModel(model, scale, scale, scale);
        ReloadModelData(model);
        return model;
    }

    Model * lattice(int resolution) {
        std::vector<glm::vec3> vertices;
        vertices.reserve(resolution * resolution * resolution);
        GLfloat vx,vy,vz;
        for(int x = 0; x < resolution; ++x) {
            for(int y = 0; y < resolution; ++y) {
                for(int z = 0; z < resolution; ++z) {
                    vx = 2*(static_cast<GLfloat>(x) / resolution) - 1;
                    vy = 2*(static_cast<GLfloat>(y) / resolution) - 1;
                    vz = 2*(static_cast<GLfloat>(z) / resolution) - 1;
                    vertices.push_back( glm::vec3( vx, vy, vz ) );
                }
            }
        }
        return LoadPointCloud( reinterpret_cast<GLfloat*>(&vertices[0]), vertices.size() );
    }

    Model * model(const std::string & path) {
        Model * model = LoadModelPlus(path.c_str());
        //CenterModel(model);
        normalizeModel(model);
        ReloadModelData(model);
        return model;
    }   

    float rad2deg(float rad) {
        return 180 * rad / M_PI;
    }

    float deg2rad(float deg) {
        return deg * M_PI / 180;
    }

    float clamp(float val, float min, float max) {
        if( val < min ) {
            return min;
        }
        if( val > max ) {
            return max;
        }
        return val;
    }

}
