#ifndef GRAPH_STRUCTURES_H
#define GRAPH_STRUCTURES_H

#include <iostream>
#include <bits/stdc++.h>
#include <sstream>
#include <regex>
#include <unordered_map>
#include <z3++.h>
#include <chrono>
#include "httplib.h"     // HTTP库
#include <nlohmann/json.hpp> // JSON库
#include <windows.h>
#include <filesystem>

using namespace std;
using json = nlohmann::json;

// 定义正负无穷
constexpr double positiveInfinity = numeric_limits<double>::infinity();
constexpr double negativeInfinity = -numeric_limits<double>::infinity();

// 定义数据范围结构
struct RANGE {
    double lb = negativeInfinity; // 下界
    double ub = positiveInfinity; // 上界
    bool is_lb_closed = false;    // 下界是否闭合
    bool is_ub_closed = false;    // 上界是否闭合
    double not_equal_to = positiveInfinity; // 不等于某值

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

// 数据结构
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

// 字典数据结构
struct DIC_DATA {
    string name;
    string type = "in";
    RANGE theoretical_range;
    vector<double> possibleValues;
};

// 节点结构
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

// 边结构
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

struct PATH { // 路径
    string path_id; //路径ID
    vector<NODE> path_nodes; //路径中节点集合
    vector<EDGE> path_edges; //路径中边集合
};

struct GRAPH { // 需求图
    string graph_id = "none"; // 需求图ID
    vector<PATH> paths; // 路径集合
    vector<NODE> graph_nodes; // 图中节点集合
    vector<EDGE> graph_edges; // 图中边集合
    vector<string> dependent_graph_id; // 依赖的图id集合（仅存储依赖图的id，不存储依赖的数据与数据的范围）——判断依赖是否成环要用这个集合构建图来判断
    int pathIdCounter = 0; // 边id计数器

    //清理函数
    void clear() {
        graph_id = "none";
        paths.clear();
        graph_nodes.clear();
        graph_edges.clear();
        dependent_graph_id.clear();
        pathIdCounter = 0;
    }

    // 通过id返回节点
    NODE& getNodeById(const string& node_id) {
        for (NODE& node : this->graph_nodes) {
            if (node.node_id == node_id) {
                return node;
            }
        }
        throw runtime_error("Node id not found.");
    }

    // 通过节点id返回以节点为源点的边
    vector<EDGE> getEdgesByNodeId(const string& node_id) {
        vector<EDGE> edges;
        for (const EDGE& edge : this->graph_edges) {
            if (edge.source_node_id == node_id) {
                edges.push_back(edge);
            }
        }
        return edges;
    }

    //判断节点是否在路径里（生成路径的内置函数）
    bool isNodeInPath(NODE& node, PATH& path) {
        for (NODE& existingNode : path.path_nodes) {
            if (existingNode.node_id == node.node_id)
                return true;
        }
        return false;
    }

    // 判断节点是否有除了环内边外的其它出边
    bool hasExternalOutEdge(NODE& node, PATH& currentPath) {
        std::vector<EDGE> outgoingEdges = getEdgesByNodeId(node.node_id);
        for (EDGE& edge : outgoingEdges) {
            // 如果边的目标节点不在当前路径中，或不在环的部分中，则认为是外部出边
            if (!isNodeInPath(getNodeById(edge.target_node_id), currentPath)) {
                return true;
            }
        }
        return false;
    }

    // 对一个图生成路径
    void generatePathRecursively(NODE& currentNode, PATH& currentPath) {
        // 检查当前节点是否已在路径中，若在，则表示存在环
        //std::ofstream file(output_loc, std::ios::app);  // 创建并以追加模式打开文件
        //if (!file) {
        //    std::cerr << "无法打开文件 output.txt 进行写入";
        //    return;
        //}

        if (isNodeInPath(currentNode, currentPath)) { // 如果有环（当前节点已经在路径里）
            currentPath.path_id = graph_id + "_" + std::to_string(++pathIdCounter) + "_cycle";
            paths.push_back(currentPath);

            // 判断是否会无限循环
            bool hasExternalEdge = false;
            int startIndex = 0;
            for (int i = 0; i < currentPath.path_nodes.size(); ++i) {
                if (currentPath.path_nodes[i].node_id == currentNode.node_id) {
                    startIndex = i; // 找到环开始的节点索引
                    break;
                }
            }

            // 检查环中的每个节点
            for (int i = startIndex; i < currentPath.path_nodes.size(); ++i) {
                if (hasExternalOutEdge(currentPath.path_nodes[i], currentPath)) {
                    hasExternalEdge = true;
                    break;
                }
            }
            //if (hasExternalEdge == false) {
            //    file << "无限循环！图ID: " << graph_id
            //        << ",  导致成环的节点ID: " << currentNode.node_id << endl;
            //}
            return; // 存在环则跳过本次递归
        }

        // 将当前节点添加到当前路径的节点集合中
        currentPath.path_nodes.push_back(currentNode);

        // 检查当前节点是否为终点（没有出度的节点）
        std::vector<EDGE> outgoingEdges = getEdgesByNodeId(currentNode.node_id);
        if (outgoingEdges.empty()) {
            // 如果是终点，则为当前路径生成唯一的 path_id
            currentPath.path_id = graph_id + "_" + std::to_string(++pathIdCounter);
            // 将当前路径保存进路径集合中
            paths.push_back(currentPath);
        }
        else {
            // 如果不是终点，则遍历每条出边
            for (EDGE& edge : outgoingEdges) {
                // 将出边保存进当前路径的边集合中
                currentPath.path_edges.push_back(edge);
                // 递归进入下一个节点
                generatePathRecursively(getNodeById(edge.target_node_id), currentPath);
                // 回溯：移除当前边，以尝试其他路径
                currentPath.path_edges.pop_back();
            }
        }
        // 回溯：移除当前节点
        currentPath.path_nodes.pop_back();
    }

    // 生成全部路径
    void generateAllPaths() {
        // 清空现有路径
        paths.clear();

        // 检查是否有起始节点
        if (!graph_nodes.empty()) {
            // 第一个节点作为起始节点
            NODE& startNode = graph_nodes.front();
            PATH initialPath;  // 创建一个初始路径，不包含任何节点和边
            generatePathRecursively(startNode, initialPath);  // 生成所有路径
        }
    }

