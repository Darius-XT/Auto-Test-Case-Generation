#ifndef GRAPH_STRUCTURES_H
#define GRAPH_STRUCTURES_H

#include <iostream>
#include <bits/stdc++.h>
#include <sstream>
#include <regex>
#include <unordered_map>
#include <z3++.h>
#include <chrono>
#include "httplib.h"     // HTTP��
#include <nlohmann/json.hpp> // JSON��
#include <windows.h>
#include <filesystem>

using namespace std;
using json = nlohmann::json;

// ������������
constexpr double positiveInfinity = numeric_limits<double>::infinity();
constexpr double negativeInfinity = -numeric_limits<double>::infinity();

// �������ݷ�Χ�ṹ
struct RANGE {
    double lb = negativeInfinity; // �½�
    double ub = positiveInfinity; // �Ͻ�
    bool is_lb_closed = false;    // �½��Ƿ�պ�
    bool is_ub_closed = false;    // �Ͻ��Ƿ�պ�
    double not_equal_to = positiveInfinity; // ������ĳֵ

    RANGE(double lowerBound = negativeInfinity,
        double upperBound = positiveInfinity,
        bool lowerBoundClosed = false,
        bool upperBoundClosed = false,
        double notEqualTo = positiveInfinity)
        : lb(lowerBound), ub(upperBound), is_lb_closed(lowerBoundClosed), is_ub_closed(upperBoundClosed), not_equal_to(notEqualTo) {}

    void print() {
        if (lb != ub) {
            if (not_equal_to != positiveInfinity)
                cout << "  Data Value: !=" << not_equal_to << endl;
            else
                cout << "  Data Range: "
                << (is_lb_closed ? "[" : "(") << lb << ", "
                << ub << (is_ub_closed ? "]" : ")") << endl;
        }
        else {
            cout << "  Data Value:" << ub << endl;
        }
    }
};

// ���ݽṹ
struct DATA {
    string name;
    string data_type;
    RANGE range;

    void clear() {
        range.lb = negativeInfinity;
        range.ub = positiveInfinity;
        range.is_lb_closed = false;
        range.is_ub_closed = false;
        range.not_equal_to = positiveInfinity;
    }
};

// �ֵ����ݽṹ
struct DIC_DATA {
    string name;
    string type = "in";
    RANGE theoretical_range;
    vector<double> possibleValues;
};

// �ڵ�ṹ
struct NODE {
    string node_id;
    string node_type;
    string node_information;
    vector<DATA> node_datas;
    string branch_no;
    string branch_yes;

    void clear() {
        node_id = "";
        node_type = "";
        node_information = "";
        branch_no = "";
        branch_yes = "";
        node_datas.clear();
    }
};

// �߽ṹ
struct EDGE {
    string edge_id;
    string source_node_id;
    string target_node_id;
    string edge_information;
    vector<DATA> edge_datas;

    void clear() {
        edge_id = "";
        source_node_id = "";
        target_node_id = "";
        edge_information = "";
        edge_datas.clear();
    }
};

struct PATH { // ·��
    string path_id; //·��ID
    vector<NODE> path_nodes; //·���нڵ㼯��
    vector<EDGE> path_edges; //·���б߼���
};

struct GRAPH { // ����ͼ
    string graph_id = "none"; // ����ͼID
    vector<PATH> paths; // ·������
    vector<NODE> graph_nodes; // ͼ�нڵ㼯��
    vector<EDGE> graph_edges; // ͼ�б߼���
    vector<string> dependent_graph_id; // ������ͼid���ϣ����洢����ͼ��id�����洢���������������ݵķ�Χ�������ж������Ƿ�ɻ�Ҫ��������Ϲ���ͼ���ж�
    int pathIdCounter = 0; // ��id������

    //������
    void clear() {
        graph_id = "none";
        paths.clear();
        graph_nodes.clear();
        graph_edges.clear();
        dependent_graph_id.clear();
        pathIdCounter = 0;
    }

    // ͨ��id���ؽڵ�
    NODE& getNodeById(const string& node_id) {
        for (NODE& node : this->graph_nodes) {
            if (node.node_id == node_id) {
                return node;
            }
        }
        throw runtime_error("Node id not found.");
    }

    // ͨ���ڵ�id�����Խڵ�ΪԴ��ı�
    vector<EDGE> getEdgesByNodeId(const string& node_id) {
        vector<EDGE> edges;
        for (const EDGE& edge : this->graph_edges) {
            if (edge.source_node_id == node_id) {
                edges.push_back(edge);
            }
        }
        return edges;
    }

    //�жϽڵ��Ƿ���·�������·�������ú�����
    bool isNodeInPath(NODE& node, PATH& path) {
        for (NODE& existingNode : path.path_nodes) {
            if (existingNode.node_id == node.node_id)
                return true;
        }
        return false;
    }

    // �жϽڵ��Ƿ��г��˻��ڱ������������
    bool hasExternalOutEdge(NODE& node, PATH& currentPath) {
        std::vector<EDGE> outgoingEdges = getEdgesByNodeId(node.node_id);
        for (EDGE& edge : outgoingEdges) {
            // ����ߵ�Ŀ��ڵ㲻�ڵ�ǰ·���У����ڻ��Ĳ����У�����Ϊ���ⲿ����
            if (!isNodeInPath(getNodeById(edge.target_node_id), currentPath)) {
                return true;
            }
        }
        return false;
    }

    // ��һ��ͼ����·��
    void generatePathRecursively(NODE& currentNode, PATH& currentPath) {
        // ��鵱ǰ�ڵ��Ƿ�����·���У����ڣ����ʾ���ڻ�
        //std::ofstream file(output_loc, std::ios::app);  // ��������׷��ģʽ���ļ�
        //if (!file) {
        //    std::cerr << "�޷����ļ� output.txt ����д��";
        //    return;
        //}

        if (isNodeInPath(currentNode, currentPath)) { // ����л�����ǰ�ڵ��Ѿ���·���
            currentPath.path_id = graph_id + "_" + std::to_string(++pathIdCounter) + "_cycle";
            paths.push_back(currentPath);

            // �ж��Ƿ������ѭ��
            bool hasExternalEdge = false;
            int startIndex = 0;
            for (int i = 0; i < currentPath.path_nodes.size(); ++i) {
                if (currentPath.path_nodes[i].node_id == currentNode.node_id) {
                    startIndex = i; // �ҵ�����ʼ�Ľڵ�����
                    break;
                }
            }

            // ��黷�е�ÿ���ڵ�
            for (int i = startIndex; i < currentPath.path_nodes.size(); ++i) {
                if (hasExternalOutEdge(currentPath.path_nodes[i], currentPath)) {
                    hasExternalEdge = true;
                    break;
                }
            }
            //if (hasExternalEdge == false) {
            //    file << "����ѭ����ͼID: " << graph_id
            //        << ",  ���³ɻ��Ľڵ�ID: " << currentNode.node_id << endl;
            //}
            return; // ���ڻ����������εݹ�
        }

        // ����ǰ�ڵ���ӵ���ǰ·���Ľڵ㼯����
        currentPath.path_nodes.push_back(currentNode);

        // ��鵱ǰ�ڵ��Ƿ�Ϊ�յ㣨û�г��ȵĽڵ㣩
        std::vector<EDGE> outgoingEdges = getEdgesByNodeId(currentNode.node_id);
        if (outgoingEdges.empty()) {
            // ������յ㣬��Ϊ��ǰ·������Ψһ�� path_id
            currentPath.path_id = graph_id + "_" + std::to_string(++pathIdCounter);
            // ����ǰ·�������·��������
            paths.push_back(currentPath);
        }
        else {
            // ��������յ㣬�����ÿ������
            for (EDGE& edge : outgoingEdges) {
                // �����߱������ǰ·���ı߼�����
                currentPath.path_edges.push_back(edge);
                // �ݹ������һ���ڵ�
                generatePathRecursively(getNodeById(edge.target_node_id), currentPath);
                // ���ݣ��Ƴ���ǰ�ߣ��Գ�������·��
                currentPath.path_edges.pop_back();
            }
        }
        // ���ݣ��Ƴ���ǰ�ڵ�
        currentPath.path_nodes.pop_back();
    }

    // ����ȫ��·��
    void generateAllPaths() {
        // �������·��
        paths.clear();

        // ����Ƿ�����ʼ�ڵ�
        if (!graph_nodes.empty()) {
            // ��һ���ڵ���Ϊ��ʼ�ڵ�
            NODE& startNode = graph_nodes.front();
            PATH initialPath;  // ����һ����ʼ·�����������κνڵ�ͱ�
            generatePathRecursively(startNode, initialPath);  // ��������·��
        }
    }

    // ���ݽڵ㣬���ؽڵ��ȫ�����ߡ�������֧ͬʱ������ڲ�����
    vector<EDGE> get_out_edges_by_node(const NODE& node) const {
        vector<EDGE> out_edges;
        for (const auto& edge : this->graph_edges) {
            if (edge.source_node_id == node.node_id) {
                out_edges.push_back(edge);
            }
        }
        return out_edges;
    }
};

struct DATA_NEW {
    string name; // ��������
    string data_type; //�������ͣ�read/write��
    vector<RANGE> range; // ��Χ
};

