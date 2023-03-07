#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), faces_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        std::string trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) {
                iss >> v.raw[i];
            }
            verts_.push_back(v);
        } else if (!line.compare(0, 4, "vt  ")) {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) {
                iss >> v.raw[i];
            }
            vertTextures_.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            std::vector<std::vector<int>> f;
            int idx, idy, idz;
            char ctrash;
            iss >> ctrash;
            while (iss >> idx >> ctrash >> idy >> ctrash >> idz) {
                // idx--; // in wavefront obj all indices start at 1, not zero
                // idy--;
                // idz--;
                std::vector<int> faceVertex{ --idx, --idy, --idz };
                f.push_back(faceVertex);
            }
            faces_.push_back(f);
        }
    }
    std::cerr << "# v# " << verts_.size() << " vt# " << vertTextures_.size() << " f# "  << faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nvertTextures() {
    return (int)vertTextures_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

std::vector<std::vector<int>> Model::face(int idx) {
    return faces_[idx];
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

Vec3f Model::vertTexture(int i) {
    return vertTextures_[i];
}

