#include "obj_parser.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>
#include <cmath>
#include <algorithm>
using namespace std;

inline bool is_space(char c) {
  return c == ' ';
}

vector<primitive*> obj_parser::parse() {
  ifstream f(filename);
  if (f.fail()) {
    cerr << "File not found.\n";
    return vector<primitive*>();
  }
  
  vertices.reserve(1000);
  normals.reserve(1000);
  primitives.reserve(1000);

  string buf;
  
  while (getline(f, buf)) {
    // skip spaces in the front
    string line(find_if_not(buf.begin(), buf.end(), is_space), buf.end());
    if (line.length() == 0) continue;
    if (line[0] == 'v') {
      double x, y, z;
      sscanf(&line[2], "%lf%lf%lf", &x, &y, &z);
      vertices.emplace_back(x, y, z);
    } else if (line[0] == 'v') {
      if (line[1] == 'n') {
        double x, y, z;
        sscanf(&line[2], "%lf%lf%lf", &x, &y, &z);
        normals.emplace_back(x, y, z);
      } else if (line[0] == 't') {}
      // texture not implemented
    } else if (line[0] == 'f') {
      if (!parse_face(move(line))) {
        return vector<primitive*>();
      }
    }
  }
  f.close();
  cout << "Reading obj complete, created " << primitives.size() << " primitives.\n";
  return primitives;
}

bool obj_parser::parse_face(string &&line) {
  static int length = 0;
  static stringstream ss;
  ss.clear();
  ss.str(line);
  string word;
  ss >> word;
  if (word != "f") {
    cerr << "Invalid paramter.\n";
    return false;
  }
  vector<unsigned int> indices;
  indices.reserve(10);
  while (!ss.eof()) {
    ss >> word;
    if (!parse_face_index(word.c_str(), indices, length)) {
      return false;
    }
  }
  if (length == 3) {
    for (int i = 0; i < indices.size() - 5; i += 6) {
      vec3 A = trim_to_vec3(trans * expand_to_vec4(vertices[indices[i]]));
      vec3 B = trim_to_vec3(trans * expand_to_vec4(vertices[indices[i+2]]));
      vec3 C = trim_to_vec3(trans * expand_to_vec4(vertices[indices[i+4]]));
      vec3 n1 = trim_to_vec3(trans * expand_to_vec4(vertices[indices[i+1]])).unit();
      vec3 n2 = trim_to_vec3(trans * expand_to_vec4(vertices[indices[i+3]])).unit();
      vec3 n3 = trim_to_vec3(trans * expand_to_vec4(vertices[indices[i+5]])).unit();
      primitives.push_back(new triangle(A, B, C, n1, n2, n3, ka, kd, ks, kr));
    }
  } else {
    for (int i = 0; i < indices.size() - 2; i += 3) {
      vec3 A = trim_to_vec3(trans * expand_to_vec4(vertices[indices[i]]));
      vec3 B = trim_to_vec3(trans * expand_to_vec4(vertices[indices[i+1]]));
      vec3 C = trim_to_vec3(trans * expand_to_vec4(vertices[indices[i+2]]));
      primitives.push_back(new triangle(A, B, C, ka, kd, ks, kr));
    }
  }
  return true;
}

bool obj_parser::parse_face_index(const char *line, vector<unsigned int> &indices, int &length) {
  unsigned int vertex, texture, normal = -1;
  if (length == 0) {
    // first use, determine face data format  1 = v/t   2 = v//n  3 = v/t/n
    length = sscanf(line, "%ud%*[/]%ud/%ud", &vertex, &texture, &normal);
    if (length == 2 && normal == static_cast<unsigned int>(-1)) {
      length = 1;
    }
  }
  
  if (length == 1) {
    sscanf(line, "%ud", &vertex);
  } else if (length == 2) {
    sscanf(line, "%ud%ud", &vertex, &normal);
  } else {
    sscanf(line, "%ud%ud%ud", &vertex, &texture, &normal); // texture not implemented
  }
  if ((vertex > vertices.size()) || (length > 1 && normal > normals.size())) {
    cerr << "Obj file f index out of bound.\n";
    return false;
  }
  indices.push_back(vertex-1);
  if (length > 2) {
    indices.push_back(normal-1);
  }
  return true;
}