// ����ΪһЩutils����

// ���ַ���A��ȥ����B��xxx�����ַ��������У��Լ�֮ǰ��һ���ַ�����and/or�����У�
void removePattern(string& A, const string& B) {
    // �޸�������ʽ���Ը������������
    // 1. ��� B ǰ�зǿհ��ַ��Ϳհ��ַ�������ƥ����Ƴ���
    // 2. ƥ�� B ���������ڵ����ݡ�
    // 3. ��� B �����ǿո���ζ�ź�������������ַ�����ʽ������ô����ո�Ҳ�ᱻ�Ƴ���
    // \s* ƥ�����������Ŀհ��ַ���ȷ���Ƴ�ǰ��Ŀո�
    string pattern = "\\s*(\\S+\\s+)?" + B + "\\([^()]*\\)\\s*";
    regex re(pattern);

    // ʹ�� regex_replace �����Һ��滻ƥ���ģʽ
    A = regex_replace(A, re, "");
}

// ��ָ��Ŀ¼���ļ�����ȥ��
void deduplicate(string path) {
    set<std::string> lines;  // ʹ��set���Զ�ȥ�أ���Ϊset���������ظ�Ԫ��
    ifstream file_in(path);  // ���ļ����ж�ȡ
    string line;

    if (!file_in) {
        std::cerr << "�޷����ļ����ж�ȡ" << std::endl;
        return;
    }

    // ��ȡ�ļ���ÿһ�У�����洢�ڼ�����
    while (getline(file_in, line)) {
        lines.insert(line);
    }

    file_in.close();  // ��ɶ�ȡ��ر��ļ�

    std::ofstream file_out(path);  // �ٴδ��ļ�����д�룬���������ļ���д���µ����ظ�����

    if (!file_out) {
        std::cerr << "�޷����ļ�����д��" << std::endl;
        return;
    }

    // �������ϲ������д���ļ�
    for (const auto& unique_line : lines) {
        file_out << unique_line << std::endl;
    }

    file_out.close();  // ���д���ر��ļ�

    return;
}

// �ж�һ���ַ����ǲ�������
bool isNumber(const std::string& str) {
    bool decimalPointSeen = false;
    bool digitSeen = false;
    bool minusSignSeen = false;

    return !str.empty() && std::all_of(str.begin(), str.end(), [&](unsigned char c) {
        if (std::isdigit(c)) {
            digitSeen = true;
            return true;
        }
        else if (c == '.' && !decimalPointSeen) {
            decimalPointSeen = true;
            return true;
        }
        else if (c == '-' && !minusSignSeen && str.find(c) == 0) {
            minusSignSeen = true;
            return true;
        }
        else if (isspace(c)) {
            return true; // ����ո�
        }
        else {
            return false;
        }
        }) && digitSeen; // ȷ��������һ������
}

// ����������Χ���ж��Ƿ��н���
bool rangesIntersect(const RANGE& range1, const RANGE& range2) {
    // ���ȣ�����Ƿ���� not_equal_to �����
    auto checkNotEqualTo = [](const RANGE& r1, const RANGE& r2) {
        if (r1.not_equal_to != std::numeric_limits<double>::infinity()) {
            if ((r2.is_lb_closed && r2.lb == r1.not_equal_to) ||
                (r2.is_ub_closed && r2.ub == r1.not_equal_to)) {
                return false;
            }
        }
        return true;
        };

    if (!checkNotEqualTo(range1, range2) || !checkNotEqualTo(range2, range1)) {
        return false;
    }

    // ����Ƿ���ڲ��ص��ĳ���
    if ((range1.ub < range2.lb) ||
        (range1.ub == range2.lb && !(range1.is_ub_closed && range2.is_lb_closed))) {
        return false;
    }

    if ((range2.ub < range1.lb) ||
        (range2.ub == range1.lb && !(range2.is_ub_closed && range1.is_lb_closed))) {
        return false;
    }

    return true; // Ĭ����Ϊ�н���
}

// �ϲ�����RANGE���󲢼���
RANGE unionOfRanges(vector<RANGE>& ranges) {
    // ����Ƿ������� not_equal_to �����
    for (const auto& range : ranges) {
        if (range.not_equal_to != std::numeric_limits<double>::infinity()) {
            double neq = range.not_equal_to;

            for (const auto& range2 : ranges) {
                if (range2.not_equal_to != std::numeric_limits<double>::infinity()) {
                    if (range2.not_equal_to != range.not_equal_to) {
                        return RANGE(); // ����ȫ��ֵ��ķ�Χ
                    }
                }
                else {
                    if (range2.lb < neq || (range2.lb == neq && range2.is_lb_closed)) {
                        return RANGE(); // ����ȫ��ֵ��ķ�Χ
                    }
                }
            }
            return range; // ���ذ��� not_equal_to �ķ�Χ
        }
    }

    // ���û������ not_equal_to �����
    double min_lb = std::numeric_limits<double>::infinity();
    double max_ub = -std::numeric_limits<double>::infinity();
    bool lb_closed = false;
    bool ub_closed = false;

    // �ҳ���С���½�������Ͻ�
    for (const auto& range : ranges) {
        if (range.lb < min_lb) {
            min_lb = range.lb;
            lb_closed = range.is_lb_closed;
        }
        else if (range.lb == min_lb && range.is_lb_closed) {
            lb_closed = true;
        }

        if (range.ub > max_ub) {
            max_ub = range.ub;
            ub_closed = range.is_ub_closed;
        }
        else if (range.ub == max_ub && range.is_ub_closed) {
            ub_closed = true;
        }
    }

    return RANGE(min_lb, max_ub, lb_closed, ub_closed);
}

// �����RANGE�Ĳ���
RANGE get_complement(const RANGE& range) {
    RANGE new_range;

    // ������
    if (range.not_equal_to != std::numeric_limits<double>::infinity()) {
        new_range.lb = range.not_equal_to;
        new_range.ub = range.not_equal_to;
        new_range.is_lb_closed = true;
        new_range.is_ub_closed = true;
    }
    // ����
    else if (range.lb == range.ub) {
        new_range.not_equal_to = range.lb;
    }
    // ���ڣ����ڣ�
    else if (range.ub == std::numeric_limits<double>::infinity()) {
        new_range.ub = range.lb;
        new_range.is_ub_closed = !range.is_lb_closed;
    }
    // С�ڣ����ڣ�
    else if (range.lb == -std::numeric_limits<double>::infinity()) {
        new_range.lb = range.ub;
        new_range.is_lb_closed = !range.is_ub_closed;
    }

    return new_range;
}

// ���ַ�����ɾ���Ӵ�
void removeSubstrings(std::string& mainString, const std::vector<std::string>& subStrings) {
    for (const auto& pattern : subStrings) {
        size_t pos = 0;
        while ((pos = mainString.find(pattern, pos)) != std::string::npos) {
            mainString.replace(pos, pattern.length(), "");
        }
    }
}

class GRAPHS { //����ͼ����
private:
    vector<GRAPH> graphs;
    vector<DIC_DATA> data_dic;
public:
    // ����Ϊ������Ҫ�������ڲ���������

    // ��ʼ���������ڲ�����������������ݵ�data_dic����data_dic��ֵ��
    void addDataToDic(vector<DIC_DATA>& data_dic, DATA& data) {
        // ���data_dic���Ƿ��Ѵ��ڸ���������
        auto it = find_if(data_dic.begin(), data_dic.end(), [&](const DIC_DATA& dic_data) {
            return dic_data.name == data.name;
            });

        if (it == data_dic.end()) { // ������ݲ����ڣ�����������ݵ�data_dic
            DIC_DATA new_data;
            new_data.name = data.name;
            new_data.possibleValues.push_back(data.range.lb); // ��ӿ��ܵ�ֵ
            data_dic.push_back(new_data);
        }
        else { // ����Ѵ��ڣ����ֵ�Ƿ�����possibleValues�У����û�У������        
            if (find(it->possibleValues.begin(), it->possibleValues.end(), data.range.lb) == it->possibleValues.end()) {
                it->possibleValues.push_back(data.range.lb);
            }
        }
    }

    // ��ʼ���������ڲ���������data����vector<DATA>�У�����֮ǰ���data�Ƿ�Ϊ����������ĸ������ݣ��������������
    void input_data(const DATA& data, vector<DATA>& datas) {
        string operators = "+-*/";
        size_t pos = data.name.find_first_of(operators);

        if (pos != string::npos) {
            // ������������ĸ�������
            DATA temp_data1;
            temp_data1.name = data.name.substr(0, pos);
            temp_data1.data_type = "read";
            input_data(temp_data1, datas);

            DATA temp_data2;
            temp_data2.name = data.name.substr(pos + 1);
            temp_data2.data_type = "read";
            input_data(temp_data2, datas);
        }
        else {
            // ��������������������Ƿ������֣������������ӵ�datas
            if (!isNumber(data.name)) {
                datas.push_back(data);
            }
        }
    }