    // 根据节点，返回节点的全部出边——检查分支同时满足的内部函数
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
    string name; // 数据名称
    string data_type; //数据类型（read/write）
    vector<RANGE> range; // 范围
};

// 以下为一些utils函数

// 从字符串A中去掉“B（xxx）”字符串（若有）以及之前的一个字符串（and/or，若有）
void removePattern(string& A, const string& B) {
    // 修改正则表达式，以覆盖以下情况：
    // 1. 如果 B 前有非空白字符和空白字符，将被匹配和移除。
    // 2. 匹配 B 及其括号内的内容。
    // 3. 如果 B 后面是空格（意味着后面可能有其他字符或表达式），那么这个空格也会被移除。
    // \s* 匹配任意数量的空白字符，确保移除前后的空格。
    string pattern = "\\s*(\\S+\\s+)?" + B + "\\([^()]*\\)\\s*";
    regex re(pattern);

    // 使用 regex_replace 来查找和替换匹配的模式
    A = regex_replace(A, re, "");
}

// 给指定目录的文件进行去重
void deduplicate(string path) {
    set<std::string> lines;  // 使用set来自动去重，因为set不允许有重复元素
    ifstream file_in(path);  // 打开文件进行读取
    string line;

    if (!file_in) {
        std::cerr << "无法打开文件进行读取" << std::endl;
        return;
    }

    // 读取文件的每一行，将其存储在集合中
    while (getline(file_in, line)) {
        lines.insert(line);
    }

    file_in.close();  // 完成读取后关闭文件

    std::ofstream file_out(path);  // 再次打开文件进行写入，这次是清空文件并写入新的无重复内容

    if (!file_out) {
        std::cerr << "无法打开文件进行写入" << std::endl;
        return;
    }

    // 遍历集合并将结果写回文件
    for (const auto& unique_line : lines) {
        file_out << unique_line << std::endl;
    }

    file_out.close();  // 完成写入后关闭文件

    return;
}

// 判断一个字符串是不是数字
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
            return true; // 允许空格
        }
        else {
            return false;
        }
        }) && digitSeen; // 确保至少有一个数字
}

// 给定两个范围，判断是否有交集
bool rangesIntersect(const RANGE& range1, const RANGE& range2) {
    // 首先，检测是否存在 not_equal_to 的情况
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

    // 检测是否存在不重叠的场景
    if ((range1.ub < range2.lb) ||
        (range1.ub == range2.lb && !(range1.is_ub_closed && range2.is_lb_closed))) {
        return false;
    }

    if ((range2.ub < range1.lb) ||
        (range2.ub == range1.lb && !(range2.is_ub_closed && range1.is_lb_closed))) {
        return false;
    }

    return true; // 默认认为有交集
}

