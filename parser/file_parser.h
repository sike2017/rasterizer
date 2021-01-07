#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include "core/mesh.h"

class FileParser
{
public:
	virtual bool parse(const std::string& path, Mesh** _mesh) = 0;

protected:
	inline bool openFile(const std::string& filename) {
		moduleFile = std::ifstream(filename, std::ios::in);
		if (!moduleFile.is_open()) {
			rlog << "open module file error\n";
			return false;
		}
		return true;
	}
	inline void closeFile() {
		moduleFile.close();
	}

	std::ifstream moduleFile;
};

class ObjParser : public FileParser
{
public:
	ObjParser() : FileParser() {
		table.insert(std::pair<std::string, ParserInstruction>("v", ParserInstruction::v));
		table.insert(std::pair<std::string, ParserInstruction>("f", ParserInstruction::f));
		table.insert(std::pair<std::string, ParserInstruction>("vt", ParserInstruction::vt));
		table.insert(std::pair<std::string, ParserInstruction>("vn", ParserInstruction::vn));
	}
	~ObjParser() {}

	enum class ParserInstruction {
		v,
		f,
		vt,
		vn
	};

	virtual bool parse(const std::string& filename, Mesh** _mesh) {
		if (!openFile(filename)) {
			return false;
		}
		std::string lineStr;
		Mesh* mesh = new Mesh;
		std::vector<Vertex*>& vArray = mesh->vArray;
		std::vector<Point2f*>& vtArray = mesh->vtArray;
		std::vector<Vector3f*>& vnArray = mesh->vnArray;
		std::vector<float> v_vector;
		while (std::getline(moduleFile, lineStr)) {
			std::pair<ParserInstruction, std::vector<float>> lineResult;
			if (lineParse(lineStr, &lineResult)) {
				if (lineResult.first == ParserInstruction::v) {
					Vertex* vertex = new Vertex(Point3f(lineResult.second[0], lineResult.second[1], lineResult.second[2], 1.0f));
					vArray.push_back(vertex);
				}
				else if (lineResult.first == ParserInstruction::f) {
					int vertex_n = 3;
					switch (lineResult.second.size()) {
					case 3:
						// f v v v
						vertex_n = 3;
						for (int index = 0; index < vertex_n; index++) {
							v_vector.push_back(lineResult.second[index]);
							v_vector.push_back(0);
							v_vector.push_back(0);
						}
						vtArray.push_back(new Point2f(0, 0));
						vnArray.push_back(new Vector3f(computeNormal(vArray[lineResult.second[0]-1]->p, vArray[lineResult.second[1]-1]->p, vArray[lineResult.second[2]-1]->p)));
						break;
					case 4:
						// f v v v v
						vertex_n = 4;
						for (int index = 0; index < vertex_n; index++) {
							v_vector.push_back(lineResult.second[index]);
							v_vector.push_back(0);
							v_vector.push_back(0);
						}
						vtArray.push_back(new Point2f(0, 0));
						vnArray.push_back(new Vector3f(computeNormal(vArray[lineResult.second[0]-1]->p, vArray[lineResult.second[1]-1]->p, vArray[lineResult.second[2]-1]->p)));
						break;
					default:
						// f v/vt/vn v/vt/vn v/vt/vn v/vt/vn ....
						vertex_n = lineResult.second.size() / 3;
						v_vector = lineResult.second;
					}
					int triangle_n = vertex_n - 2;
					int v0;
					int v1;
					int v2;
					int v[3];
					int vt[3];
					int vn[3];
					for (int index = 0; index < triangle_n; index++) {
						v0 = 0;
						v1 = 3 * (1 + index);
						v2 = 3 * (2 + index);
						v[0] = v_vector[v0] - 1;
						v[1] = v_vector[v1] - 1;
						v[2] = v_vector[v2] - 1;
						vt[0] = v_vector[v0 + 1] - 1;
						vt[1] = v_vector[v1 + 1] - 1;
						vt[2] = v_vector[v2 + 1] - 1;
						vn[0] = v_vector[v0 + 2] - 1;
						vn[1] = v_vector[v1 + 2] - 1;
						vn[2] = v_vector[v2 + 2] - 1;
						mesh->add(
							new Triangle(Triple<Vertex*>(vArray[v[0]], vArray[v[1]], vArray[v[2]]), Triple<Point2f*>(vtArray[vt[0]], vtArray[vt[1]], vtArray[vt[2]]), Triple<Vector3f*>(vnArray[vn[0]], vnArray[vn[1]], vnArray[vn[2]])));
					}
				}
				else if (lineResult.first == ParserInstruction::vn) {
					vnArray.push_back(new Vector3f(lineResult.second[0], lineResult.second[1], lineResult.second[2], 0.0f));
				}
				else if (lineResult.first == ParserInstruction::vt) {
					vtArray.push_back(new Point2f(lineResult.second[0], lineResult.second[1]));
				}
			}
		}
		*_mesh = mesh;
		closeFile();
		return true;
	}

private:
	inline bool isFloatString(const std::string& str) {
		for (const char& le : str) {
			if (le != '-' && le != '.' && le != '0' && le != '1' && le != '2' && le != '3' && le != '4'
				&& le != '5' && le != '6' && le != '7' && le != '8' && le != '9' && le != '+' && le != 'e' && le != ' ') {
				return false;
			}
		}
		return true;
	}