    // ����ȱʧ�����ڲ��������ж������Ƿ����ⲿ���췽
    bool check_data_has_out_producer(const DATA& data1, const GRAPH& sourceGraph) {
        // ����ÿ��ͼ
        for (const GRAPH& graph : graphs) {
            // �����������ڵ���ͬͼ
            if (&graph == &sourceGraph) continue;

            // ����ͼ�е�ÿ���ڵ�
            for (const NODE& node : graph.graph_nodes) {
                // ���ڵ��Ƿ�Ϊд����
                if (node.node_type == "write") {
                    // �����ڵ������
                    for (const DATA& data2 : node.node_datas) {
                        // ��������Ƿ�Ϊд����������ƥ�䣬���ҷ�Χ�н���
                        if (data2.data_type == "write" && data2.name == data1.name && rangesIntersect(data1.range, data2.range)) {
                            return true; // �ҵ���ƥ���д��������
                        }
                    }
                }
            }
        }

        return false; // û���ҵ�ƥ���д��������
    }

    // ����ȱʧ�����ڲ��������ж������Ƿ����ⲿ���ѷ�
    bool check_data_has_out_consumer(const DATA& data1, const GRAPH& sourceGraph) {
        // ����ÿ��ͼ
        for (const GRAPH& graph : graphs) {
            // �����������ڵ���ͬͼ
            if (&graph == &sourceGraph) continue;

            // ����ͼ�еĽڵ�
            for (const NODE& node : graph.graph_nodes) {
                // �����ڵ��е�����
                for (const DATA& data2 : node.node_datas) {
                    // ��������Ƿ�Ϊ������������ƥ�䣬���ҷ�Χ�н���
                    if (data2.data_type == "read" && data2.name == data1.name && rangesIntersect(data1.range, data2.range)) {
                        return true; // �ҵ���ƥ��Ķ���������
                    }
                }
            }

            // ����ͼ�еı�
            for (const EDGE& edge : graph.graph_edges) {
                // �������е�����
                for (const DATA& data2 : edge.edge_datas) {
                    // �������ƥ�䣬���ҷ�Χ�н���
                    if (data2.name == data1.name && rangesIntersect(data1.range, data2.range)) {
                        return true; // �ҵ���ƥ�������
                    }
                }
            }
        }

        return false; // û���ҵ�ƥ�������
    }


    // ����ȱʧ�����ڲ��������ж��Ƿ����ٴ���һ��·�����������췽�ڵ��ڱ��ڵ�֮ǰ
    bool isProducerAtOrBeforeNode(const GRAPH& graph, const NODE& consumerNode, const DATA& data) {
        for (const PATH& path : graph.paths) {
            int producerIndex = -1;
            int consumerIndex = -1;

            // ��ÿ��·��������ָ�������ݵ������߽ڵ�͵�ǰ�ڵ㣨�����ߣ�
            for (int i = 0; i < path.path_nodes.size(); ++i) {
                const NODE& currentNode = path.path_nodes[i];

                // ���Ҿ�����ͬ���ݵ������߽ڵ㣨д�������
                for (const DATA& nodeData : currentNode.node_datas) {
                    if (nodeData.name == data.name && nodeData.data_type == "write" && rangesIntersect(data.range, nodeData.range)) {
                        producerIndex = i;  // ��¼�����߽ڵ������
                        break;  // �ҵ������߽ڵ�󣬿�����ǰ�����ڲ�ѭ��
                    }
                }

                // ȷ�ϵ�ǰ�������Ľڵ��Ƿ�Ϊ����������߽ڵ�
                if (&currentNode == &consumerNode) {
                    consumerIndex = i; // ��¼�����߽ڵ������
                    break;  // �ҵ������߽ڵ���������������ǰ·����������ǰ�������ѭ��
                }
            }

            // �����ͬһ·�����ҵ�����Ӧ�������ߺ������ߣ�������������������֮ǰ������true
            if (producerIndex != -1 && consumerIndex != -1 && producerIndex < consumerIndex) {
                return true;
            }
        }

        // �������·���ж�û���ҵ�������������������������֮ǰ�����������false
        return false;
    }


    // ����ȱʧ�����ڲ��������ж��Ƿ����ٴ���һ��·�����������췽�ڵ��ڱ���֮ǰ
    bool isProducerAtOrBeforeEdge(const GRAPH& graph, const EDGE& edge, const DATA& data) {
        for (const PATH& path : graph.paths) {
            int producerIndex = -1;
            int edgeStartIndex = -1;

            // ��·���б����ڵ㣬Ѱ�������߽ڵ�ͱߵ����ڵ�
            for (int i = 0; i < path.path_nodes.size(); ++i) {
                const NODE& currentNode = path.path_nodes[i];

                // ���ڵ��е������Ƿ������Ϊ�����ߵ�����
                for (const DATA& nodeData : currentNode.node_datas) {
                    if (nodeData.name == data.name && nodeData.data_type == "write" && rangesIntersect(data.range, nodeData.range)) {
                        producerIndex = i; // ��¼�����߽ڵ������
                        break; // �ҵ����������������ߺ󣬿�����ǰ�����ڲ�ѭ��
                    }
                }

                // ȷ���ߵ����ڵ������
                if (currentNode.node_id == edge.source_node_id) {
                    edgeStartIndex = i;
                    break; // �ҵ��ߵ������������������ǰ·������Ϊ����ֻ��ȷ��������������֮ǰ
                }
            }

            // ����ҵ������ߣ�����������λ�ڱߵ�����֮ǰ������true
            if (producerIndex != -1 && edgeStartIndex != -1 && producerIndex <= edgeStartIndex) {
                return true;
            }
        }

        // �������·���ж�û���ҵ������������������ڱߵ�����֮ǰ�����������false
        return false;
    }


    // SMT��ص��ڲ����������������������ʽ����"x < 10"
    z3::expr parseSimpleConditionToExpr(const std::string& condition, z3::context& c) {
        std::istringstream iss(condition);
        std::string varName, op;
        int value;

        iss >> varName >> op >> value;

        z3::expr var = c.int_const(varName.c_str()); // ��������
        z3::expr val = c.int_val(value); // ֵ����

        // ʹ�� switch-case �򻯶���������ж�
        if (op == ">") return var > val;
        if (op == "<") return var < val;
        if (op == "==") return var == val;
        if (op == ">=") return var >= val;
        if (op == "<=") return var <= val;
        if (op == "!=") return var != val;

        // ���û��ƥ��Ĳ����������Կ����׳��쳣�򷵻�һ��Ĭ�ϵ���Ч���ʽ
        throw std::invalid_argument("Unsupported operator: " + op);
    }


    // SMT��ص��ڲ������������ַ�������������z3���ʽ
    z3::expr parseConditionToExpr(const std::string& condition, z3::context& c) {
        std::string conditionCopy = condition;
        removePattern(conditionCopy, "duration");
        removePattern(conditionCopy, "after");

        std::istringstream iss(conditionCopy);
        std::string token;
        std::vector<z3::expr> exprs;
        std::vector<std::string> ops;

        while (iss >> token) {
            if (token == "and" || token == "or") {
                ops.push_back(token);
            }
            else if (token[0] == '(') {
                // ���������ڵı��ʽ
                int count = 1;
                std::string subExpr = token.substr(1);
                while (iss >> token) {
                    if (token[0] == '(') {
                        count++;
                    }
                    if (token.back() == ')') {
                        count--;
                        if (count == 0) {
                            subExpr += " " + token.substr(0, token.length() - 1);
                            break;
                        }
                    }
                    subExpr += " " + token;
                }
                // �ݹ鴦�������ڵı��ʽ
                z3::expr e = parseConditionToExpr(subExpr, c);
                exprs.push_back(e);
            }
            else {
                // ������������ʽ
                std::string simpleExpr = token;
                iss >> token; // ��ȡ������
                simpleExpr += " " + token;
                iss >> token; // ��ȡֵ
                simpleExpr += " " + token;

                z3::expr e = parseSimpleConditionToExpr(simpleExpr, c);
                exprs.push_back(e);
            }
        }

        // �����߼�������������ϱ��ʽ
        z3::expr result = exprs.front();
        for (size_t i = 0; i < ops.size(); ++i) {
            if (ops[i] == "and") {
                result = result && exprs[i + 1];
            }
            else if (ops[i] == "or") {
                result = result || exprs[i + 1];
            }
        }

        return result;
    }


    // SMT��ص��ڲ���������ȡ���ʽ�б����ĸ�������
    void collectVariables(const z3::expr& e, std::set<z3::expr>& variables) {
        if (e.is_var()) {
            variables.insert(e);
        }
        else if (e.is_app()) {
            for (unsigned i = 0; i < e.num_args(); ++i) {
                collectVariables(e.arg(i), variables);
            }
        }
        else if (e.is_quantifier()) {
            collectVariables(e.body(), variables);
        }
    }

    // SMT��ص��ڲ���������ȡ���ʽ�е����б���
    std::vector<z3::expr> getVariables(const z3::expr& e) {
        std::set<z3::expr> variables;
        collectVariables(e, variables);
        return std::vector<z3::expr>(variables.begin(), variables.end());
    }

    // SMT��ص��ڲ����������� Z3 ���������Ʋ��Ҷ�Ӧ�� RANGE ���󣬷������ߵ�map
    map<z3::expr, RANGE> getVariableRanges(const std::vector<z3::expr>& variables, const std::vector<DIC_DATA>& data_dic) {
        map<z3::expr, RANGE> variableRanges;

        for (const auto& var : variables) {
            string varName = var.decl().name().str();
            for (const auto& data : data_dic) {
                if (data.name == varName) {
                    variableRanges[var] = data.theoretical_range;
                    break;
                }
            }
        }
        return variableRanges;
    }