// 合并各个RANGE（求并集）
RANGE unionOfRanges(vector<RANGE>& ranges) {
    // 检查是否有启用 not_equal_to 的情况
    for (const auto& range : ranges) {
        if (range.not_equal_to != std::numeric_limits<double>::infinity()) {
            double neq = range.not_equal_to;

            for (const auto& range2 : ranges) {
                if (range2.not_equal_to != std::numeric_limits<double>::infinity()) {
                    if (range2.not_equal_to != range.not_equal_to) {
                        return RANGE(); // 返回全体值域的范围
                    }
                }
                else {
                    if (range2.lb < neq || (range2.lb == neq && range2.is_lb_closed)) {
                        return RANGE(); // 返回全体值域的范围
                    }
                }
            }
            return range; // 返回包含 not_equal_to 的范围
        }
    }

    // 如果没有启用 not_equal_to 的情况
    double min_lb = std::numeric_limits<double>::infinity();
    double max_ub = -std::numeric_limits<double>::infinity();
    bool lb_closed = false;
    bool ub_closed = false;

    // 找出最小的下界和最大的上界
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

// 求给定RANGE的补集
RANGE get_complement(const RANGE& range) {
    RANGE new_range;

    // 不等于
    if (range.not_equal_to != std::numeric_limits<double>::infinity()) {
        new_range.lb = range.not_equal_to;
        new_range.ub = range.not_equal_to;
        new_range.is_lb_closed = true;
        new_range.is_ub_closed = true;
    }
    // 等于
    else if (range.lb == range.ub) {
        new_range.not_equal_to = range.lb;
    }
    // 大于（等于）
    else if (range.ub == std::numeric_limits<double>::infinity()) {
        new_range.ub = range.lb;
        new_range.is_ub_closed = !range.is_lb_closed;
    }
    // 小于（等于）
    else if (range.lb == -std::numeric_limits<double>::infinity()) {
        new_range.lb = range.ub;
        new_range.is_lb_closed = !range.is_ub_closed;
    }

    return new_range;
}

// 从字符串中删除子串
void removeSubstrings(std::string& mainString, const std::vector<std::string>& subStrings) {
    for (const auto& pattern : subStrings) {
        size_t pos = 0;
        while ((pos = mainString.find(pattern, pos)) != std::string::npos) {
            mainString.replace(pos, pattern.length(), "");
        }
    }
}

class GRAPHS { //需求图集合
private:
    vector<GRAPH> graphs;
    vector<DIC_DATA> data_dic;
public:
    // 以下为各类主要函数的内部辅助函数

    // 初始化函数的内部函数：尝试添加数据到data_dic（给data_dic赋值）
    void addDataToDic(vector<DIC_DATA>& data_dic, DATA& data) {
        // 检查data_dic中是否已存在该数据名称
        auto it = find_if(data_dic.begin(), data_dic.end(), [&](const DIC_DATA& dic_data) {
            return dic_data.name == data.name;
            });

        if (it == data_dic.end()) { // 如果数据不存在，则添加新数据到data_dic
            DIC_DATA new_data;
            new_data.name = data.name;
            new_data.possibleValues.push_back(data.range.lb); // 添加可能的值
            data_dic.push_back(new_data);
        }
        else { // 如果已存在，检查值是否已在possibleValues中，如果没有，则添加        
            if (find(it->possibleValues.begin(), it->possibleValues.end(), data.range.lb) == it->possibleValues.end()) {
                it->possibleValues.push_back(data.range.lb);
            }
        }
    }

    // 初始化函数的内部函数：将data送入vector<DATA>中，送入之前检查data是否为含有运算符的复合数据，若是则处理后送入
    void input_data(const DATA& data, vector<DATA>& datas) {
        string operators = "+-*/";
        size_t pos = data.name.find_first_of(operators);

        if (pos != string::npos) {
            // 处理含有运算符的复合数据
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
            // 不含运算符的情况，检查是否是数字，如果不是则添加到datas
            if (!isNumber(data.name)) {
                datas.push_back(data);
            }
        }
    }

    // 需求缺失检测的内部函数：判断数据是否有外部制造方
    bool check_data_has_out_producer(const DATA& data1, const GRAPH& sourceGraph) {
        // 遍历每个图
        for (const GRAPH& graph : graphs) {
            // 跳过数据所在的相同图
            if (&graph == &sourceGraph) continue;

            // 遍历图中的每个节点
            for (const NODE& node : graph.graph_nodes) {
                // 检查节点是否为写类型
                if (node.node_type == "write") {
                    // 遍历节点的数据
                    for (const DATA& data2 : node.node_datas) {
                        // 检查数据是否为写类型且名称匹配，并且范围有交集
                        if (data2.data_type == "write" && data2.name == data1.name && rangesIntersect(data1.range, data2.range)) {
                            return true; // 找到了匹配的写类型数据
                        }
                    }
                }
            }
        }

        return false; // 没有找到匹配的写类型数据
    }

    // 需求缺失检测的内部函数：判断数据是否有外部消费方
    bool check_data_has_out_consumer(const DATA& data1, const GRAPH& sourceGraph) {
        // 遍历每个图
        for (const GRAPH& graph : graphs) {
            // 跳过数据所在的相同图
            if (&graph == &sourceGraph) continue;

            // 遍历图中的节点
            for (const NODE& node : graph.graph_nodes) {
                // 遍历节点中的数据
                for (const DATA& data2 : node.node_datas) {
                    // 检查数据是否为读类型且名称匹配，并且范围有交集
                    if (data2.data_type == "read" && data2.name == data1.name && rangesIntersect(data1.range, data2.range)) {
                        return true; // 找到了匹配的读类型数据
                    }
                }
            }

            // 遍历图中的边
            for (const EDGE& edge : graph.graph_edges) {
                // 遍历边中的数据
                for (const DATA& data2 : edge.edge_datas) {
                    // 检查名称匹配，并且范围有交集
                    if (data2.name == data1.name && rangesIntersect(data1.range, data2.range)) {
                        return true; // 找到了匹配的数据
                    }
                }
            }
        }

        return false; // 没有找到匹配的数据
    }


    // 需求缺失检测的内部函数：判断是否至少存在一条路径，其中制造方节点在本节点之前
    bool isProducerAtOrBeforeNode(const GRAPH& graph, const NODE& consumerNode, const DATA& data) {
        for (const PATH& path : graph.paths) {
            int producerIndex = -1;
            int consumerIndex = -1;

            // 在每条路径中搜索指定的数据的生产者节点和当前节点（消费者）
            for (int i = 0; i < path.path_nodes.size(); ++i) {
                const NODE& currentNode = path.path_nodes[i];

                // 查找具有相同数据的生产者节点（写入操作）
                for (const DATA& nodeData : currentNode.node_datas) {
                    if (nodeData.name == data.name && nodeData.data_type == "write" && rangesIntersect(data.range, nodeData.range)) {
                        producerIndex = i;  // 记录生产者节点的索引
                        break;  // 找到生产者节点后，可以提前结束内层循环
                    }
                }

                // 确认当前遍历到的节点是否为传入的消费者节点
                if (&currentNode == &consumerNode) {
                    consumerIndex = i; // 记录消费者节点的索引
                    break;  // 找到消费者节点后，无需继续遍历当前路径，可以提前结束外层循环
                }
            }

            // 如果在同一路径中找到了相应的生产者和消费者，并且生产者在消费者之前，返回true
            if (producerIndex != -1 && consumerIndex != -1 && producerIndex < consumerIndex) {
                return true;
            }
        }

        // 如果所有路径中都没有找到满足条件的生产者在消费者之前的情况，返回false
        return false;
    }


    // 需求缺失检测的内部函数：判断是否至少存在一条路径，其中制造方节点在本边之前
    bool isProducerAtOrBeforeEdge(const GRAPH& graph, const EDGE& edge, const DATA& data) {
        for (const PATH& path : graph.paths) {
            int producerIndex = -1;
            int edgeStartIndex = -1;

            // 在路径中遍历节点，寻找生产者节点和边的起点节点
            for (int i = 0; i < path.path_nodes.size(); ++i) {
                const NODE& currentNode = path.path_nodes[i];

                // 检查节点中的数据是否符合作为生产者的条件
                for (const DATA& nodeData : currentNode.node_datas) {
                    if (nodeData.name == data.name && nodeData.data_type == "write" && rangesIntersect(data.range, nodeData.range)) {
                        producerIndex = i; // 记录生产者节点的索引
                        break; // 找到符合条件的生产者后，可以提前结束内层循环
                    }
                }

                // 确定边的起点节点的索引
                if (currentNode.node_id == edge.source_node_id) {
                    edgeStartIndex = i;
                    break; // 找到边的起点后，无需继续遍历当前路径，因为我们只需确保生产者在起点或之前
                }
            }

            // 如果找到生产者，并且生产者位于边的起点或之前，返回true
            if (producerIndex != -1 && edgeStartIndex != -1 && producerIndex <= edgeStartIndex) {
                return true;
            }
        }

        // 如果所有路径中都没有找到满足条件的生产者在边的起点或之前的情况，返回false
        return false;
    }


    // SMT相关的内部函数：解析单个条件表达式，如"x < 10"
    z3::expr parseSimpleConditionToExpr(const std::string& condition, z3::context& c) {
        std::istringstream iss(condition);
        std::string varName, op;
        int value;

        iss >> varName >> op >> value;

        z3::expr var = c.int_const(varName.c_str()); // 变量对象
        z3::expr val = c.int_val(value); // 值对象

        // 使用 switch-case 简化多个条件的判断
        if (op == ">") return var > val;
        if (op == "<") return var < val;
        if (op == "==") return var == val;
        if (op == ">=") return var >= val;
        if (op == "<=") return var <= val;
        if (op == "!=") return var != val;

        // 如果没有匹配的操作符，可以考虑抛出异常或返回一个默认的无效表达式
        throw std::invalid_argument("Unsupported operator: " + op);
    }


    // SMT相关的内部函数：根据字符串返回完整的z3表达式
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
                // 处理括号内的表达式
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
                // 递归处理括号内的表达式
                z3::expr e = parseConditionToExpr(subExpr, c);
                exprs.push_back(e);
            }
            else {
                // 处理简单条件表达式
                std::string simpleExpr = token;
                iss >> token; // 读取操作符
                simpleExpr += " " + token;
                iss >> token; // 读取值
                simpleExpr += " " + token;

                z3::expr e = parseSimpleConditionToExpr(simpleExpr, c);
                exprs.push_back(e);
            }
        }

        // 根据逻辑运算符构建复合表达式
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


    // SMT相关的内部函数：提取表达式中变量的辅助函数
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

    // SMT相关的内部函数：提取表达式中的所有变量
    std::vector<z3::expr> getVariables(const z3::expr& e) {
        std::set<z3::expr> variables;
        collectVariables(e, variables);
        return std::vector<z3::expr>(variables.begin(), variables.end());
    }

    // SMT相关的内部函数：根据 Z3 变量的名称查找对应的 RANGE 对象，返回两者的map
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

    // 函数：根据 RANGE 对象构建 Z3 表达式
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

    // 函数：根据变量范围构建 Z3 表达式
    z3::expr buildConstraints(z3::context& c, const std::map<z3::expr, RANGE>& variableRanges) {
        z3::expr_vector constraints(c);

        for (std::map<z3::expr, RANGE>::const_iterator it = variableRanges.begin(); it != variableRanges.end(); ++it) {
            const z3::expr& var = it->first;
            const RANGE& range = it->second;
            constraints.push_back(buildRangeExpression(c, var, range));
        }

        return mk_and(constraints);
    }


    // 分支同时满足的内部函数：传入各个edges，判断各条edges能否被同时满足
    bool checkEdgesForSatisfiability(const vector<EDGE>& edges) {
        z3::context c;
        z3::solver s(c);

        for (const auto& edge : edges) {
            // 解析每条边的edge_information，并构建Z3表达式
            z3::expr conditionExpr = parseConditionToExpr(edge.edge_information, c);
            s.add(conditionExpr);
        }

        return s.check() == z3::sat;
    }

    // 以下为需求图的基本函数

    // 传入图信息，对图信息进行修改/添加——如果传入的modify_all为false，说明是局部修改；否则为整体初始化
    void requirement_diagram_analysis(const json& received_data, httplib::Response& res) {
        GRAPH temp_graph; // 当前读取的需求图对象
        NODE temp_node; // 当前读取的节点对象
        EDGE temp_edge; // 当前读取的边对象
        DATA temp_data; // 当前读取的数据对象
        vector<GRAPH> temp_graphs; // 本次有修改/添加的所有图
        // 检查 method 字段
        bool modify_all = received_data.value("modify_all", false);  // 默认为 false
        // 打印 method 字段值
        std::cout << "modify_all: " << (modify_all ? "True" : "False") << std::endl;
        if (modify_all == false) { // 只修改部分
            for (const auto& graph : received_data["graphs"]) {
                // 遍历 "graphs" 数组中的每个图
                bool exist = false;
                for (GRAPH& Graph : graphs) {
                    if (graph["graph_id"] == Graph.graph_id) {
                        Graph.clear();
                        exist = true;
                        for (const auto& node : graph["nodes"]) {
                            temp_node.node_id = node["node_id"];
                            temp_node.node_type = node["node_type"];
                            temp_node.node_information = node["node_information"];
                            // 判断是否有 "branch_yes" 和 "branch_no" 字段
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
                if (exist == false) { // 如果不是已经存在的图
                    temp_graph.graph_id = graph["graph_id"];
                    for (const auto& node : graph["nodes"]) {
                        temp_node.node_id = node["node_id"];
                        temp_node.node_type = node["node_type"];
                        temp_node.node_information = node["node_information"];
                        // 判断是否有 "branch_yes" 和 "branch_no" 字段
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

            // 使用information对datas赋值
            for (auto& graph : temp_graphs) { // 遍历图
                // 遍历图中所有节点并初步赋值（此时会将诸如a+b-c视为一个整体数据并保存）
                for (auto& node : graph.graph_nodes) {
                    // 创建节点中信息的副本并删除左右括号与“duration”（若有）
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

                    // 以空格为分隔符读取每个字符串
                    while (iss >> s) {
                        if (s == "and" || s == "or") {
                            iss >> s;
                        }
                        data_temp1.name = s;
                        data_temp1.data_type = node.node_type;
                        iss >> s;
                        if (s == ">") {
                            iss >> s;
                            if (isNumber(s) == true) { // 说明读一个数据，指明了具体数值
                                data_temp1.range = RANGE(double(stoi(s)), positiveInfinity, false, false, positiveInfinity);
                                input_data(data_temp1, node.node_datas); // 将data送入vector<DATA>中，送入之前检查data是否为含有运算符的复合数据，若是则处理后送入
                            }
                            else { // 说明读两个数据，数值均未知
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, node.node_datas);
                                input_data(data_temp2, node.node_datas);
                            }
                        }
                        else if (s == ">=") {
                            iss >> s;
                            if (isNumber(s) == true) { // 说明读一个数据，指明了具体数值
                                data_temp1.range = RANGE(double(stoi(s)), positiveInfinity, true, false, positiveInfinity);
                                input_data(data_temp1, node.node_datas);
                            }
                            else { // 说明读两个数据，数值均未知
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, node.node_datas);
                                input_data(data_temp2, node.node_datas);
                            }
                        }
                        else if (s == "<") {
                            iss >> s;
                            if (isNumber(s) == true) { // 说明读一个数据，指明了具体数值
                                data_temp1.range = RANGE(negativeInfinity, double(stoi(s)), false, false, positiveInfinity);
                                input_data(data_temp1, node.node_datas);
                            }
                            else { // 说明读两个数据，数值均未知
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, node.node_datas);
                                input_data(data_temp2, node.node_datas);
                            }
                        }
                        else if (s == "<=") {
                            iss >> s;
                            if (isNumber(s) == true) { // 说明读一个数据，指明了具体数值
                                data_temp1.range = RANGE(negativeInfinity, double(stoi(s)), false, true, positiveInfinity);
                                input_data(data_temp1, node.node_datas);
                            }
                            else { // 说明读两个数据，数值均未知
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, node.node_datas);
                                input_data(data_temp2, node.node_datas);
                            }
                        }
                        else if (s == "!=") {
                            iss >> s;
                            if (isNumber(s) == true) { // 说明读一个数据，指明了具体数值
                                data_temp1.range = RANGE(negativeInfinity, positiveInfinity, false, false, double(stoi(s)));
                                input_data(data_temp1, node.node_datas);
                            }
                            else { // 说明读/写多个数据，数值均未知
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, node.node_datas);
                                input_data(data_temp2, node.node_datas);
                            }
                        }
                        else if (s == "==" || s == "=") {
                            iss >> s;
                            if (isNumber(s) == true) { // 说明读一个数据，指明了具体数值
                                data_temp1.range = RANGE(double(stoi(s)), double(stoi(s)), true, true, positiveInfinity);
                                input_data(data_temp1, node.node_datas);
                            }
                            else { // 说明读/写多个数据，数值均未知
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, node.node_datas);
                                input_data(data_temp2, node.node_datas);
                            }
                        }
                    }
                }

                // 遍历图中所有边
                for (auto& edge : graph.graph_edges) {
                    // 创建边中信息的副本并删除左右括号与“duration”（若有）
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

                    // 以空格为分隔符读取每个字符串
                    while (iss >> s) {
                        if (s == "and" || s == "or") {
                            iss >> s;
                        }
                        data_temp1.name = s;
                        data_temp1.data_type = "read";
                        iss >> s;
                        if (s == ">") {
                            iss >> s;
                            if (isNumber(s) == true) { // 说明读一个数据，指明了具体数值
                                data_temp1.range = RANGE(double(stoi(s)), positiveInfinity, false, false, positiveInfinity);
                                input_data(data_temp1, edge.edge_datas);
                            }
                            else { // 说明读两个数据，数值均未知
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, edge.edge_datas);
                                input_data(data_temp2, edge.edge_datas);
                            }
                        }
                        else if (s == ">=") {
                            iss >> s;
                            if (isNumber(s) == true) { // 说明读一个数据，指明了具体数值
                                data_temp1.range = RANGE(double(stoi(s)), positiveInfinity, true, false, positiveInfinity);
                                input_data(data_temp1, edge.edge_datas);
                            }
                            else { // 说明读两个数据，数值均未知
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, edge.edge_datas);
                                input_data(data_temp2, edge.edge_datas);
                            }
                        }
                        else if (s == "<") {
                            iss >> s;
                            if (isNumber(s) == true) { // 说明读一个数据，指明了具体数值
                                data_temp1.range = RANGE(negativeInfinity, double(stoi(s)), false, false, positiveInfinity);
                                input_data(data_temp1, edge.edge_datas);
                            }
                            else { // 说明读两个数据，数值均未知
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, edge.edge_datas);
                                input_data(data_temp2, edge.edge_datas);
                            }
                        }
                        else if (s == "<=") {
                            iss >> s;
                            if (isNumber(s) == true) { // 说明读一个数据，指明了具体数值
                                data_temp1.range = RANGE(negativeInfinity, double(stoi(s)), false, true, positiveInfinity);
                                input_data(data_temp1, edge.edge_datas);
                            }
                            else { // 说明读两个数据，数值均未知
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, edge.edge_datas);
                                input_data(data_temp2, edge.edge_datas);
                            }
                        }
                        else if (s == "!=") {
                            iss >> s;
                            if (isNumber(s) == true) { // 说明读一个数据，指明了具体数值
                                data_temp1.range = RANGE(negativeInfinity, positiveInfinity, false, false, double(stoi(s)));
                                input_data(data_temp1, edge.edge_datas);
                            }
                            else { // 说明读/写多个数据，数值均未知
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, edge.edge_datas);
                                input_data(data_temp2, edge.edge_datas);
                            }
                        }
                        else if (s == "==" || s == "=") {
                            iss >> s;
                            if (isNumber(s) == true) { // 说明读一个数据，指明了具体数值
                                data_temp1.range = RANGE(double(stoi(s)), double(stoi(s)), true, true, positiveInfinity);
                                input_data(data_temp1, edge.edge_datas);
                            }
                            else { // 说明读/写多个数据，数值均未知
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, edge.edge_datas);
                                input_data(data_temp2, edge.edge_datas);
                            }
                        }
                    }
                }
            }

            // 对paths赋值——将节点与边加入path
            for (auto& graph : temp_graphs) {
                graph.generateAllPaths();
            }
        }
        else { // 全部传入并修改
            graphs.clear();
            for (const auto& graph : received_data["graphs"]) {
                // 遍历 "graphs" 数组中的每个图
                temp_graph.clear();
                temp_graph.graph_id = graph["graph_id"];
                for (const auto& node : graph["nodes"]) {
                    temp_node.clear();
                    temp_node.node_id = node["node_id"];
                    temp_node.node_type = node["node_type"];
                    temp_node.node_information = node["node_information"];
                    // 判断是否有 "branch_yes" 和 "branch_no" 字段
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

            // 使用information对datas赋值
            for (auto& graph : graphs) { // 遍历图
                // 遍历图中所有节点并初步赋值（此时会将诸如a+b-c视为一个整体数据并保存）
                for (auto& node : graph.graph_nodes) {
                    // 创建节点中信息的副本并删除左右括号与“duration”（若有）
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

                    // 以空格为分隔符读取每个字符串
                    while (iss >> s) {
                        if (s == "and" || s == "or") {
                            iss >> s;
                        }
                        data_temp1.name = s;
                        data_temp1.data_type = node.node_type;
                        iss >> s;
                        if (s == ">") {
                            iss >> s;
                            if (isNumber(s) == true) { // 说明读一个数据，指明了具体数值
                                data_temp1.range = RANGE(double(stoi(s)), positiveInfinity, false, false, positiveInfinity);
                                input_data(data_temp1, node.node_datas); // 将data送入vector<DATA>中，送入之前检查data是否为含有运算符的复合数据，若是则处理后送入
                            }
                            else { // 说明读两个数据，数值均未知
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, node.node_datas);
                                input_data(data_temp2, node.node_datas);
                            }
                        }
                        else if (s == ">=") {
                            iss >> s;
                            if (isNumber(s) == true) { // 说明读一个数据，指明了具体数值
                                data_temp1.range = RANGE(double(stoi(s)), positiveInfinity, true, false, positiveInfinity);
                                input_data(data_temp1, node.node_datas);
                            }
                            else { // 说明读两个数据，数值均未知
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, node.node_datas);
                                input_data(data_temp2, node.node_datas);
                            }
                        }
                        else if (s == "<") {
                            iss >> s;
                            if (isNumber(s) == true) { // 说明读一个数据，指明了具体数值
                                data_temp1.range = RANGE(negativeInfinity, double(stoi(s)), false, false, positiveInfinity);
                                input_data(data_temp1, node.node_datas);
                            }
                            else { // 说明读两个数据，数值均未知
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, node.node_datas);
                                input_data(data_temp2, node.node_datas);
                            }
                        }
                        else if (s == "<=") {
                            iss >> s;
                            if (isNumber(s) == true) { // 说明读一个数据，指明了具体数值
                                data_temp1.range = RANGE(negativeInfinity, double(stoi(s)), false, true, positiveInfinity);
                                input_data(data_temp1, node.node_datas);
                            }
                            else { // 说明读两个数据，数值均未知
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, node.node_datas);
                                input_data(data_temp2, node.node_datas);
                            }
                        }
                        else if (s == "!=") {
                            iss >> s;
                            if (isNumber(s) == true) { // 说明读一个数据，指明了具体数值
                                data_temp1.range = RANGE(negativeInfinity, positiveInfinity, false, false, double(stoi(s)));
                                input_data(data_temp1, node.node_datas);
                            }
                            else { // 说明读/写多个数据，数值均未知
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, node.node_datas);
                                input_data(data_temp2, node.node_datas);
                            }
                        }
                        else if (s == "==" || s == "=") {
                            iss >> s;
                            if (isNumber(s) == true) { // 说明读一个数据，指明了具体数值
                                data_temp1.range = RANGE(double(stoi(s)), double(stoi(s)), true, true, positiveInfinity);
                                input_data(data_temp1, node.node_datas);
                            }
                            else { // 说明读/写多个数据，数值均未知
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, node.node_datas);
                                input_data(data_temp2, node.node_datas);
                            }
                        }
                    }
                }

                // 遍历图中所有边
                for (auto& edge : graph.graph_edges) {
                    // 创建边中信息的副本并删除左右括号与“duration”（若有）
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

                    // 以空格为分隔符读取每个字符串
                    while (iss >> s) {
                        if (s == "and" || s == "or") {
                            iss >> s;
                        }
                        data_temp1.name = s;
                        data_temp1.data_type = "read";
                        iss >> s;
                        if (s == ">") {
                            iss >> s;
                            if (isNumber(s) == true) { // 说明读一个数据，指明了具体数值
                                data_temp1.range = RANGE(double(stoi(s)), positiveInfinity, false, false, positiveInfinity);
                                input_data(data_temp1, edge.edge_datas);
                            }
                            else { // 说明读两个数据，数值均未知
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, edge.edge_datas);
                                input_data(data_temp2, edge.edge_datas);
                            }
                        }
                        else if (s == ">=") {
                            iss >> s;
                            if (isNumber(s) == true) { // 说明读一个数据，指明了具体数值
                                data_temp1.range = RANGE(double(stoi(s)), positiveInfinity, true, false, positiveInfinity);
                                input_data(data_temp1, edge.edge_datas);
                            }
                            else { // 说明读两个数据，数值均未知
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, edge.edge_datas);
                                input_data(data_temp2, edge.edge_datas);
                            }
                        }
                        else if (s == "<") {
                            iss >> s;
                            if (isNumber(s) == true) { // 说明读一个数据，指明了具体数值
                                data_temp1.range = RANGE(negativeInfinity, double(stoi(s)), false, false, positiveInfinity);
                                input_data(data_temp1, edge.edge_datas);
                            }
                            else { // 说明读两个数据，数值均未知
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, edge.edge_datas);
                                input_data(data_temp2, edge.edge_datas);
                            }
                        }
                        else if (s == "<=") {
                            iss >> s;
                            if (isNumber(s) == true) { // 说明读一个数据，指明了具体数值
                                data_temp1.range = RANGE(negativeInfinity, double(stoi(s)), false, true, positiveInfinity);
                                input_data(data_temp1, edge.edge_datas);
                            }
                            else { // 说明读两个数据，数值均未知
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, edge.edge_datas);
                                input_data(data_temp2, edge.edge_datas);
                            }
                        }
                        else if (s == "!=") {
                            iss >> s;
                            if (isNumber(s) == true) { // 说明读一个数据，指明了具体数值
                                data_temp1.range = RANGE(negativeInfinity, positiveInfinity, false, false, double(stoi(s)));
                                input_data(data_temp1, edge.edge_datas);
                            }
                            else { // 说明读/写多个数据，数值均未知
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, edge.edge_datas);
                                input_data(data_temp2, edge.edge_datas);
                            }
                        }
                        else if (s == "==" || s == "=") {
                            iss >> s;
                            if (isNumber(s) == true) { // 说明读一个数据，指明了具体数值
                                data_temp1.range = RANGE(double(stoi(s)), double(stoi(s)), true, true, positiveInfinity);
                                input_data(data_temp1, edge.edge_datas);
                            }
                            else { // 说明读/写多个数据，数值均未知
                                data_temp2.data_type = "read";
                                data_temp2.name = s;
                                input_data(data_temp1, edge.edge_datas);
                                input_data(data_temp2, edge.edge_datas);
                            }
                        }
                    }
                }
            }

            // 对paths赋值——将节点与边加入path
            for (auto& graph : graphs) {
                graph.generateAllPaths();
            }
        }

        // 创建一个 JSON 对象
        nlohmann::json response_data;
        response_data["state"] = "data update success!";




        // 创建一个数组用于存放每个图的路径字典
        nlohmann::json paths_array = nlohmann::json::array();

        // 遍历每个图
        for (const auto& graph : graphs) {
            nlohmann::json graph_data;
            graph_data["graph_id"] = graph.graph_id;

            // 创建一个数组存放该图中的路径
            nlohmann::json paths_in_graph = nlohmann::json::array();

            // 遍历该图中的每条路径
            for (const auto& path : graph.paths) {
                nlohmann::json path_data;
                path_data["path_id"] = path.path_id;

                // 创建一个数组存放路径中的节点、条件和边
                nlohmann::json nodes_conditions_transitions_json = nlohmann::json::array();

                // 遍历路径中的每个节点
                for (size_t i = 0; i < path.path_nodes.size(); ++i) {
                    const auto& node = path.path_nodes[i];

                    // 如果是read类型节点（条件判断）
                    if (node.node_type == "read") {
                        nlohmann::json condition_data;
                        condition_data["condition_id"] = node.node_id;
                        condition_data["condition_information"] = node.node_information;

                        // 添加条件的分支信息
                        condition_data["branch_yes"] = node.branch_yes;
                        condition_data["branch_no"] = node.branch_no;

                        // 将条件数据添加到节点之间
                        nodes_conditions_transitions_json.push_back(condition_data);
                    }

                    // 如果是write类型节点（赋值操作），直接添加
                    else if (node.node_type == "write") {
                        // 节点信息
                        nlohmann::json node_data;
                        node_data["node_id"] = node.node_id;
                        node_data["node_information"] = node.node_information;
                        // 对于写操作，直接输出节点数据
                        nodes_conditions_transitions_json.push_back(node_data);
                    }

                    // 对于每个节点，必须通过一条边（transition）连接到下一个节点
                    // 找到连接当前节点的边（transition）
                    for (const auto& edge : path.path_edges) {
                        if (node.node_type == "write") { //如果是写节点，通过边来得到对应的节点
                            if (edge.source_node_id == node.node_id) {
                                nlohmann::json transition_data;
                                transition_data["transition_id"] = edge.edge_id;
                                transition_data["transition_information"] = edge.edge_information;

                                // 将边数据添加到路径中
                                nodes_conditions_transitions_json.push_back(transition_data);
                            }
                        }
                        else if (node.node_type == "read") { // 否则根据节点的branch来得到对应的边
                            if (edge.edge_id == node.branch_yes || edge.edge_id == node.branch_no) {
                                nlohmann::json transition_data;
                                transition_data["transition_id"] = edge.edge_id;
                                transition_data["transition_information"] = edge.edge_information;

                                // 将边数据添加到路径中
                                nodes_conditions_transitions_json.push_back(transition_data);
                            }
                        }
                    }
                }

                // 将路径中的节点、条件和边信息添加到路径数据中
                path_data["nodes and conditions and transitions"] = nodes_conditions_transitions_json;

                // 将路径数据添加到该图的路径数组中
                paths_in_graph.push_back(path_data);
            }

            // 将该图的路径信息添加到主路径数组中
            graph_data["paths"] = paths_in_graph;
            paths_array.push_back(graph_data);
        }

        // 将路径字典添加到response_data中
        response_data["paths"] = paths_array;



        // 设置响应类型为 JSON，并将 JSON 内容作为响应
        res.set_content(response_data.dump(), "application/json");  // .dump() 将 JSON 对象转换为字符串

    }

    // 返回全部依赖关系
    void return_requirements(httplib::Response& res) {
        // 创建一个 JSON 对象
        nlohmann::json response_data;

        int count = 0;

        // 创建一个 JSON 数组来存储依赖关系
        nlohmann::json dependencies = nlohmann::json::array();


        // 遍历 graphs 集合中的每个图
        for (int i = 0; i < graphs.size(); ++i) {
            GRAPH& currentGraph = graphs[i];
            
            // 遍历当前图中的每条路径
            for (PATH& path : currentGraph.paths) {
                // 遍历路径上的所有节点
                for (NODE& node : path.path_nodes) { 
                    // 如果是写节点
                    if (node.node_type == "write") {
                        for (DATA& data : node.node_datas) { // 遍历节点的数据
                            if (data.data_type == "read") { // 如果是读数据
                                string readName = data.name; // 获取读节点数据的名称

                                // 遍历除当前图以外的其他图
                                for (int j = 0; j < graphs.size(); ++j) {
                                    if (i == j) continue; // 跳过当前图

                                    GRAPH& otherGraph = graphs[j];

                                    // 遍历其他图中的路径
                                    for (PATH& otherPath : otherGraph.paths) {
                                        for (NODE& writeNode : otherPath.path_nodes) { // 遍历路径上的所有节点
                                            for (DATA& writeData : writeNode.node_datas) { // 遍历写节点的数据
                                                if (writeNode.node_type == "write" && writeData.data_type == "write" && writeData.name == readName) { // 如果是写节点的写数据且名称与读数据名称相同
                                                    RANGE readRange = data.range; // 获取读节点数据的范围
                                                    RANGE writeRange = writeData.range; // 获取写节点数据的范围

                                                    count++;

                                                    // 创建依赖关系的 JSON 对象
                                                    nlohmann::json dependency;
                                                    dependency["dependent_path_id"] = path.path_id;
                                                    dependency["depended_path_id"] = otherPath.path_id;
                                                    dependency["data_name"] = readName;

                                                    // 添加依赖数据范围
                                                    if (readRange.not_equal_to != positiveInfinity) {
                                                        dependency["dependent_range"] = "!=" + to_string(readRange.not_equal_to);
                                                    }
                                                    else {
                                                        dependency["dependent_range"] = (readRange.is_lb_closed ? "[" : "(") +
                                                            to_string(readRange.lb) + ", " + to_string(readRange.ub) + (readRange.is_ub_closed ? "]" : ")");
                                                    }

                                                    // 添加被依赖数据范围
                                                    if (writeRange.not_equal_to != positiveInfinity) {
                                                        dependency["depended_range"] = "!=" + to_string(writeRange.not_equal_to);
                                                    }
                                                    else {
                                                        dependency["depended_range"] = (writeRange.is_lb_closed ? "[" : "(") +
                                                            to_string(writeRange.lb) + ", " + to_string(writeRange.ub) + (writeRange.is_ub_closed ? "]" : ")");
                                                    }
                                                    dependency["dependent_node_id"] = node.node_id;
                                                    dependency["depended_node_id"] = writeNode.node_id;

                                                    // 将依赖关系添加到数组中
                                                    dependencies.push_back(dependency);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // 如果是读节点（读节点的yes和no分支需要额外处理）
                    else if (node.node_type == "read") {
                        for (DATA& data : node.node_datas) { // 遍历节点的数据
                            string readName = data.name; // 获取读节点数据的名称(一定为读数据)

                            // 遍历除当前图以外的其他图
                            for (int j = 0; j < graphs.size(); ++j) {
                                if (i == j) continue; // 跳过当前图

                                GRAPH& otherGraph = graphs[j];

                                // 遍历其他图中的路径
                                for (PATH& otherPath : otherGraph.paths) {
                                    for (NODE& writeNode : otherPath.path_nodes) { // 遍历路径上的所有节点
                                        for (DATA& writeData : writeNode.node_datas) { // 遍历写节点的数据
                                            if (writeNode.node_type == "write" && writeData.data_type == "write" && writeData.name == readName) { // 如果是写节点的写数据且名称与读数据名称相同
                                                RANGE readRange = data.range; // 获取读数据的范围
                                                RANGE writeRange = writeData.range; // 获取写节点数据的范围

                                                // 若有交集，说明为yes分支
                                                if (rangesIntersect(writeData.range, data.range)) {
                                                    bool exist = false;
                                                    for (auto edge : path.path_edges) {
                                                        if (edge.edge_id == node.branch_yes) {
                                                            exist = true;
                                                            break;
                                                        }
                                                    }

                                                    // 若读节点的yes出边在路径中，说明应该输出依赖
                                                    if (exist) {
                                                        // 创建依赖关系的 JSON 对象
                                                        nlohmann::json dependency;
                                                        dependency["dependent_path_id"] = path.path_id;
                                                        dependency["depended_path_id"] = otherPath.path_id;
                                                        dependency["data_name"] = readName;

                                                        // 添加依赖数据范围
                                                        dependency["dependent_range"] = (readRange.is_lb_closed ? "[" : "(") +
                                                            to_string(readRange.lb) + ", " + to_string(readRange.ub) + (readRange.is_ub_closed ? "]" : ")");
                                                        dependency["depended_range"] = (writeRange.is_lb_closed ? "[" : "(") +
                                                            to_string(writeRange.lb) + ", " + to_string(writeRange.ub) + (writeRange.is_ub_closed ? "]" : ")");
                                                        dependency["dependent_node_id"] = node.node_id;
                                                        dependency["depended_node_id"] = writeNode.node_id;

                                                        // 将依赖关系添加到数组中
                                                        dependencies.push_back(dependency);
                                                    }
                                                }
                                                // 否则为no分支
                                                else {
                                                    bool exist = false;
                                                    for (auto edge : path.path_edges) {
                                                        if (edge.edge_id == node.branch_no) {
                                                            exist = true;
                                                            break;
                                                        }
                                                    }

                                                    // 若读节点的no出边在路径中，说明应该输出依赖，且要求数据范围的补集
                                                    if (exist) {
                                                        RANGE new_range = get_complement(readRange);

                                                        // 创建依赖关系的 JSON 对象
                                                        nlohmann::json dependency;
                                                        dependency["dependent_path_id"] = path.path_id;
                                                        dependency["depended_path_id"] = otherPath.path_id;
                                                        dependency["data_name"] = readName;

                                                        // 添加依赖数据范围
                                                        dependency["dependent_range"] = (new_range.is_lb_closed ? "[" : "(") +
                                                            to_string(new_range.lb) + ", " + to_string(new_range.ub) + (new_range.is_ub_closed ? "]" : ")");
                                                        dependency["depended_range"] = (writeRange.is_lb_closed ? "[" : "(") +
                                                            to_string(writeRange.lb) + ", " + to_string(writeRange.ub) + (writeRange.is_ub_closed ? "]" : ")");
                                                        dependency["dependent_node_id"] = node.node_id;
                                                        dependency["depended_node_id"] = writeNode.node_id;

                                                        // 将依赖关系添加到数组中
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
                for (EDGE& edge : path.path_edges) { // 遍历路径中所有的边
                    for (DATA& data : edge.edge_datas) { // 遍历边上的数据
                        if (data.data_type == "read") { // 如果是读数据
                            string readName = data.name; // 获取读数据的名称

                            // 遍历除当前图以外的其他图
                            for (int j = 0; j < graphs.size(); ++j) {
                                if (i == j) continue; // 跳过当前图

                                GRAPH& otherGraph = graphs[j];

                                // 遍历其他图中的路径
                                for (PATH& otherPath : otherGraph.paths) {
                                    for (NODE& writeNode : otherPath.path_nodes) { // 遍历路径上的所有节点
                                        for (DATA& writeData : writeNode.node_datas) { // 遍历写节点的数据
                                            if (writeNode.node_type == "write" && writeData.data_type == "write" && writeData.name == readName) { // 如果是写节点的写数据且名称与读数据名称相同
                                                RANGE readRange = data.range; // 获取读数据的范围
                                                RANGE writeRange = writeData.range; // 获取写节点数据的范围

                                                // 创建依赖关系的 JSON 对象
                                                nlohmann::json dependency;
                                                dependency["dependent_path_id"] = path.path_id;
                                                dependency["depended_path_id"] = otherPath.path_id;
                                                dependency["data_name"] = readName;

                                                // 添加依赖数据范围
                                                dependency["dependent_range"] = (readRange.is_lb_closed ? "[" : "(") +
                                                    to_string(readRange.lb) + ", " + to_string(readRange.ub) + (readRange.is_ub_closed ? "]" : ")");
                                                dependency["depended_range"] = (writeRange.is_lb_closed ? "[" : "(") +
                                                    to_string(writeRange.lb) + ", " + to_string(writeRange.ub) + (writeRange.is_ub_closed ? "]" : ")");
                                                dependency["dependent_node_id"] = edge.edge_id;
                                                dependency["depended_node_id"] = writeNode.node_id;

                                                // 将依赖关系添加到 JSON 数组中
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

        // 将所有依赖关系加入到 response_data 中
        response_data["dependencies"] = dependencies;
        response_data["state"] = "requirement returned";

        cout << "Response data: " << response_data.dump() << endl;

        // 设置响应类型为 JSON，并将 JSON 内容作为响应
        res.set_content(response_data.dump(), "application/json");  // .dump() 将 JSON 对象转换为字符串

    }

    // 返回路径字典，并生成对应的 JSON 格式
    void return_paths(httplib::Response& res) {
        // 创建一个 JSON 对象
        nlohmann::json response_data;
        response_data["state"] = "path returned";

        // 创建一个数组用于存放每个图的路径字典
        nlohmann::json paths_array = nlohmann::json::array();

        // 遍历每个图
        for (const auto& graph : graphs) {
            nlohmann::json graph_data;
            graph_data["graph_id"] = graph.graph_id;

            // 创建一个数组存放该图中的路径
            nlohmann::json paths_in_graph = nlohmann::json::array();

            // 遍历该图中的每条路径
            for (const auto& path : graph.paths) {
                nlohmann::json path_data;
                path_data["path_id"] = path.path_id;

                // 创建一个数组存放路径中的节点、条件和边
                nlohmann::json nodes_conditions_transitions_json = nlohmann::json::array();

                // 遍历路径中的每个节点
                for (size_t i = 0; i < path.path_nodes.size(); ++i) {
                    const auto& node = path.path_nodes[i];

                    // 如果是read类型节点（条件判断）
                    if (node.node_type == "read") {
                        nlohmann::json condition_data;
                        condition_data["condition_id"] = node.node_id;
                        condition_data["condition_information"] = node.node_information;

                        // 添加条件的分支信息
                        condition_data["branch_yes"] = node.branch_yes;
                        condition_data["branch_no"] = node.branch_no;

                        // 将条件数据添加到节点之间
                        nodes_conditions_transitions_json.push_back(condition_data);
                    }

                    // 如果是write类型节点（赋值操作），直接添加
                    else if (node.node_type == "write") {
                        // 节点信息
                        nlohmann::json node_data;
                        node_data["node_id"] = node.node_id;
                        node_data["node_information"] = node.node_information;
                        // 对于写操作，直接输出节点数据
                        nodes_conditions_transitions_json.push_back(node_data);
                    }

                    // 对于每个节点，必须通过一条边（transition）连接到下一个节点
                    // 找到连接当前节点的边（transition）
                    for (const auto& edge : path.path_edges) {
                        if (node.node_type == "write") { //如果是写节点，通过边来得到对应的节点
                            if (edge.source_node_id == node.node_id) {
                                nlohmann::json transition_data;
                                transition_data["transition_id"] = edge.edge_id;
                                transition_data["transition_information"] = edge.edge_information;

                                // 将边数据添加到路径中
                                nodes_conditions_transitions_json.push_back(transition_data);
                            }
                        }
                        else if (node.node_type == "read") { // 否则根据节点的branch来得到对应的边
                            if (edge.edge_id == node.branch_yes || edge.edge_id == node.branch_no) {
                                nlohmann::json transition_data;
                                transition_data["transition_id"] = edge.edge_id;
                                transition_data["transition_information"] = edge.edge_information;

                                // 将边数据添加到路径中
                                nodes_conditions_transitions_json.push_back(transition_data);
                            }
                        }
                    }
                }

                // 将路径中的节点、条件和边信息添加到路径数据中
                path_data["nodes and conditions and transitions"] = nodes_conditions_transitions_json;

                // 将路径数据添加到该图的路径数组中
                paths_in_graph.push_back(path_data);
            }

            // 将该图的路径信息添加到主路径数组中
            graph_data["paths"] = paths_in_graph;
            paths_array.push_back(graph_data);
        }

        // 将路径字典添加到response_data中
        response_data["paths"] = paths_array;

        // 设置响应类型为 JSON，并将 JSON 内容作为响应
        res.set_content(response_data.dump(), "application/json");  // .dump() 将 JSON 对象转换为字符串
    }

    void printPath(const PATH& path) {
        cout << "Path ID: " << path.path_id << endl;

        // 打印路径中的节点信息
        cout << "Nodes in Path:" << endl;
        for (const auto& node : path.path_nodes) {
            cout << "  Node ID: " << node.node_id << endl;
            cout << "  Node Type: " << node.node_type << endl;
            cout << "  Node Information: " << node.node_information << endl;

            // 打印节点中的数据集合
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

        // 打印路径中的边信息
        cout << "Edges in Path:" << endl;
        for (const auto& edge : path.path_edges) {
            cout << "  Edge ID: " << edge.edge_id << endl;
            cout << "  Source Node ID: " << edge.source_node_id << endl;
            cout << "  Target Node ID: " << edge.target_node_id << endl;
            cout << "  Edge Information: " << edge.edge_information << endl;

            // 打印边中的数据集合
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

    // 服务器实现
    void run_server(GRAPHS& x, const string& port) {
        // 创建服务器实例
        httplib::Server server;

        // 设置一个POST路由，响应客户端请求
        server.Post("/data", [&x](const httplib::Request& req, httplib::Response& res) {
            try {
                // 解析接收到的JSON数据
                json received_data = json::parse(req.body);
                std::cout << "Received JSON data: " << received_data.dump() << std::endl;

                // mode=a:传入图信息并进行更新
                if (received_data["mode"] == "a") {
                    x.requirement_diagram_analysis(received_data, res);
                }
                // mode=b:返回依赖关系
                else if (received_data["mode"] == "b") {
                    x.return_requirements(res);
                }
                // mode=c:返回路径信息
                else if (received_data["mode"] == "c") {
                    x.return_paths(res);
                }
            }
            catch (const std::exception& e) {
                // 错误处理
                res.status = 400;
                res.set_content(e.what(), "text/plain");
            }
        });

        // 将传入的端口号从字符串转换为整数
        int port_number = std::stoi(port);  // 使用 stoi 将字符串转换为整数

        // 在一个单独的线程中运行服务器
        std::thread server_thread([&server, port_number]() {
            std::cout << "Server is listening on http://localhost:" << port_number << std::endl;
            server.listen("0.0.0.0", port_number);  // 使用传入的端口号
            });

        // 等待直到用户中断（例如按Ctrl+C）或程序被手动终止
        server_thread.join(); // 使主线程等待服务器线程的完成
        std::cout << "Server has stopped." << std::endl;
    }
};

GRAPHS x; // 定义为全局变量


#endif // GRAPH_STRUCTURES_H