	template<typename T>
	inline T convertStringToNumber(const std::string& s) {
		std::istringstream istream(s);
		T r;
		istream >> r;
		return r;
	}

	inline bool lineParse(const std::string& str, std::pair<ParserInstruction, std::vector<float>>* out) {
		std::vector<std::string> e;
		std::string temp;
		int index = 0;

		for (const char& l : str) {
			index++;
			if (l != ' ' && index != str.length()) {
				temp.push_back(l);
			}
			else {
				if (index == str.length()) {
					temp.push_back(l);
				}
				if (!temp.empty()) {
					e.push_back(temp);
					temp.clear();
				}
				continue;
			}
		}
		if (e.empty()) {
			return false;
		}

		std::map<std::string, ParserInstruction>::iterator itor = table.find(e[0]);
		if (itor == table.end()) {
			return false;
		}
		bool first;
		int floatCount;
		switch (itor->second) {
		case ParserInstruction::v:
			vLineParse(e, itor, out);
			break;
		case ParserInstruction::f:
-			fLineParse(e, itor, out);
			break;
		case ParserInstruction::vt:
			first = true;
			floatCount = 0;
			for (const std::string& l : e) {
				if (first) {
					first = false;
					continue;
				}
				if (floatCount == 2) {
					break;
				}
				if (!isFloatString(l)) {
					return false;
				}
				out->second.push_back(convertStringToNumber<float>(l));
				floatCount++;
			}
			out->first = itor->second;
			break;
		case ParserInstruction::vn:
			first = true;
 			floatCount = 0;
			for (const std::string& l : e) {
				if (first) {
					first = false;
					continue;
				}
				if (floatCount == 3) {
					break;
				}
				if (!isFloatString(l)) {
					return false;
				}
				out->second.push_back(convertStringToNumber<float>(l));
				floatCount++;
			}
			out->first = itor->second;
			break;
		}

		return true;
	}

	inline bool vLineParse(const std::vector<std::string>& e, const std::map<std::string, ParserInstruction>::iterator& itor, std::pair<ParserInstruction, std::vector<float>>* out) {
		bool first = true;
		int floatCount = 0;
		for (const std::string& l : e) {
			if (first) {
				first = false;
				continue;
			}
			if (floatCount == 3) {
				break;
			}
			if (!isFloatString(l)) {
				return false;
			}
			out->second.push_back(convertStringToNumber<float>(l));
			floatCount++;
		}
		out->first = itor->second;
		return true;
	}

	inline bool fLineParse(const std::vector<std::string>& e, const std::map<std::string, ParserInstruction>::iterator& itor, std::pair<ParserInstruction, std::vector<float>>* out) {
		bool first = true;
		int vCount = 0;
		for (const std::string& l : e) {
			if (first) {
				first = false;
				continue;
			}
			std::string vString;
			int index = 0;
			for (const char& tl : l) {
				index++;
				if (tl != '/') {
					if (tl != '0' && tl != '1' && tl != '2' && tl != '3' && tl != '4'
						&& tl != '5' && tl != '6' && tl != '7' && tl != '8' && tl != '9' && tl != ' ') {
						return false;
					}
					else {
						vString += tl;
						if (index == l.size()) {
							out->second.push_back(convertStringToNumber<int>(vString));
							out->first = itor->second;
							vString.clear();
						}
					}
				}
				else {
					out->second.push_back(convertStringToNumber<int>(vString));
					vString.clear();
				}
			}
		}
		return true;
	}

	std::map<std::string, ParserInstruction> table;
};