    // ���������� RANGE ���󹹽� Z3 ���ʽ
    z3::expr buildRangeExpression(z3::context& c, const z3::expr& var, const RANGE& range) {
        z3::expr_vector constraints(c);

        if (range.lb != -std::numeric_limits<double>::infinity()) {
            if (range.is_lb_closed) {
                constraints.push_back(var >= c.real_val(std::to_string(range.lb).c_str()));
            }
            else {
                constraints.push_back(var > c.real_val(std::to_string(range.lb).c_str()));
            }
        }

        if (range.ub != std::numeric_limits<double>::infinity()) {
            if (range.is_ub_closed) {
                constraints.push_back(var <= c.real_val(std::to_string(range.ub).c_str()));
            }
            else {
                constraints.push_back(var < c.real_val(std::to_string(range.ub).c_str()));
            }
        }

        if (range.not_equal_to != std::numeric_limits<double>::infinity()) {
            constraints.push_back(var != c.real_val(std::to_string(range.not_equal_to).c_str()));
        }

        return mk_and(constraints);
    }

    // ���������ݱ�����Χ���� Z3 ���ʽ
    z3::expr buildConstraints(z3::context& c, const std::map<z3::expr, RANGE>& variableRanges) {
        z3::expr_vector constraints(c);

        for (std::map<z3::expr, RANGE>::const_iterator it = variableRanges.begin(); it != variableRanges.end(); ++it) {
            const z3::expr& var = it->first;
            const RANGE& range = it->second;
            constraints.push_back(buildRangeExpression(c, var, range));
        }

        return mk_and(constraints);
    }


    // ��֧ͬʱ������ڲ��������������edges���жϸ���edges�ܷ�ͬʱ����
    bool checkEdgesForSatisfiability(const vector<EDGE>& edges) {
        z3::context c;
        z3::solver s(c);

        for (const auto& edge : edges) {
            // ����ÿ���ߵ�edge_information��������Z3���ʽ
            z3::expr conditionExpr = parseConditionToExpr(edge.edge_information, c);
            s.add(conditionExpr);
        }

        return s.check() == z3::sat;
    }

    // ����Ϊ����ͼ�Ļ�������

    // ����ͼ��Ϣ����ͼ��Ϣ�����޸�/��ӡ�����������modify_allΪfalse��˵���Ǿֲ��޸ģ�����Ϊ�����ʼ��
    void requirement_diagram_analysis(const json& received_data, httplib::Response& res) {
        GRAPH temp_graph; // ��ǰ��ȡ������ͼ����
        NODE temp_node; // ��ǰ��ȡ�Ľڵ����
        EDGE temp_edge; // ��ǰ��ȡ�ı߶���
        DATA temp_data; // ��ǰ��ȡ�����ݶ���
        vector<GRAPH> temp_graphs; // �������޸�/��ӵ�����ͼ
        // ��� method �ֶ�
        bool modify_all = received_data.value("modify_all", false);  // Ĭ��Ϊ false
        // ��ӡ method �ֶ�ֵ
        std::cout << "modify_all: " << (modify_all ? "True" : "False") << std::endl;
        if (modify_all == false) { // ֻ�޸Ĳ���
            for (const auto& graph : received_data["graphs"]) {
                // ���� "graphs" �����е�ÿ��ͼ
                bool exist = false;
                for (GRAPH& Graph : graphs) {
                    if (graph["graph_id"] == Graph.graph_id) {
                        Graph.clear();
                        exist = true;
                        for (const auto& node : graph["nodes"]) {
                            temp_node.node_id = node["node_id"];
                            temp_node.node_type = node["node_type"];
                            temp_node.node_information = node["node_information"];
                            // �ж��Ƿ��� "branch_yes" �� "branch_no" �ֶ�
                            if (temp_node.node_type == "read") {
                                temp_node.branch_yes = node["branch_yes"];
                                temp_node.branch_no = node["branch_no"];
                            }
                            Graph.graph_nodes.push_back(temp_node);
                        }
                        for (const auto& edge : graph["edges"]) {
                            temp_edge.edge_id = edge["edge_id"];
                            temp_edge.source_node_id = edge["source_node_id"];
                            temp_edge.target_node_id = edge["target_node_id"];
                            temp_edge.edge_information = edge["edge_information"];
                            Graph.graph_edges.push_back(temp_edge);
                        }
                        temp_graphs.push_back(Graph);
                    }
                }
                if (exist == false) { // ��������Ѿ����ڵ�ͼ
                    temp_graph.graph_id = graph["graph_id"];
                    for (const auto& node : graph["nodes"]) {
                        temp_node.node_id = node["node_id"];
                        temp_node.node_type = node["node_type"];
                        temp_node.node_information = node["node_information"];
                        // �ж��Ƿ��� "branch_yes" �� "branch_no" �ֶ�
                        if (temp_node.node_type == "read") {
                            temp_node.branch_yes = node["branch_yes"];
                            temp_node.branch_no = node["branch_no"];
                        }
                        temp_graph.graph_nodes.push_back(temp_node);
                    }
                    for (const auto& edge : graph["edges"]) {
                        temp_edge.edge_id = edge["edge_id"];
                        temp_edge.source_node_id = edge["source_node_id"];
                        temp_edge.target_node_id = edge["target_node_id"];
                        temp_edge.edge_information = edge["edge_information"];
                        temp_graph.graph_edges.push_back(temp_edge);
                    }
                    graphs.push_back(temp_graph);
                    temp_graphs.push_back(temp_graph);
                }
            }

            // ʹ��information��datas��ֵ
            for (auto& graph : temp_graphs) { // ����ͼ
                // ����ͼ�����нڵ㲢������ֵ����ʱ�Ὣ����a+b-c��Ϊһ���������ݲ����棩
                for (auto& node : graph.graph_nodes) {
                    // �����ڵ�����Ϣ�ĸ�����ɾ�����������롰duration�������У�
                    string processed_node_info = node.node_information;
                    processed_node_info.erase(remove(processed_node_info.begin(), processed_node_info.end(), '('), processed_node_info.end());
                    processed_node_info.erase(remove(processed_node_info.begin(), processed_node_info.end(), ')'), processed_node_info.end());
                    processed_node_info.erase(remove(processed_node_info.begin(), processed_node_info.end(), ','), processed_node_info.end());
                    vector<string> subStrings;
                    subStrings.push_back("duration");
                    subStrings.push_back("min");
                    subStrings.push_back("fabs");
                    removeSubstrings(processed_node_info, subStrings);
                    istringstream iss(processed_node_info);
                    string s;
                    DATA data_temp1;
                    DATA data_temp2;

                    // �Կո�Ϊ�ָ�����ȡÿ���ַ���
                    while (iss >> s) {
                        if (s == "and" || s == "or") {
                            iss >> s;
                        }
                        data_temp1.name = s;
                        data_temp1.data_type = node.node_type;
                        iss >> s;
                        if (s == ">") {
                            iss >> s;
                            if (isNumber(s) == true) { // ˵����һ�����ݣ�ָ���˾�����ֵ
                                data_temp1.range = RANGE(double(stoi(s)), positiveInfinity, false, false, positiveInfinity);
                                input_data(data_temp1, node.node_datas); // ��data����vector<DATA>�У�����֮ǰ���data�Ƿ�Ϊ����������ĸ������ݣ��������������
                            }
                            else { // ˵�����������ݣ���ֵ��δ֪
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, node.node_datas);
                                input_data(data_temp2, node.node_datas);
                            }
                        }
                        else if (s == ">=") {
                            iss >> s;
                            if (isNumber(s) == true) { // ˵����һ�����ݣ�ָ���˾�����ֵ
                                data_temp1.range = RANGE(double(stoi(s)), positiveInfinity, true, false, positiveInfinity);
                                input_data(data_temp1, node.node_datas);
                            }
                            else { // ˵�����������ݣ���ֵ��δ֪
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, node.node_datas);
                                input_data(data_temp2, node.node_datas);
                            }
                        }
                        else if (s == "<") {
                            iss >> s;
                            if (isNumber(s) == true) { // ˵����һ�����ݣ�ָ���˾�����ֵ
                                data_temp1.range = RANGE(negativeInfinity, double(stoi(s)), false, false, positiveInfinity);
                                input_data(data_temp1, node.node_datas);
                            }
                            else { // ˵�����������ݣ���ֵ��δ֪
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, node.node_datas);
                                input_data(data_temp2, node.node_datas);
                            }
                        }
                        else if (s == "<=") {
                            iss >> s;
                            if (isNumber(s) == true) { // ˵����һ�����ݣ�ָ���˾�����ֵ
                                data_temp1.range = RANGE(negativeInfinity, double(stoi(s)), false, true, positiveInfinity);
                                input_data(data_temp1, node.node_datas);
                            }
                            else { // ˵�����������ݣ���ֵ��δ֪
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, node.node_datas);
                                input_data(data_temp2, node.node_datas);
                            }
                        }
                        else if (s == "!=") {
                            iss >> s;
                            if (isNumber(s) == true) { // ˵����һ�����ݣ�ָ���˾�����ֵ
                                data_temp1.range = RANGE(negativeInfinity, positiveInfinity, false, false, double(stoi(s)));
                                input_data(data_temp1, node.node_datas);
                            }
                            else { // ˵����/д������ݣ���ֵ��δ֪
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, node.node_datas);
                                input_data(data_temp2, node.node_datas);
                            }
                        }
                        else if (s == "==" || s == "=") {
                            iss >> s;
                            if (isNumber(s) == true) { // ˵����һ�����ݣ�ָ���˾�����ֵ
                                data_temp1.range = RANGE(double(stoi(s)), double(stoi(s)), true, true, positiveInfinity);
                                input_data(data_temp1, node.node_datas);
                            }
                            else { // ˵����/д������ݣ���ֵ��δ֪
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, node.node_datas);
                                input_data(data_temp2, node.node_datas);
                            }
                        }
                    }
                }

                // ����ͼ�����б�
                for (auto& edge : graph.graph_edges) {
                    // ����������Ϣ�ĸ�����ɾ�����������롰duration�������У�
                    string processed_edge_info = edge.edge_information;
                    processed_edge_info.erase(remove(processed_edge_info.begin(), processed_edge_info.end(), '('), processed_edge_info.end());
                    processed_edge_info.erase(remove(processed_edge_info.begin(), processed_edge_info.end(), ')'), processed_edge_info.end());
                    processed_edge_info.erase(remove(processed_edge_info.begin(), processed_edge_info.end(), ','), processed_edge_info.end());
                    vector<string> subStrings;
                    subStrings.push_back("duration");
                    subStrings.push_back("min");
                    subStrings.push_back("fabs");
                    removeSubstrings(processed_edge_info, subStrings);
                    istringstream iss(processed_edge_info);
                    string s;
                    DATA data_temp1;
                    DATA data_temp2;

                    // �Կո�Ϊ�ָ�����ȡÿ���ַ���
                    while (iss >> s) {
                        if (s == "and" || s == "or") {
                            iss >> s;
                        }
                        data_temp1.name = s;
                        data_temp1.data_type = "read";
                        iss >> s;
                        if (s == ">") {
                            iss >> s;
                            if (isNumber(s) == true) { // ˵����һ�����ݣ�ָ���˾�����ֵ
                                data_temp1.range = RANGE(double(stoi(s)), positiveInfinity, false, false, positiveInfinity);
                                input_data(data_temp1, edge.edge_datas);
                            }
                            else { // ˵�����������ݣ���ֵ��δ֪
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, edge.edge_datas);
                                input_data(data_temp2, edge.edge_datas);
                            }
                        }
                        else if (s == ">=") {
                            iss >> s;
                            if (isNumber(s) == true) { // ˵����һ�����ݣ�ָ���˾�����ֵ
                                data_temp1.range = RANGE(double(stoi(s)), positiveInfinity, true, false, positiveInfinity);
                                input_data(data_temp1, edge.edge_datas);
                            }
                            else { // ˵�����������ݣ���ֵ��δ֪
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, edge.edge_datas);
                                input_data(data_temp2, edge.edge_datas);
                            }
                        }
                        else if (s == "<") {
                            iss >> s;
                            if (isNumber(s) == true) { // ˵����һ�����ݣ�ָ���˾�����ֵ
                                data_temp1.range = RANGE(negativeInfinity, double(stoi(s)), false, false, positiveInfinity);
                                input_data(data_temp1, edge.edge_datas);
                            }
                            else { // ˵�����������ݣ���ֵ��δ֪
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, edge.edge_datas);
                                input_data(data_temp2, edge.edge_datas);
                            }
                        }
                        else if (s == "<=") {
                            iss >> s;
                            if (isNumber(s) == true) { // ˵����һ�����ݣ�ָ���˾�����ֵ
                                data_temp1.range = RANGE(negativeInfinity, double(stoi(s)), false, true, positiveInfinity);
                                input_data(data_temp1, edge.edge_datas);
                            }
                            else { // ˵�����������ݣ���ֵ��δ֪
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, edge.edge_datas);
                                input_data(data_temp2, edge.edge_datas);
                            }
                        }
                        else if (s == "!=") {
                            iss >> s;
                            if (isNumber(s) == true) { // ˵����һ�����ݣ�ָ���˾�����ֵ
                                data_temp1.range = RANGE(negativeInfinity, positiveInfinity, false, false, double(stoi(s)));
                                input_data(data_temp1, edge.edge_datas);
                            }
                            else { // ˵����/д������ݣ���ֵ��δ֪
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, edge.edge_datas);
                                input_data(data_temp2, edge.edge_datas);
                            }
                        }
                        else if (s == "==" || s == "=") {
                            iss >> s;
                            if (isNumber(s) == true) { // ˵����һ�����ݣ�ָ���˾�����ֵ
                                data_temp1.range = RANGE(double(stoi(s)), double(stoi(s)), true, true, positiveInfinity);
                                input_data(data_temp1, edge.edge_datas);
                            }
                            else { // ˵����/д������ݣ���ֵ��δ֪
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, edge.edge_datas);
                                input_data(data_temp2, edge.edge_datas);
                            }
                        }
                    }
                }
            }

            // ��paths��ֵ�������ڵ���߼���path
            for (auto& graph : temp_graphs) {
                graph.generateAllPaths();
            }
        }
        else { // ȫ�����벢�޸�
            graphs.clear();
            for (const auto& graph : received_data["graphs"]) {
                // ���� "graphs" �����е�ÿ��ͼ
                temp_graph.clear();
                temp_graph.graph_id = graph["graph_id"];
                for (const auto& node : graph["nodes"]) {
                    temp_node.clear();
                    temp_node.node_id = node["node_id"];
                    temp_node.node_type = node["node_type"];
                    temp_node.node_information = node["node_information"];
                    // �ж��Ƿ��� "branch_yes" �� "branch_no" �ֶ�
                    if (temp_node.node_type == "read") {
                        temp_node.branch_yes = node["branch_yes"];
                        temp_node.branch_no = node["branch_no"];
                    }
                    temp_graph.graph_nodes.push_back(temp_node);
                }
                for (const auto& edge : graph["edges"]) {
                    temp_edge.clear();
                    temp_edge.edge_id = edge["edge_id"];
                    temp_edge.source_node_id = edge["source_node_id"];
                    temp_edge.target_node_id = edge["target_node_id"];
                    temp_edge.edge_information = edge["edge_information"];
                    temp_graph.graph_edges.push_back(temp_edge);
                }
                graphs.push_back(temp_graph);
            }

            // ʹ��information��datas��ֵ
            for (auto& graph : graphs) { // ����ͼ
                // ����ͼ�����нڵ㲢������ֵ����ʱ�Ὣ����a+b-c��Ϊһ���������ݲ����棩
                for (auto& node : graph.graph_nodes) {
                    // �����ڵ�����Ϣ�ĸ�����ɾ�����������롰duration�������У�
                    string processed_node_info = node.node_information;
                    processed_node_info.erase(remove(processed_node_info.begin(), processed_node_info.end(), '('), processed_node_info.end());
                    processed_node_info.erase(remove(processed_node_info.begin(), processed_node_info.end(), ')'), processed_node_info.end());
                    processed_node_info.erase(remove(processed_node_info.begin(), processed_node_info.end(), ','), processed_node_info.end());
                    vector<string> subStrings;
                    subStrings.push_back("duration");
                    subStrings.push_back("min");
                    subStrings.push_back("fabs");
                    removeSubstrings(processed_node_info, subStrings);
                    istringstream iss(processed_node_info);
                    string s;
                    DATA data_temp1;
                    DATA data_temp2;

                    // �Կո�Ϊ�ָ�����ȡÿ���ַ���
                    while (iss >> s) {
                        if (s == "and" || s == "or") {
                            iss >> s;
                        }
                        data_temp1.name = s;
                        data_temp1.data_type = node.node_type;
                        iss >> s;
                        if (s == ">") {
                            iss >> s;
                            if (isNumber(s) == true) { // ˵����һ�����ݣ�ָ���˾�����ֵ
                                data_temp1.range = RANGE(double(stoi(s)), positiveInfinity, false, false, positiveInfinity);
                                input_data(data_temp1, node.node_datas); // ��data����vector<DATA>�У�����֮ǰ���data�Ƿ�Ϊ����������ĸ������ݣ��������������
                            }
                            else { // ˵�����������ݣ���ֵ��δ֪
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, node.node_datas);
                                input_data(data_temp2, node.node_datas);
                            }
                        }
                        else if (s == ">=") {
                            iss >> s;
                            if (isNumber(s) == true) { // ˵����һ�����ݣ�ָ���˾�����ֵ
                                data_temp1.range = RANGE(double(stoi(s)), positiveInfinity, true, false, positiveInfinity);
                                input_data(data_temp1, node.node_datas);
                            }
                            else { // ˵�����������ݣ���ֵ��δ֪
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, node.node_datas);
                                input_data(data_temp2, node.node_datas);
                            }
                        }
                        else if (s == "<") {
                            iss >> s;
                            if (isNumber(s) == true) { // ˵����һ�����ݣ�ָ���˾�����ֵ
                                data_temp1.range = RANGE(negativeInfinity, double(stoi(s)), false, false, positiveInfinity);
                                input_data(data_temp1, node.node_datas);
                            }
                            else { // ˵�����������ݣ���ֵ��δ֪
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, node.node_datas);
                                input_data(data_temp2, node.node_datas);
                            }
                        }
                        else if (s == "<=") {
                            iss >> s;
                            if (isNumber(s) == true) { // ˵����һ�����ݣ�ָ���˾�����ֵ
                                data_temp1.range = RANGE(negativeInfinity, double(stoi(s)), false, true, positiveInfinity);
                                input_data(data_temp1, node.node_datas);
                            }
                            else { // ˵�����������ݣ���ֵ��δ֪
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, node.node_datas);
                                input_data(data_temp2, node.node_datas);
                            }
                        }
                        else if (s == "!=") {
                            iss >> s;
                            if (isNumber(s) == true) { // ˵����һ�����ݣ�ָ���˾�����ֵ
                                data_temp1.range = RANGE(negativeInfinity, positiveInfinity, false, false, double(stoi(s)));
                                input_data(data_temp1, node.node_datas);
                            }
                            else { // ˵����/д������ݣ���ֵ��δ֪
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, node.node_datas);
                                input_data(data_temp2, node.node_datas);
                            }
                        }
                        else if (s == "==" || s == "=") {
                            iss >> s;
                            if (isNumber(s) == true) { // ˵����һ�����ݣ�ָ���˾�����ֵ
                                data_temp1.range = RANGE(double(stoi(s)), double(stoi(s)), true, true, positiveInfinity);
                                input_data(data_temp1, node.node_datas);
                            }
                            else { // ˵����/д������ݣ���ֵ��δ֪
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, node.node_datas);
                                input_data(data_temp2, node.node_datas);
                            }
                        }
                    }
                }

                // ����ͼ�����б�
                for (auto& edge : graph.graph_edges) {
                    // ����������Ϣ�ĸ�����ɾ�����������롰duration�������У�
                    string processed_edge_info = edge.edge_information;
                    processed_edge_info.erase(remove(processed_edge_info.begin(), processed_edge_info.end(), '('), processed_edge_info.end());
                    processed_edge_info.erase(remove(processed_edge_info.begin(), processed_edge_info.end(), ')'), processed_edge_info.end());
                    processed_edge_info.erase(remove(processed_edge_info.begin(), processed_edge_info.end(), ','), processed_edge_info.end());
                    vector<string> subStrings;
                    subStrings.push_back("duration");
                    subStrings.push_back("min");
                    subStrings.push_back("fabs");
                    removeSubstrings(processed_edge_info, subStrings);
                    istringstream iss(processed_edge_info);
                    string s;
                    DATA data_temp1;
                    DATA data_temp2;

                    // �Կո�Ϊ�ָ�����ȡÿ���ַ���
                    while (iss >> s) {
                        if (s == "and" || s == "or") {
                            iss >> s;
                        }
                        data_temp1.name = s;
                        data_temp1.data_type = "read";
                        iss >> s;
                        if (s == ">") {
                            iss >> s;
                            if (isNumber(s) == true) { // ˵����һ�����ݣ�ָ���˾�����ֵ
                                data_temp1.range = RANGE(double(stoi(s)), positiveInfinity, false, false, positiveInfinity);
                                input_data(data_temp1, edge.edge_datas);
                            }
                            else { // ˵�����������ݣ���ֵ��δ֪
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, edge.edge_datas);
                                input_data(data_temp2, edge.edge_datas);
                            }
                        }
                        else if (s == ">=") {
                            iss >> s;
                            if (isNumber(s) == true) { // ˵����һ�����ݣ�ָ���˾�����ֵ
                                data_temp1.range = RANGE(double(stoi(s)), positiveInfinity, true, false, positiveInfinity);
                                input_data(data_temp1, edge.edge_datas);
                            }
                            else { // ˵�����������ݣ���ֵ��δ֪
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, edge.edge_datas);
                                input_data(data_temp2, edge.edge_datas);
                            }
                        }
                        else if (s == "<") {
                            iss >> s;
                            if (isNumber(s) == true) { // ˵����һ�����ݣ�ָ���˾�����ֵ
                                data_temp1.range = RANGE(negativeInfinity, double(stoi(s)), false, false, positiveInfinity);
                                input_data(data_temp1, edge.edge_datas);
                            }
                            else { // ˵�����������ݣ���ֵ��δ֪
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, edge.edge_datas);
                                input_data(data_temp2, edge.edge_datas);
                            }
                        }
                        else if (s == "<=") {
                            iss >> s;
                            if (isNumber(s) == true) { // ˵����һ�����ݣ�ָ���˾�����ֵ
                                data_temp1.range = RANGE(negativeInfinity, double(stoi(s)), false, true, positiveInfinity);
                                input_data(data_temp1, edge.edge_datas);
                            }
                            else { // ˵�����������ݣ���ֵ��δ֪
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, edge.edge_datas);
                                input_data(data_temp2, edge.edge_datas);
                            }
                        }
                        else if (s == "!=") {
                            iss >> s;
                            if (isNumber(s) == true) { // ˵����һ�����ݣ�ָ���˾�����ֵ
                                data_temp1.range = RANGE(negativeInfinity, positiveInfinity, false, false, double(stoi(s)));
                                input_data(data_temp1, edge.edge_datas);
                            }
                            else { // ˵����/д������ݣ���ֵ��δ֪
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, edge.edge_datas);
                                input_data(data_temp2, edge.edge_datas);
                            }
                        }
                        else if (s == "==" || s == "=") {
                            iss >> s;
                            if (isNumber(s) == true) { // ˵����һ�����ݣ�ָ���˾�����ֵ
                                data_temp1.range = RANGE(double(stoi(s)), double(stoi(s)), true, true, positiveInfinity);
                                input_data(data_temp1, edge.edge_datas);
                            }
                            else { // ˵����/д������ݣ���ֵ��δ֪
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, edge.edge_datas);
                                input_data(data_temp2, edge.edge_datas);
                            }
                        }
                    }
                }
            }

            // ��paths��ֵ�������ڵ���߼���path
            for (auto& graph : graphs) {
                graph.generateAllPaths();
            }
        }

        // ����һ�� JSON ����
        nlohmann::json response_data;
        response_data["state"] = "data update success!";




        // ����һ���������ڴ��ÿ��ͼ��·���ֵ�
        nlohmann::json paths_array = nlohmann::json::array();

        // ����ÿ��ͼ
        for (const auto& graph : graphs) {
            nlohmann::json graph_data;
            graph_data["graph_id"] = graph.graph_id;

            // ����һ�������Ÿ�ͼ�е�·��
            nlohmann::json paths_in_graph = nlohmann::json::array();

            // ������ͼ�е�ÿ��·��
            for (const auto& path : graph.paths) {
                nlohmann::json path_data;
                path_data["path_id"] = path.path_id;

                // ����һ��������·���еĽڵ㡢�����ͱ�
                nlohmann::json nodes_conditions_transitions_json = nlohmann::json::array();

                // ����·���е�ÿ���ڵ�
                for (size_t i = 0; i < path.path_nodes.size(); ++i) {
                    const auto& node = path.path_nodes[i];

                    // �����read���ͽڵ㣨�����жϣ�
                    if (node.node_type == "read") {
                        nlohmann::json condition_data;
                        condition_data["condition_id"] = node.node_id;
                        condition_data["condition_information"] = node.node_information;

                        // ��������ķ�֧��Ϣ
                        condition_data["branch_yes"] = node.branch_yes;
                        condition_data["branch_no"] = node.branch_no;

                        // ������������ӵ��ڵ�֮��
                        nodes_conditions_transitions_json.push_back(condition_data);
                    }

                    // �����write���ͽڵ㣨��ֵ��������ֱ�����
                    else if (node.node_type == "write") {
                        // �ڵ���Ϣ
                        nlohmann::json node_data;
                        node_data["node_id"] = node.node_id;
                        node_data["node_information"] = node.node_information;
                        // ����д������ֱ������ڵ�����
                        nodes_conditions_transitions_json.push_back(node_data);
                    }

                    // ����ÿ���ڵ㣬����ͨ��һ���ߣ�transition�����ӵ���һ���ڵ�
                    // �ҵ����ӵ�ǰ�ڵ�ıߣ�transition��
                    for (const auto& edge : path.path_edges) {
                        if (node.node_type == "write") { //�����д�ڵ㣬ͨ�������õ���Ӧ�Ľڵ�
                            if (edge.source_node_id == node.node_id) {
                                nlohmann::json transition_data;
                                transition_data["transition_id"] = edge.edge_id;
                                transition_data["transition_information"] = edge.edge_information;

                                // ����������ӵ�·����
                                nodes_conditions_transitions_json.push_back(transition_data);
                            }
                        }
                        else if (node.node_type == "read") { // ������ݽڵ��branch���õ���Ӧ�ı�
                            if (edge.edge_id == node.branch_yes || edge.edge_id == node.branch_no) {
                                nlohmann::json transition_data;
                                transition_data["transition_id"] = edge.edge_id;
                                transition_data["transition_information"] = edge.edge_information;

                                // ����������ӵ�·����
                                nodes_conditions_transitions_json.push_back(transition_data);
                            }
                        }
                    }
                }

                // ��·���еĽڵ㡢�����ͱ���Ϣ��ӵ�·��������
                path_data["nodes and conditions and transitions"] = nodes_conditions_transitions_json;

                // ��·��������ӵ���ͼ��·��������
                paths_in_graph.push_back(path_data);
            }

            // ����ͼ��·����Ϣ��ӵ���·��������
            graph_data["paths"] = paths_in_graph;
            paths_array.push_back(graph_data);
        }

        // ��·���ֵ���ӵ�response_data��
        response_data["paths"] = paths_array;



        // ������Ӧ����Ϊ JSON������ JSON ������Ϊ��Ӧ
        res.set_content(response_data.dump(), "application/json");  // .dump() �� JSON ����ת��Ϊ�ַ���

    }

    // ����ȫ��������ϵ
    void return_requirements(httplib::Response& res) {
        // ����һ�� JSON ����
        nlohmann::json response_data;

        int count = 0;

        // ����һ�� JSON �������洢������ϵ
        nlohmann::json dependencies = nlohmann::json::array();


        // ���� graphs �����е�ÿ��ͼ
        for (int i = 0; i < graphs.size(); ++i) {
            GRAPH& currentGraph = graphs[i];
            
            // ������ǰͼ�е�ÿ��·��
            for (PATH& path : currentGraph.paths) {
                // ����·���ϵ����нڵ�
                for (NODE& node : path.path_nodes) { 
                    // �����д�ڵ�
                    if (node.node_type == "write") {
                        for (DATA& data : node.node_datas) { // �����ڵ������
                            if (data.data_type == "read") { // ����Ƕ�����
                                string readName = data.name; // ��ȡ���ڵ����ݵ�����

                                // ��������ǰͼ���������ͼ
                                for (int j = 0; j < graphs.size(); ++j) {
                                    if (i == j) continue; // ������ǰͼ

                                    GRAPH& otherGraph = graphs[j];

                                    // ��������ͼ�е�·��
                                    for (PATH& otherPath : otherGraph.paths) {
                                        for (NODE& writeNode : otherPath.path_nodes) { // ����·���ϵ����нڵ�
                                            for (DATA& writeData : writeNode.node_datas) { // ����д�ڵ������
                                                if (writeNode.node_type == "write" && writeData.data_type == "write" && writeData.name == readName) { // �����д�ڵ��д�����������������������ͬ
                                                    RANGE readRange = data.range; // ��ȡ���ڵ����ݵķ�Χ
                                                    RANGE writeRange = writeData.range; // ��ȡд�ڵ����ݵķ�Χ

                                                    count++;

                                                    // ����������ϵ�� JSON ����
                                                    nlohmann::json dependency;
                                                    dependency["dependent_path_id"] = path.path_id;
                                                    dependency["depended_path_id"] = otherPath.path_id;
                                                    dependency["data_name"] = readName;

                                                    // ����������ݷ�Χ
                                                    if (readRange.not_equal_to != positiveInfinity) {
                                                        dependency["dependent_range"] = "!=" + to_string(readRange.not_equal_to);
                                                    }
                                                    else {
                                                        dependency["dependent_range"] = (readRange.is_lb_closed ? "[" : "(") +
                                                            to_string(readRange.lb) + ", " + to_string(readRange.ub) + (readRange.is_ub_closed ? "]" : ")");
                                                    }

                                                    // ��ӱ��������ݷ�Χ
                                                    if (writeRange.not_equal_to != positiveInfinity) {
                                                        dependency["depended_range"] = "!=" + to_string(writeRange.not_equal_to);
                                                    }
                                                    else {
                                                        dependency["depended_range"] = (writeRange.is_lb_closed ? "[" : "(") +
                                                            to_string(writeRange.lb) + ", " + to_string(writeRange.ub) + (writeRange.is_ub_closed ? "]" : ")");
                                                    }
                                                    dependency["dependent_node_id"] = node.node_id;
                                                    dependency["depended_node_id"] = writeNode.node_id;

                                                    // ��������ϵ��ӵ�������
                                                    dependencies.push_back(dependency);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // ����Ƕ��ڵ㣨���ڵ��yes��no��֧��Ҫ���⴦��
                    else if (node.node_type == "read") {
                        for (DATA& data : node.node_datas) { // �����ڵ������
                            string readName = data.name; // ��ȡ���ڵ����ݵ�����(һ��Ϊ������)

                            // ��������ǰͼ���������ͼ
                            for (int j = 0; j < graphs.size(); ++j) {
                                if (i == j) continue; // ������ǰͼ

                                GRAPH& otherGraph = graphs[j];

                                // ��������ͼ�е�·��
                                for (PATH& otherPath : otherGraph.paths) {
                                    for (NODE& writeNode : otherPath.path_nodes) { // ����·���ϵ����нڵ�
                                        for (DATA& writeData : writeNode.node_datas) { // ����д�ڵ������
                                            if (writeNode.node_type == "write" && writeData.data_type == "write" && writeData.name == readName) { // �����д�ڵ��д�����������������������ͬ
                                                RANGE readRange = data.range; // ��ȡ�����ݵķ�Χ
                                                RANGE writeRange = writeData.range; // ��ȡд�ڵ����ݵķ�Χ

                                                // ���н�����˵��Ϊyes��֧
                                                if (rangesIntersect(writeData.range, data.range)) {
                                                    bool exist = false;
                                                    for (auto edge : path.path_edges) {
                                                        if (edge.edge_id == node.branch_yes) {
                                                            exist = true;
                                                            break;
                                                        }
                                                    }

                                                    // �����ڵ��yes������·���У�˵��Ӧ���������
                                                    if (exist) {
                                                        // ����������ϵ�� JSON ����
                                                        nlohmann::json dependency;
                                                        dependency["dependent_path_id"] = path.path_id;
                                                        dependency["depended_path_id"] = otherPath.path_id;
                                                        dependency["data_name"] = readName;

                                                        // ����������ݷ�Χ
                                                        dependency["dependent_range"] = (readRange.is_lb_closed ? "[" : "(") +
                                                            to_string(readRange.lb) + ", " + to_string(readRange.ub) + (readRange.is_ub_closed ? "]" : ")");
                                                        dependency["depended_range"] = (writeRange.is_lb_closed ? "[" : "(") +
                                                            to_string(writeRange.lb) + ", " + to_string(writeRange.ub) + (writeRange.is_ub_closed ? "]" : ")");
                                                        dependency["dependent_node_id"] = node.node_id;
                                                        dependency["depended_node_id"] = writeNode.node_id;

                                                        // ��������ϵ��ӵ�������
                                                        dependencies.push_back(dependency);
                                                    }
                                                }
                                                // ����Ϊno��֧
                                                else {
                                                    bool exist = false;
                                                    for (auto edge : path.path_edges) {
                                                        if (edge.edge_id == node.branch_no) {
                                                            exist = true;
                                                            break;
                                                        }
                                                    }

                                                    // �����ڵ��no������·���У�˵��Ӧ�������������Ҫ�����ݷ�Χ�Ĳ���
                                                    if (exist) {
                                                        RANGE new_range = get_complement(readRange);

                                                        // ����������ϵ�� JSON ����
                                                        nlohmann::json dependency;
                                                        dependency["dependent_path_id"] = path.path_id;
                                                        dependency["depended_path_id"] = otherPath.path_id;
                                                        dependency["data_name"] = readName;

                                                        // ����������ݷ�Χ
                                                        dependency["dependent_range"] = (new_range.is_lb_closed ? "[" : "(") +
                                                            to_string(new_range.lb) + ", " + to_string(new_range.ub) + (new_range.is_ub_closed ? "]" : ")");
                                                        dependency["depended_range"] = (writeRange.is_lb_closed ? "[" : "(") +
                                                            to_string(writeRange.lb) + ", " + to_string(writeRange.ub) + (writeRange.is_ub_closed ? "]" : ")");
                                                        dependency["dependent_node_id"] = node.node_id;
                                                        dependency["depended_node_id"] = writeNode.node_id;

                                                        // ��������ϵ��ӵ�������
                                                        dependencies.push_back(dependency);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                for (EDGE& edge : path.path_edges) { // ����·�������еı�
                    for (DATA& data : edge.edge_datas) { // �������ϵ�����
                        if (data.data_type == "read") { // ����Ƕ�����
                            string readName = data.name; // ��ȡ�����ݵ�����

                            // ��������ǰͼ���������ͼ
                            for (int j = 0; j < graphs.size(); ++j) {
                                if (i == j) continue; // ������ǰͼ

                                GRAPH& otherGraph = graphs[j];

                                // ��������ͼ�е�·��
                                for (PATH& otherPath : otherGraph.paths) {
                                    for (NODE& writeNode : otherPath.path_nodes) { // ����·���ϵ����нڵ�
                                        for (DATA& writeData : writeNode.node_datas) { // ����д�ڵ������
                                            if (writeNode.node_type == "write" && writeData.data_type == "write" && writeData.name == readName) { // �����д�ڵ��д�����������������������ͬ
                                                RANGE readRange = data.range; // ��ȡ�����ݵķ�Χ
                                                RANGE writeRange = writeData.range; // ��ȡд�ڵ����ݵķ�Χ

                                                // ����������ϵ�� JSON ����
                                                nlohmann::json dependency;
                                                dependency["dependent_path_id"] = path.path_id;
                                                dependency["depended_path_id"] = otherPath.path_id;
                                                dependency["data_name"] = readName;

                                                // ����������ݷ�Χ
                                                dependency["dependent_range"] = (readRange.is_lb_closed ? "[" : "(") +
                                                    to_string(readRange.lb) + ", " + to_string(readRange.ub) + (readRange.is_ub_closed ? "]" : ")");
                                                dependency["depended_range"] = (writeRange.is_lb_closed ? "[" : "(") +
                                                    to_string(writeRange.lb) + ", " + to_string(writeRange.ub) + (writeRange.is_ub_closed ? "]" : ")");
                                                dependency["dependent_node_id"] = edge.edge_id;
                                                dependency["depended_node_id"] = writeNode.node_id;

                                                // ��������ϵ��ӵ� JSON ������
                                                dependencies.push_back(dependency);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // ������������ϵ���뵽 response_data ��
        response_data["dependencies"] = dependencies;
        response_data["state"] = "requirement returned";

        cout << "Response data: " << response_data.dump() << endl;

        // ������Ӧ����Ϊ JSON������ JSON ������Ϊ��Ӧ
        res.set_content(response_data.dump(), "application/json");  // .dump() �� JSON ����ת��Ϊ�ַ���

    }

    // ����·���ֵ䣬�����ɶ�Ӧ�� JSON ��ʽ
    void return_paths(httplib::Response& res) {
        // ����һ�� JSON ����
        nlohmann::json response_data;
        response_data["state"] = "path returned";

        // ����һ���������ڴ��ÿ��ͼ��·���ֵ�
        nlohmann::json paths_array = nlohmann::json::array();

        // ����ÿ��ͼ
        for (const auto& graph : graphs) {
            nlohmann::json graph_data;
            graph_data["graph_id"] = graph.graph_id;

            // ����һ�������Ÿ�ͼ�е�·��
            nlohmann::json paths_in_graph = nlohmann::json::array();

            // ������ͼ�е�ÿ��·��
            for (const auto& path : graph.paths) {
                nlohmann::json path_data;
                path_data["path_id"] = path.path_id;

                // ����һ��������·���еĽڵ㡢�����ͱ�
                nlohmann::json nodes_conditions_transitions_json = nlohmann::json::array();

                // ����·���е�ÿ���ڵ�
                for (size_t i = 0; i < path.path_nodes.size(); ++i) {
                    const auto& node = path.path_nodes[i];

                    // �����read���ͽڵ㣨�����жϣ�
                    if (node.node_type == "read") {
                        nlohmann::json condition_data;
                        condition_data["condition_id"] = node.node_id;
                        condition_data["condition_information"] = node.node_information;

                        // ��������ķ�֧��Ϣ
                        condition_data["branch_yes"] = node.branch_yes;
                        condition_data["branch_no"] = node.branch_no;

                        // ������������ӵ��ڵ�֮��
                        nodes_conditions_transitions_json.push_back(condition_data);
                    }

                    // �����write���ͽڵ㣨��ֵ��������ֱ�����
                    else if (node.node_type == "write") {
                        // �ڵ���Ϣ
                        nlohmann::json node_data;
                        node_data["node_id"] = node.node_id;
                        node_data["node_information"] = node.node_information;
                        // ����д������ֱ������ڵ�����
                        nodes_conditions_transitions_json.push_back(node_data);
                    }

                    // ����ÿ���ڵ㣬����ͨ��һ���ߣ�transition�����ӵ���һ���ڵ�
                    // �ҵ����ӵ�ǰ�ڵ�ıߣ�transition��
                    for (const auto& edge : path.path_edges) {
                        if (node.node_type == "write") { //�����д�ڵ㣬ͨ�������õ���Ӧ�Ľڵ�
                            if (edge.source_node_id == node.node_id) {
                                nlohmann::json transition_data;
                                transition_data["transition_id"] = edge.edge_id;
                                transition_data["transition_information"] = edge.edge_information;

                                // ����������ӵ�·����
                                nodes_conditions_transitions_json.push_back(transition_data);
                            }
                        }
                        else if (node.node_type == "read") { // ������ݽڵ��branch���õ���Ӧ�ı�
                            if (edge.edge_id == node.branch_yes || edge.edge_id == node.branch_no) {
                                nlohmann::json transition_data;
                                transition_data["transition_id"] = edge.edge_id;
                                transition_data["transition_information"] = edge.edge_information;

                                // ����������ӵ�·����
                                nodes_conditions_transitions_json.push_back(transition_data);
                            }
                        }
                    }
                }

                // ��·���еĽڵ㡢�����ͱ���Ϣ��ӵ�·��������
                path_data["nodes and conditions and transitions"] = nodes_conditions_transitions_json;

                // ��·��������ӵ���ͼ��·��������
                paths_in_graph.push_back(path_data);
            }

            // ����ͼ��·����Ϣ��ӵ���·��������
            graph_data["paths"] = paths_in_graph;
            paths_array.push_back(graph_data);
        }

        // ��·���ֵ���ӵ�response_data��
        response_data["paths"] = paths_array;

        // ������Ӧ����Ϊ JSON������ JSON ������Ϊ��Ӧ
        res.set_content(response_data.dump(), "application/json");  // .dump() �� JSON ����ת��Ϊ�ַ���
    }

    void printPath(const PATH& path) {
        cout << "Path ID: " << path.path_id << endl;

        // ��ӡ·���еĽڵ���Ϣ
        cout << "Nodes in Path:" << endl;
        for (const auto& node : path.path_nodes) {
            cout << "  Node ID: " << node.node_id << endl;
            cout << "  Node Type: " << node.node_type << endl;
            cout << "  Node Information: " << node.node_information << endl;

            // ��ӡ�ڵ��е����ݼ���
            if (!node.node_datas.empty()) {
                cout << "  Node Data:" << endl;
                for (const auto& data : node.node_datas) {
                    cout << "    Data Name: " << data.name << endl;
                    cout << "    Data Type: " << data.data_type << endl;
                    cout << "    Range: [" << (data.range.is_lb_closed ? "[" : "(")
                        << data.range.lb << ", " << data.range.ub
                        << (data.range.is_ub_closed ? "]" : ")") << "]" << endl;
                }
            }
            cout << "  Branch Yes: " << node.branch_yes << endl;
            cout << "  Branch No: " << node.branch_no << endl;
        }

        // ��ӡ·���еı���Ϣ
        cout << "Edges in Path:" << endl;
        for (const auto& edge : path.path_edges) {
            cout << "  Edge ID: " << edge.edge_id << endl;
            cout << "  Source Node ID: " << edge.source_node_id << endl;
            cout << "  Target Node ID: " << edge.target_node_id << endl;
            cout << "  Edge Information: " << edge.edge_information << endl;

            // ��ӡ���е����ݼ���
            if (!edge.edge_datas.empty()) {
                cout << "  Edge Data:" << endl;
                for (const auto& data : edge.edge_datas) {
                    cout << "    Data Name: " << data.name << endl;
                    cout << "    Data Type: " << data.data_type << endl;
                    cout << "    Range: [" << (data.range.is_lb_closed ? "[" : "(")
                        << data.range.lb << ", " << data.range.ub
                        << (data.range.is_ub_closed ? "]" : ")") << "]" << endl;
                }
            }
        }

        cout << "End of Path\n" << endl;
    }

    // ������ʵ��
    void run_server(GRAPHS& x, const string& port) {
        // ����������ʵ��
        httplib::Server server;

        // ����һ��POST·�ɣ���Ӧ�ͻ�������
        server.Post("/data", [&x](const httplib::Request& req, httplib::Response& res) {
            try {
                // �������յ���JSON����
                json received_data = json::parse(req.body);
                std::cout << "Received JSON data: " << received_data.dump() << std::endl;

                // mode=a:����ͼ��Ϣ�����и���
                if (received_data["mode"] == "a") {
                    x.requirement_diagram_analysis(received_data, res);
                }
                // mode=b:����������ϵ
                else if (received_data["mode"] == "b") {
                    x.return_requirements(res);
                }
                // mode=c:����·����Ϣ
                else if (received_data["mode"] == "c") {
                    x.return_paths(res);
                }
            }
            catch (const std::exception& e) {
                // ������
                res.status = 400;
                res.set_content(e.what(), "text/plain");
            }
        });

        // ������Ķ˿ںŴ��ַ���ת��Ϊ����
        int port_number = std::stoi(port);  // ʹ�� stoi ���ַ���ת��Ϊ����

        // ��һ���������߳������з�����
        std::thread server_thread([&server, port_number]() {
            std::cout << "Server is listening on http://localhost:" << port_number << std::endl;
            server.listen("0.0.0.0", port_number);  // ʹ�ô���Ķ˿ں�
            });

        // �ȴ�ֱ���û��жϣ����簴Ctrl+C��������ֶ���ֹ
        server_thread.join(); // ʹ���̵߳ȴ��������̵߳����
        std::cout << "Server has stopped." << std::endl;
    }
};

GRAPHS x; // ����Ϊȫ�ֱ���


#endif // GRAPH_STRUCTURES_H

