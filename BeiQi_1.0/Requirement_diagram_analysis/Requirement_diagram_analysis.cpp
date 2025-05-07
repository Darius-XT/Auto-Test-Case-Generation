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

using namespace std;
using json = nlohmann::json;

// 定义正负无穷
constexpr double positiveInfinity = numeric_limits<double>::infinity();
constexpr double negativeInfinity = -numeric_limits<double>::infinity();

// 定义用到的各文件路径
string dependency_extract_path_loc = "../datas/dependency_extract_path.txt";
string dependency_extract_graph_loc = "../datas/dependency_extract_graph.txt";
string output_loc = "../datas/output.json";
//string input_loc = "../datas/input.txt";
string input_loc = "../../LLM_Requirement_Modeling/datas/input.txt";
string path_dic_loc = "../datas/path_dic.json";

// 以下定义各种数据结构

struct RANGE { // 数据范围
    double lb = negativeInfinity; // 下界
    double ub = positiveInfinity; // 上界
    bool is_lb_closed = false; // 下界是否闭合
    bool is_ub_closed = false; // 上界是否闭合
    double not_equal_to = positiveInfinity; // 不等于某值（若未启用则等于positiveInfinity）

    RANGE(double lowerBound = negativeInfinity,
        double upperBound = positiveInfinity,
        bool lowerBoundClosed = false,
        bool upperBoundClosed = false,
        double notEqualTo = positiveInfinity)
        : lb(lowerBound), ub(upperBound), is_lb_closed(lowerBoundClosed), is_ub_closed(upperBoundClosed), not_equal_to(notEqualTo) {}

    void print() { // 打印到控制台
        if (lb != ub) { // 上下界不等（范围）
            if (not_equal_to != positiveInfinity) // 不等于
                cout << "  Data Value: !=" << not_equal_to << endl; // 不等于标识启用，按不等于处理
            else { // 已知范围
                cout << "  Data Range: "
                    << (is_lb_closed ? "[" : "(") << lb << ", "
                    << ub << (is_ub_closed ? "]" : ")") << endl;
            }
        }
        else { // 上下界相等（值）
            cout << "  Data Value:" << ub << endl;
        }
    }
};

struct DATA { // 数据（单个节点/边中的数据）
    string name; // 数据名称
    string data_type; //数据类型（read/write）
    RANGE range; // 范围
    //清理函数
    void clear() {
        range.lb = negativeInfinity;
        range.ub = positiveInfinity;
        range.is_lb_closed = false;
        range.is_ub_closed = false;
        range.not_equal_to = positiveInfinity;
    }
};

// 数据（字典中的数据）
struct DIC_DATA {
    string name;  // 数据名称
    string type = "in";  // 数据类型（in/out）
    RANGE theoretical_range; // 数据值域
    vector<double> possibleValues; // 数据可能取值
};

struct NODE { // 节点
    string node_id; //节点ID
    string node_type; //节点类型（read/write）
    string node_information; //节点原始信息
    vector<DATA> node_datas; //节点中数据集合
    string branch_no; // no分支的边id
    string branch_yes; // yes分支的边id

    //清理函数
    void clear() {
        node_information = "";
        node_datas.clear();
    }
};

struct EDGE { // 边
    string edge_id; //边ID
    string source_node_id; //源节点ID
    string target_node_id; //目标节点ID
    string edge_information; //边原始信息
    vector<DATA> edge_datas; //边中数据集合

    //清理函数
    void clear() {
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
    string graph_name = "none"; //需求图名
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

        if (isNodeInPath(currentNode, currentPath)) { // 如果有环（当前节点已经在路径里）
            currentPath.path_id = graph_id + "_path_" + std::to_string(++pathIdCounter) + "_cycle";
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
            return; // 存在环则跳过本次递归
        }

        // 将当前节点添加到当前路径的节点集合中
        currentPath.path_nodes.push_back(currentNode);

        // 检查当前节点是否为终点（没有出度的节点）
        std::vector<EDGE> outgoingEdges = getEdgesByNodeId(currentNode.node_id);
        if (outgoingEdges.empty()) {
            // 如果是终点，则为当前路径生成唯一的 path_id
            currentPath.path_id = graph_id + "_path_" + std::to_string(++pathIdCounter);
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
        return c.bool_val(false);
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

    // 初始化
    void initialization() {
        GRAPH temp_graph; // 当前读取的需求图对象
        NODE temp_node; // 当前读取的节点对象
        EDGE temp_edge; // 当前读取的边对象
        DATA temp_data; // 当前读取的数据对象
        ifstream file(input_loc);
        if (!file.is_open()) {
            cerr << "Error opening file." << endl;
        }
        string line;

        // 对id，type，information赋值
        while (getline(file, line)) {
            istringstream iss(line);
            string word;//遍历
            string name;//存储名称
            while (iss >> word) {
                if (word == "graph_id:") {
                    iss >> word;
                    temp_graph.graph_id = word;
                }
                if (word == "graph_name:") {
                    iss >> word;
                    temp_graph.graph_name = word;
                }
                else if (word == "node_id:") {
                    iss >> word;
                    temp_node.node_id = word;
                }
                else if (word == "node_type:") {
                    iss >> word;
                    temp_node.node_type = word;
                }
                else if (word == "branch_yes:") {
                    iss >> word;
                    temp_node.branch_yes = word;
                }
                else if (word == "branch_no:") {
                    iss >> word;
                    temp_node.branch_no = word;
                }
                else if (word == "node_information:") {
                    // 去掉“node_information:”，用剩下的部分赋值
                    size_t length = string("node_information: ").length();
                    line = line.substr(length);
                    temp_node.node_information = line;
                }
                else if (word == "node_over") {
                    temp_graph.graph_nodes.push_back(temp_node);
                    temp_node.clear();
                }
                else if (word == "edge_id:") {
                    iss >> word;
                    temp_edge.edge_id = word;
                }
                else if (word == "source_node_id:") {
                    iss >> word;
                    temp_edge.source_node_id = word;
                }
                else if (word == "target_node_id:") {
                    iss >> word;
                    temp_edge.target_node_id = word;
                }
                else if (word == "edge_information:") {
                    // 去掉“edge_information:”，用剩下的部分赋值
                    size_t length = string("edge_information: ").length();
                    line = line.substr(length);
                    temp_edge.edge_information = line;
                }
                else if (word == "edge_over") {
                    temp_graph.graph_edges.push_back(temp_edge);
                    temp_edge.clear();
                }
                else if (word == "graph_over") {
                    graphs.push_back(temp_graph);
                    temp_graph.clear();
                }
            }
        }
        file.close();

        // 使用information对datas赋值:遍历所有图
        for (auto& graph : graphs) { 

            // 老版本,暂时不用
            auto process_info_to_datas_old = [this](const string& info, const string& default_data_type, vector<DATA>& datas) {
                // 创建副本，保存预处理结果:对原始的字符串进行格式改动,对其中的函数进行处理,方便后续提取 data
                string processed_info = info;
                processed_info.erase(remove(processed_info.begin(), processed_info.end(), '('), processed_info.end());
                processed_info.erase(remove(processed_info.begin(), processed_info.end(), ')'), processed_info.end());
                processed_info.erase(remove(processed_info.begin(), processed_info.end(), ','), processed_info.end());
                vector<string> subStrings = { "duration", "min", "fabs" };
                removeSubstrings(processed_info, subStrings);

                istringstream iss(processed_info);
                string s;
                DATA data_temp1;
                DATA data_temp2;

                while (iss >> s) {
                    // 如果是 and/or, 说明之前的子表达式已经处理完了,之后是新的表达式
                    if (s == "and" || s == "or") {
                        continue;
                    }
                    // 读取的第一个字符一定是名字
                    data_temp1.name = s;
                    data_temp1.data_type = default_data_type;
                    iss >> s;
                    // 第二个字符应该是运算符
                    if (s == ">" || s == ">=" || s == "<" || s == "<=" || s == "!=" || s == "==" || s == "=") {
                        // 第三个字符是要被赋值的对象,可能是一个数字常量,也可能是一个变量
                        iss >> s;
                        // 如果是数字常量
                        if (isNumber(s)) {
                            if (s == ">") data_temp1.range = RANGE(stod(s), positiveInfinity, false, false, positiveInfinity);
                            else if (s == ">=") data_temp1.range = RANGE(stod(s), positiveInfinity, true, false, positiveInfinity);
                            else if (s == "<") data_temp1.range = RANGE(negativeInfinity, stod(s), false, false, positiveInfinity);
                            else if (s == "<=") data_temp1.range = RANGE(negativeInfinity, stod(s), false, true, positiveInfinity);
                            else if (s == "!=") data_temp1.range = RANGE(negativeInfinity, positiveInfinity, false, false, stod(s));
                            else if (s == "==" || s == "=") data_temp1.range = RANGE(stod(s), stod(s), true, true, positiveInfinity);
                            input_data(data_temp1, datas);
                        }
                        // 如果是变量,说明是读类型
                        else {
                            data_temp2.data_type = "read";
                            data_temp2.name = s;
                            input_data(data_temp1, datas);
                            input_data(data_temp2, datas);
                        }
                    }
                }
            };
            
            // 输入字符串, 节点的类型与要存储的vector<Data>, 根据表达式,提取出其中所有的变量加入vector
            auto process_info_to_datas = [this](const string& info, const string& default_data_type, vector<DATA>& datas) {
                
                // 内部函数 trim :去除字符串的首尾空格
                auto trim = [](string& str) {
                    // 删除字符串前面的空格
                    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
                        return !std::isspace(ch);  // 找到第一个不是空格的字符
                        }));

                    // 删除字符串后面的空格
                    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
                        return !std::isspace(ch);  // 找到第一个不是空格的字符
                        }).base(), str.end());
                    };

                // 内部函数 countSubstrings:返回字符串的子串个数(按空格分隔,共有多少个字符串)
                auto countSubstrings = [](const std::string& str) -> int {
                    std::istringstream iss(str);
                    std::string word;
                    int count = 0;

                    while (iss >> word) {
                        count++;
                    }

                    return count;
                };

                // 内部函数: 从子表达式(左式 / 右式)中提取变量，记录类型, 保存到 vector<Data> 中
                auto extract_datas_from_substring = [this, trim](const string& expr, const string& data_type, vector<DATA>& datas) {
                    string temp_expr = expr;

                    // 第一步: 先处理所有函数调用，把函数名去掉，把括号内的逗号换成空格
                    std::regex func_call_regex(R"((\w+)\s*\(([^)]*)\))");
                    std::smatch match;
                    while (std::regex_search(temp_expr, match, func_call_regex)) {
                        string args = match[2]; // 括号里的内容
                        for (char& ch : args) {
                            if (ch == ',') ch = ' '; // 逗号换成空格
                        }
                        // 用参数替换整个函数调用部分
                        temp_expr = match.prefix().str() + " " + args + " " + match.suffix().str();
                    }

                    // 第二步：将表达式中所有操作符替换为空格（包括算术和比较符）
                    std::string operators = "+-*/<>=!&|(),";
                    for (char& ch : temp_expr) {
                        if (operators.find(ch) != std::string::npos) {
                            ch = ' ';
                        }
                    }

                    // 第三步: 按空格分词，筛选出变量
                    std::istringstream iss(temp_expr);
                    string token;
                    while (iss >> token) {
                        trim(token);
                        if (!token.empty() && !isNumber(token)) {  // 不是数字，就是变量
                            DATA d{ token, data_type };
                            input_data(d, datas);
                        }
                    }
                };

                // 创建副本，保存预处理结果:之后会对原字符串去除括号、逗号以及空格并清理函数名
                string processed_info = info;

                // 首先按 and / or / && / || / & / | 拆分为子表达式
                std::vector<std::string> sub_expressions;
                std::regex split_regex(R"(\b(and|or)\b|&&|\|\||&|\|)");
                std::sregex_token_iterator iter(info.begin(), info.end(), split_regex, -1);
                std::sregex_token_iterator end;

                for (; iter != end; ++iter) {
                    std::string sub_expr = *iter;
                    if (!sub_expr.empty()) {
                        sub_expressions.push_back(sub_expr);
                    }
                }

                // 此时的各个子表达式一定不含逻辑连接词,再对每个子表达式执行后续操作
                for (const auto& sub_expr : sub_expressions) {
                    // 首先检查是否是简单的"变量 操作符 数字常量"形式,如果是说明要对范围进行限定,此时直接处理并return
                    if (countSubstrings(processed_info) == 3) {
                        static const vector<string> comparators = { ">=", "<=", "==", "!=", ">", "<", "=" };
                        istringstream iss(processed_info);
                        string s;
                        DATA data_temp1;

                        // 读取第一个字符
                        iss >> s;
                        data_temp1.name = s;

                        // 读取第二个字符,判断是不是操作符
                        iss >> s;
                        auto it = std::find(comparators.begin(), comparators.end(), s);

                        // 如果存在,说明是操作符
                        if (it != comparators.end()) {
                            string temp_opr = s;
                            // 读取第三个字符串
                            iss >> s;
                            // 如果是数字
                            if (isNumber(s)) {
                                data_temp1.data_type = default_data_type;
                                if (temp_opr == ">") data_temp1.range = RANGE(stod(s), positiveInfinity, false, false, positiveInfinity);
                                else if (temp_opr == ">=") data_temp1.range = RANGE(stod(s), positiveInfinity, true, false, positiveInfinity);
                                else if (temp_opr == "<") data_temp1.range = RANGE(negativeInfinity, stod(s), false, false, positiveInfinity);
                                else if (temp_opr == "<=") data_temp1.range = RANGE(negativeInfinity, stod(s), false, true, positiveInfinity);
                                else if (temp_opr == "!=") data_temp1.range = RANGE(negativeInfinity, positiveInfinity, false, false, stod(s));
                                else if (temp_opr == "==" || temp_opr == "=") data_temp1.range = RANGE(stod(s), stod(s), true, true, positiveInfinity);
                                input_data(data_temp1, datas);
                                return;
                            }
                        }
                    }

                    // 如果没有返回,说明不是最简单的可以赋值的情况,此时按复杂表达式进行判断
                    // 如果是读节点
                    if (default_data_type == "read") {

                        // 可能使用的分割符
                        static const vector<string> comparators = { ">=", "<=", "==", "!=", ">", "<", "=" };

                        string comparator;
                        size_t comparator_pos = string::npos;

                        // 找到分隔符所在的位置
                        for (const auto& comp : comparators) {
                            size_t pos = processed_info.find(comp);

                            // string::npos是find函数提供的默认字符串,与无搜索结果对应,这里不相等就表示找到了结果
                            if (pos != string::npos) {
                                comparator = comp;
                                comparator_pos = pos;
                                break;
                            }
                        }

                        // 同上,不相等表示找到了结果
                        if (comparator_pos != string::npos) {
                            string left_expr = processed_info.substr(0, comparator_pos);
                            string right_expr = processed_info.substr(comparator_pos + comparator.length());

                            // 去掉字符串的首尾空格
                            trim(left_expr);
                            trim(right_expr);

                            // 左右子表达式都作为"read"处理
                            extract_datas_from_substring(left_expr, "read", datas);
                            extract_datas_from_substring(right_expr, "read", datas);
                        }
                    }

                    else if (default_data_type == "write") {
                        // 处理赋值表达式，分隔符必须是"="
                        size_t pos = processed_info.find('=');

                        // 如果找到
                        if (pos != string::npos) {
                            string left_expr = processed_info.substr(0, pos);
                            string right_expr = processed_info.substr(pos + 1);

                            // 去除字符串首尾空格
                            trim(left_expr);
                            trim(right_expr);

                            // 左边是被赋值的变量，type 是"write"
                            extract_datas_from_substring(left_expr, "write", datas);
                            // 右边是来源，type 是"read"
                            extract_datas_from_substring(right_expr, "read", datas);
                        }
                    }
                }
            };

            for (auto& node : graph.graph_nodes) {
                process_info_to_datas(node.node_information, node.node_type, node.node_datas);
            }

            // 遍历图中所有边
            for (auto& edge : graph.graph_edges) {
                process_info_to_datas(edge.edge_information, "read", edge.edge_datas);
            }
        }

        // 对paths赋值——将节点与边加入path
        for (auto& graph : graphs) {
            graph.generateAllPaths();
        }

        // 对data_dic赋值
        for (auto& graph : graphs) {
            // 遍历图中所有节点
            for (auto& node : graph.graph_nodes) {
                // 检查节点是否为"write"类型
                if (node.node_type == "write") {
                    // 遍历该节点中所有数据项
                    for (auto& data : node.node_datas) {
                        // 确保数据类型为"write"且被赋予了具体值
                        if (data.data_type == "write" && data.range.lb == data.range.ub) {
                            addDataToDic(data_dic, data);
                        }
                    }
                }
            }
        }
    }

    // 打印输出 - 输出需求图中的详细信息
    void output() {
        cout << "以下打印各个需求图中的信息：" << endl;
        for (auto& graph : graphs) {
            // 打印需求图的基本信息
            cout << "Graph ID: " << graph.graph_id << endl;

            // 打印需求图中的节点信息
            cout << "Nodes:" << endl;
            for (auto& node : graph.graph_nodes) {
                cout << " Node ID: " << node.node_id << endl;
                cout << " Node Type: " << node.node_type << endl;
                cout << " Node Branch Yes: " << node.branch_yes << endl;
                cout << " Node Branch No: " << node.branch_no << endl;
                cout << " Node Information: " << node.node_information << endl;
                for (auto& data : node.node_datas) {
                    cout << "  Data Name: " << data.name << endl;
                    cout << "  Data Type: " << data.data_type << endl;
                    data.range.print();
                }
            }

            // 打印需求图中的边信息
            cout << "Edges:" << endl;
            for (auto& edge : graph.graph_edges) {
                cout << " Edge ID: " << edge.edge_id << endl;
                cout << " Source Node ID: " << edge.source_node_id << endl;
                cout << " Target Node ID: " << edge.target_node_id << endl;
                cout << " Edge Information: " << edge.edge_information << endl;
                for (auto& data : edge.edge_datas) {
                    cout << "  Data Name: " << data.name << endl;
                    cout << "  Data Type: " << data.data_type << endl;
                    data.range.print();
                }
            }

            // 打印需求图中的路径信息
            cout << "Paths:" << endl;
            for (auto& path : graph.paths) {
                cout << " Path ID: " << path.path_id << endl;
                for (auto& p_node : path.path_nodes) {
                    cout << "  Path Node Information: " << p_node.node_information << endl;
                }
            }

            // 打印分隔符以区分不同的需求图
            cout << "--------------------------------------" << endl;
        }
        cout << "以下打印数据字典：" << endl;
        for (const auto& dic_data : data_dic) {
            cout << "名称: " << dic_data.name << ", 类型: " << dic_data.type;
            if (dic_data.theoretical_range.lb != dic_data.theoretical_range.ub) { // 上下界不等（范围）
                if (dic_data.theoretical_range.not_equal_to != positiveInfinity) // 不等于
                    cout << "  理论值域: !=" << dic_data.theoretical_range.not_equal_to; // 不等于标识启用，按不等于处理
                else { // 已知范围
                    cout << "   理论值域: "
                        << (dic_data.theoretical_range.is_lb_closed ? "[" : "(") << dic_data.theoretical_range.lb << ", "
                        << dic_data.theoretical_range.ub << (dic_data.theoretical_range.is_ub_closed ? "]" : ")");
                }
            }
            else { // 上下界相等（值）
                cout << "  理论值域:" << dic_data.theoretical_range.ub;
            }
            cout << "  可能取值: ";
            for (const auto& value : dic_data.possibleValues) {
                cout << value << " ";
            }
            cout << endl;
        }
        cout << "---------------------------" << endl;
    }

    // 以下为项目要求的功能函数

    // 输出各路径中节点ID到path_dic.json
    void output_path_dic() {
        // 创建一个 JSON 对象作为顶层结构
        json response_data;

        // 所有图的路径信息数组
        json paths_array = json::array();

        for (const auto& graph : graphs) {
            json graph_data;
            graph_data["graph_id"] = graph.graph_id;
            graph_data["graph_name"] = graph.graph_id; // 按你要求用 graph_id 代替 name

            json paths_in_graph = json::array();

            for (const auto& path : graph.paths) {
                json path_data;
                path_data["path_id"] = path.path_id;

                json nodes_conditions_transitions_json = json::array();

                for (size_t i = 0; i < path.path_nodes.size(); ++i) {
                    const auto& node = path.path_nodes[i];

                    // 处理 read 类型节点（判断）
                    if (node.node_type == "read") {
                        json condition_data;
                        condition_data["condition_id"] = node.node_id;
                        condition_data["condition_information"] = node.node_information;
                        condition_data["branch_yes"] = node.branch_yes;
                        condition_data["branch_no"] = node.branch_no;
                        nodes_conditions_transitions_json.push_back(condition_data);
                    }
                    // 处理 write 类型节点（赋值）
                    else if (node.node_type == "write") {
                        json node_data;
                        node_data["node_id"] = node.node_id;
                        node_data["node_information"] = node.node_information;
                        nodes_conditions_transitions_json.push_back(node_data);
                    }

                    // 处理 transition（边）
                    for (const auto& edge : path.path_edges) {
                        bool match = false;
                        if (node.node_type == "write") {
                            match = (edge.source_node_id == node.node_id);
                        }
                        else if (node.node_type == "read") {
                            match = (edge.edge_id == node.branch_yes || edge.edge_id == node.branch_no);
                        }

                        if (match) {
                            json transition_data;
                            transition_data["transition_id"] = edge.edge_id;
                            transition_data["transition_information"] = edge.edge_information;
                            nodes_conditions_transitions_json.push_back(transition_data);
                        }
                    }
                }

                path_data["nodes and conditions and transitions"] = nodes_conditions_transitions_json;
                paths_in_graph.push_back(path_data);
            }

            graph_data["paths"] = paths_in_graph;
            paths_array.push_back(graph_data);
        }

        response_data["paths"] = paths_array;

        // 写入 JSON 文件
        std::ofstream out(path_dic_loc);
        if (!out.is_open()) {
            std::cerr << "无法打开文件用于写入 JSON。" << std::endl;
            return;
        }

        out << response_data.dump(4);  // 格式化缩进为4
        out.close();
    }

    // 依赖提取（输出dependency_extract_path.txt，dependency_extract_graph.txt以及部分的output.txt）
    void dependency_extraction() {
        int count = 0;
        // 先按路径提取（详细结果输入output.txt文档，简略的去重版本输入dependency_extract_path.txt文档）
        std::ofstream file(output_loc, std::ios::app | std::ios::binary);
        if (file.tellp() == 0) {  // 文件为空才写入 BOM
            unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
            file.write(reinterpret_cast<char*>(bom), sizeof(bom));
        }
        if (!file) {
            std::cerr << "无法打开文件 output.txt 进行写入";
            return;
        }
        std::ofstream file2(dependency_extract_path_loc, std::ios::app);  // 创建并以追加模式打开文件
        if (!file2) {
            std::cerr << "无法打开文件 dependency_extract_path.txt 进行写入";
            return;
        }
        std::ofstream file3(dependency_extract_graph_loc, std::ios::app);  // 创建并以追加模式打开文件
        if (!file3) {
            std::cerr << "无法打开文件 dependency_extract_graph.txt 进行写入";
            return;
        }
        
        // ====== 依赖分析与 JSON 构建逻辑开始 ======
        nlohmann::json response_data;
        nlohmann::json dependencies = nlohmann::json::array();

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

        // 添加汇总信息到 response_data
        response_data["dependencies"] = dependencies;

        // ====== 写入 JSON 到 output_loc 文件 ======
        std::ofstream json_out(output_loc);
        if (json_out.is_open()) {
            json_out << response_data.dump(4);  // dump(4) 表示格式化缩进
            json_out.close();
            std::cout << "依赖信息成功保存至 JSON 文件: " << output_loc << std::endl;
        }
        else {
            std::cerr << "无法打开文件用于写入 JSON: " << output_loc << std::endl;
        }

        // 按图提取依赖关系（简略的去重版本输入dependency_extract_graph.txt文档）
        for (int i = 0; i < graphs.size(); ++i) {
            GRAPH& currentGraph = graphs[i];
            // 遍历图中的所有节点
            for (NODE& node : currentGraph.graph_nodes) {
                // 如果是写节点
                if (node.node_type == "write") {
                    for (DATA& data : node.node_datas) { // 遍历节点的数据
                        if (data.data_type == "read") { // 如果是读数据
                            string readName = data.name; // 获取读节点数据的名称

                            // 遍历除当前图以外的其他图
                            for (int j = 0; j < graphs.size(); ++j) {
                                if (i == j) continue; // 跳过当前图

                                GRAPH& otherGraph = graphs[j];

                                // 遍历其他图
                                for (NODE& writeNode : otherGraph.graph_nodes) { // 遍历路径上的所有节点
                                    for (DATA& writeData : writeNode.node_datas) { // 遍历写节点的数据
                                        if (writeNode.node_type == "write" && writeData.data_type == "write" && writeData.name == readName && rangesIntersect(writeData.range, data.range) == true) { // 如果是写节点的写数据且名称与读数据名称相同,且有交集
                                            RANGE readRange = data.range; // 获取读节点数据的范围
                                            RANGE writeRange = writeData.range; // 获取写节点数据的范围

                                            file3 << "<依赖图名：" << currentGraph.graph_name
                                                << " 被依赖图名：" << otherGraph.graph_name
                                                << " 数据名称：" << readName
                                                << ">" << endl;

                                            //检查是否已经记录了依赖关系
                                            if (std::find(currentGraph.dependent_graph_id.begin(), currentGraph.dependent_graph_id.end(), otherGraph.graph_id) == currentGraph.dependent_graph_id.end()) {
                                                // 如果没有记录，则添加依赖的图的ID
                                                currentGraph.dependent_graph_id.push_back(otherGraph.graph_id);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                // 如果是读节点
                else if (node.node_type == "read") {
                    for (DATA& data : node.node_datas) { // 遍历节点的数据
                        if (data.data_type == "read") { // 如果是读数据
                            string readName = data.name; // 获取读节点数据的名称

                            // 遍历除当前图以外的其他图
                            for (int j = 0; j < graphs.size(); ++j) {
                                if (i == j) continue; // 跳过当前图

                                GRAPH& otherGraph = graphs[j];

                                // 遍历其他图
                                for (NODE& writeNode : otherGraph.graph_nodes) { // 遍历路径上的所有节点
                                    for (DATA& writeData : writeNode.node_datas) { // 遍历写节点的数据
                                        if (writeNode.node_type == "write" && writeData.data_type == "write" && writeData.name == readName) { // 如果是写节点的写数据且名称与读数据名称相同（无需有交集，因为读节点关于任何范围都可以形成依赖）
                                            RANGE readRange = data.range; // 获取读节点数据的范围
                                            RANGE writeRange = writeData.range; // 获取写节点数据的范围

                                            file3 << "<依赖图名：" << currentGraph.graph_name
                                                << " 被依赖图名：" << otherGraph.graph_name
                                                << " 数据名称：" << readName
                                                << ">" << endl;

                                            //检查是否已经记录了依赖关系
                                            if (std::find(currentGraph.dependent_graph_id.begin(), currentGraph.dependent_graph_id.end(), otherGraph.graph_id) == currentGraph.dependent_graph_id.end()) {
                                                // 如果没有记录，则添加依赖的图的ID
                                                currentGraph.dependent_graph_id.push_back(otherGraph.graph_id);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            for (EDGE& edge : currentGraph.graph_edges) { // 遍历图中所有的边
                for (DATA& data : edge.edge_datas) { // 遍历边上的数据
                    if (data.data_type == "read") { // 如果是读数据
                        string readName = data.name; // 获取读数据的名称

                        // 遍历除当前图以外的其他图
                        for (int j = 0; j < graphs.size(); ++j) {
                            if (i == j) continue; // 跳过当前图

                            GRAPH& otherGraph = graphs[j];

                            // 遍历其他图
                            for (NODE& writeNode : otherGraph.graph_nodes) { // 遍历所有节点
                                for (DATA& writeData : writeNode.node_datas) { // 遍历写节点的数据
                                    if (writeNode.node_type == "write" && writeData.data_type == "write" && writeData.name == readName && rangesIntersect(writeData.range, data.range) == true) { // 如果是写节点的写数据且名称与读数据名称相同,且有交集
                                        RANGE readRange = data.range; // 获取读数据的范围
                                        RANGE writeRange = writeData.range; // 获取写节点数据的范围

                                        // 输出发现的依赖关系
                                        file3 << "<依赖图id：" << currentGraph.graph_id
                                            << " 被依赖图id：" << otherGraph.graph_id
                                            << " 数据名称：" << readName
                                            << ">" << endl;
                                        //检查是否已经记录了依赖关系
                                        if (std::find(currentGraph.dependent_graph_id.begin(), currentGraph.dependent_graph_id.end(), otherGraph.graph_id) == currentGraph.dependent_graph_id.end()) {
                                            // 如果没有记录，则添加依赖的图的ID
                                            currentGraph.dependent_graph_id.push_back(otherGraph.graph_id);
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

    // 检测边条件永不满足
    void checkEdgeConditionUnsatisfiable() {
        int count = 0;
        std::ofstream file(output_loc, std::ios::app);  // 创建并以追加模式打开文件
        if (!file) {
            std::cerr << "无法打开文件 output.txt 进行写入";
            return;
        }
        file << "检测边条件永不满足：" << endl;
        z3::context c; // 若在同一个上下文 (z3::context) 中创建变量并构建表达式，那么这些变量在不同的表达式中是相同的
        for (const auto& graph : graphs) {
            for (const auto& edge : graph.graph_edges) {
                if (edge.edge_information != "") { // 边中有信息
                    z3::solver s(c);
                    z3::expr edgeExpr = parseConditionToExpr(edge.edge_information, c);
                    s.add(edgeExpr);
                    vector<z3::expr> variables = getVariables(edgeExpr); // 从z3表达式中得到所有的z3变量
                    map<z3::expr, RANGE> variableRanges = getVariableRanges(variables, data_dic); // 根据z3变量得到其与值域的map
                    z3::expr constraints = buildConstraints(c, variableRanges); // 根据z3变量与值域的map得到对应的z3范围限制表达式
                    s.add(constraints);

                    if (s.check() == z3::unsat) {
                        // 当条件不可满足时打印信息
                        file << "边条件 '" << edge.edge_information << "' 永无可能满足" << endl;
                        count++;
                    }
                }
            }
        }
        file << endl << "共发现" << count << "个边条件永不满足的情况" << endl;
        file << "--------------------------------------" << endl;
    }

    // 检测分支同时满足
    void checkGraphEdgesForSMT() {
        int count = 0;
        std::ofstream file(output_loc, std::ios::app);  // 创建并以追加模式打开文件
        if (!file) {
            std::cerr << "无法打开文件 output.txt 进行写入";
            return;
        }
        file << "检测分支同时满足：" << endl;
        for (const auto& graph : graphs) {
            for (const auto& node : graph.graph_nodes) {
                if (node.node_type == "write") { // 只检测写节点（读节点的两条出边上一定无条件，排除）
                    auto edges = graph.get_out_edges_by_node(node);
                    bool have_empty_edge = false;
                    for (auto it = edges.begin(); it != edges.end(); ) {
                        if (it->edge_information == "" && edges.size() > 1) {
                            count++;
                            file << "图id：" << graph.graph_id << "，节点id： " << node.node_id << endl;
                            file << "可被同时满足的边列表为：" << endl;
                            for (auto edge : edges) {
                                if (edge.edge_information == "")
                                    file << edge.edge_information << "空边" << endl;
                                else
                                    file << edge.edge_information << " " << endl;
                            }
                            file << endl;
                            have_empty_edge = true;
                            break;
                            //it = edges.erase(it); // erase会返回指向被删除元素之后的元素的迭代器
                        }
                        else {
                            ++it; // 只有在不删除元素时才增加迭代器
                        }
                    }

                    if (edges.size() > 1 && have_empty_edge == false) {
                        bool allUnsatisfiable = true;
                        // 遍历所有无重复的两两边组合
                        for (size_t i = 0; i < edges.size() && allUnsatisfiable; ++i) {
                            for (size_t j = i + 1; j < edges.size(); ++j) {
                                // 生成当前组合，并检查是否满足
                                vector<EDGE> edgePair = { edges[i], edges[j] };
                                if (checkEdgesForSatisfiability(edgePair)) {
                                    // 如果这对边满足，那么存在至少一对边可以同时满足
                                    allUnsatisfiable = false;
                                    break; // 早期终止：已发现至少一对边可以同时满足
                                }
                            }
                        }
                        if (!allUnsatisfiable) {
                            // 如果发现至少有一对边可以同时满足
                            count++;
                            file << "图id：" << graph.graph_id << "，节点id： " << node.node_id << endl;
                            file << "可被同时满足的边列表为：" << endl;
                            for (auto edge : edges) {
                                file << edge.edge_information << " " << endl;
                            }
                            file << endl;
                        }
                    }
                }
            }
        }
        file << "共发现" << count << "个分支同时满足的情况" << endl;
        file << "--------------------------------------" << endl;
    }

    // 检测需求图缺失（生产方&消费方）——包括外部需求图缺失与内部先读后写的特例
    void graph_miss_detect() {
        int count_pro = 0;
        int count_cus = 0;
        int count_read_first = 0;
        std::ofstream file(output_loc, std::ios::app);  // 创建并以追加模式打开文件
        if (!file) {
            std::cerr << "无法打开文件 output.txt 进行写入";
            return;
        }
        file << "检测生产方需求图缺失：" << endl;
        // 遍历graphs集合中的每个图
        for (GRAPH& graph : graphs) {
            // 遍历图中的所有节点
            for (NODE& node : graph.graph_nodes) {
                // 遍历节点中的所有数据
                for (DATA& data1 : node.node_datas) {
                    if (data1.data_type == "read") {  // 如果数据是读类型
                        bool has_producer = check_data_has_out_producer(data1, graph); // 调用check_data_has_out_producer来检查数据是否有外部制造方
                        bool has_inner_producer = false;
                        if (!has_producer) {  // 如果没有外部制造方,进一步判断内部有没有制造方
                            for (NODE& node2 : graph.graph_nodes) {
                                for (DATA& data2 : node2.node_datas) {
                                    if (data2.data_type == "write" && data2.name == data1.name && rangesIntersect(data1.range, data2.range) == true) { // 如果有内部制造方，判断是否至少存在一条路径，其中制造方节点在本节点之前
                                        has_inner_producer = true;
                                        if (isProducerAtOrBeforeNode(graph, node, data1) == false) { // 如果不存在一条路径，其中制造方节点在前，报错先读后写
                                            count_read_first++;
                                            file << "<图id：" << graph.graph_id << "  节点信息：" << node.node_information << "  生产方缺失数据：" << data1.name << ">  （先读后写）" << endl;
                                        }
                                    }
                                }
                            }
                            if (has_inner_producer == false && data1.name != "") {
                                count_pro++;
                                file << "<图id：" << graph.graph_id << "  节点信息：" << node.node_information << "  生产方缺失数据：" << data1.name << ">" << endl;
                            }

                        }
                    }
                }
            }

            for (EDGE& edge : graph.graph_edges) {
                // 遍历边中的所有数据
                for (DATA& data1 : edge.edge_datas) {
                    if (data1.data_type == "read") { // 如果数据是读类型
                        bool has_producer = check_data_has_out_producer(data1, graph); // 调用check_data_has_producer来检查数据是否有制造方
                        bool has_inner_producer = false;
                        if (!has_producer) { // 如果没有外部制造方，进一步判断内部有没有制造方
                            for (NODE& node2 : graph.graph_nodes) {
                                // 遍历节点中的所有数据
                                for (DATA& data2 : node2.node_datas) {
                                    if (data2.data_type == "write" && data2.name == data1.name && rangesIntersect(data1.range, data2.range) == true) { // 如果有内部制造方，判断是否至少存在一条路径，其中制造方节点在本边之前
                                        has_inner_producer = true;
                                        if (isProducerAtOrBeforeEdge(graph, edge, data1) == false) { // 如果不存在一条路径，其中制造方节点在前，报错先读后写
                                            count_read_first++;
                                            file << "<图id：" << graph.graph_id << "   边信息：" << edge.edge_information << "  生产方缺失数据：" << data1.name << ">  （先读后写）" << endl;
                                        }
                                    }
                                }
                            }
                            if (has_inner_producer == false && data1.name != "") { // 如果没有内部制造方，报错：生产方缺失
                                count_pro++;
                                file << "<图id：" << graph.graph_id << "  边信息：" << edge.edge_information << "  生产方缺失数据：" << data1.name << ">" << endl;
                            }
                        }
                    }
                }
            }
        }
        file << endl;
        file << "检测消费方需求图缺失：" << endl;
        // 遍历graphs集合中的每个图
        for (GRAPH& graph : graphs) {
            // 遍历图中的所有节点
            for (NODE& node : graph.graph_nodes) {
                // 遍历节点中的所有数据
                for (DATA& data : node.node_datas) {
                    // 如果数据是写类型
                    if (data.data_type == "write") {
                        // 调用check_data_has_consumer来检查数据是否有消费方
                        bool has_consumer = check_data_has_out_consumer(data, graph);
                        // 如果没有外部制造方，说明缺失制消费方需求图
                        if (!has_consumer) {
                            // 输出结果
                            count_cus++;
                            file << "<图id：" << graph.graph_id << "  节点信息：" << node.node_information << "  消费方缺失数据：" << data.name << ">" << endl;
                        }
                    }
                }
            }
        }
        file << "共发现" << count_pro << "个生产方缺失的情况" << endl;
        file << "共发现" << count_read_first << "个先读后写的情况" << endl;
        file << "共发现" << count_cus << "个消费方缺失的情况" << endl;
        file << "--------------------------------------" << endl;
    }

    // 检测循环依赖
    void check_dependency_cycle() {
        std::ofstream file(output_loc, std::ios::app);  // 创建并以追加模式打开文件
        if (!file) {
            std::cerr << "无法打开文件 output.txt 进行写入";
            return;
        }
        file << "检测循环依赖：" << endl;

        unordered_map<string, vector<string>> depGraphs; // 存储每个图的依赖关系
        unordered_map<string, bool> visited; // 记录访问状态，防止重复访问同一个图
        vector<string> path; // 记录访问路径，用于打印环

        // 构建依赖关系图
        for (auto& graph : graphs) {
            depGraphs[graph.graph_id] = graph.dependent_graph_id;
        }

        function<bool(const string&)> dfsCheckCycle = [&](const string& currentGraphId) -> bool {
            if (visited[currentGraphId]) {
                // 发现环: 打印并返回true
                for (const auto& id : path) {
                    file << id << " -> ";
                }
                file << currentGraphId << endl; // 闭环处再次打印当前图ID
                return true; // 发现环
            }
            if (depGraphs.find(currentGraphId) == depGraphs.end()) {
                return false; // 当前图没有依赖或已检查过
            }

            visited[currentGraphId] = true; // 标记当前图为已访问
            path.push_back(currentGraphId); // 添加当前图ID到路径中

            for (const auto& depId : depGraphs[currentGraphId]) {
                if (dfsCheckCycle(depId)) {
                    return true; // 递归检测到环
                }
            }

            // 回溯
            visited[currentGraphId] = false; // 取消当前图的访问标记
            path.pop_back(); // 从路径中移除当前图ID

            return false; // 未发现环
            };

        // 遍历所有图，检查是否存在环
        for (auto& graph : graphs) {
            if (dfsCheckCycle(graph.graph_id)) {
                file << "存在循环依赖" << endl;
                file << "--------------------------------------" << endl;
                return; // 一旦发现环即停止检查
            }
        }

        file << "不存在循环依赖" << endl;
        file << "--------------------------------------" << endl;
    }

    // 检测分支缺失（警告）
    void checkForMissingBranches() {
        int count = 0;
        ofstream file(output_loc, ios::app);  // 创建并以追加模式打开文件
        if (!file) {
            cerr << "无法打开文件 output.txt 进行写入";
            return;
        }
        file << "检测分支缺失：" << endl;
        for (const auto& graph : graphs) {
            for (const auto& node : graph.graph_nodes) {
                auto edges = graph.get_out_edges_by_node(node);
                vector<DATA> datas; // 当前节点所有出边中包含的全部数据集合
                for (auto edge : edges) { //遍历节点的全部出边
                    for (auto data : edge.edge_datas) {
                        datas.push_back(data);
                    }
                }

                // 使用unordered_map来分组存储相同name的range
                unordered_map<string, vector<RANGE>> nameToRanges;

                // 分组
                for (const auto& data : datas) {
                    nameToRanges[data.name].push_back(data.range);
                }

                // 最终结果：一个vector<vector<RANGE>>
                vector<vector<RANGE>> finalRanges;

                for (const auto& entry : nameToRanges) {
                    finalRanges.push_back(entry.second);
                }
                for (auto& entry : nameToRanges) {
                    auto& name = entry.first;
                    auto& ranges = entry.second;
                    RANGE range; // 并集返回的范围
                    range = unionOfRanges(ranges); // 调用unionOfRanges函数，并保存结果
                    if (range.is_lb_closed == false && range.is_ub_closed == false && range.lb == negativeInfinity && range.ub == positiveInfinity) { // 覆盖了全部值域
                    }
                    else {
                        count++;
                        file << "节点id：" << node.node_id << "  未覆盖完全的数据名称：" << name << "  值域：" << "(-inf,inf)";
                        if (range.lb != range.ub) { // 上下界不等（范围）
                            if (range.lb == negativeInfinity && range.ub == positiveInfinity) { // 上下界均为无穷（未知/不等于）
                                if (range.not_equal_to == positiveInfinity) // 不等于标识未启用
                                    file << "  data range: unknown" << endl; // 说明数据范围未知
                                else
                                    file << "  data value: !=" << range.not_equal_to << endl; // 不等于标识启用，按不等于处理
                            }
                            else { // 已知范围
                                file << "  data range: "
                                    << (range.is_lb_closed ? "[" : "(") << range.lb << ", "
                                    << range.ub << (range.is_ub_closed ? "]" : ")") << endl;
                            }
                        }
                        else { // 上下界相等（值）
                            file << "  data value: " << range.ub << endl;
                        }
                    }
                }
            }
        }
        file << "共发现" << count << "个分支缺失的情况" << endl;
        file << "-------------------------------------------------" << endl;
    }

    // 传入图信息，对现有图信息进行修改/添加
    void requirement_diagram_analysis(const json& received_data) {
        GRAPH temp_graph; // 当前读取的需求图对象
        NODE temp_node; // 当前读取的节点对象
        EDGE temp_edge; // 当前读取的边对象
        DATA temp_data; // 当前读取的数据对象
        vector<GRAPH> temp_graphs; // 本次有修改/添加的所有图
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
            if(exist == false){ // 如果不是已经存在的图
                temp_graph.graph_id = graph["graph_id"];
                for (const auto& node : graph["nodes"]) {
                    temp_node.node_id = node["node_id"];
                    temp_node.node_type = node["node_type"];
                    temp_node.node_information = node["node_information"];
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
        std::ofstream file2(output_loc, std::ios::app);  // 创建并以追加模式打开文件
        if (!file2) {
            std::cerr << "无法打开文件 output.txt 进行写入";
            return;
        }
        file2 << "检测无限循环：" << endl;
        for (auto& graph : graphs) {
            graph.generateAllPaths();
        }
    }

    // 定义服务器处理函数
    void run_server(GRAPHS& x) {
        // 创建服务器实例
        httplib::Server server;
        // 设置一个POST路由，响应客户端请求
        server.Post("/data", [&x](const httplib::Request& req, httplib::Response& res) {
            try {
                json received_data = json::parse(req.body);
                std::cout << "Received JSON data: " << received_data.dump() << std::endl;
                // 模拟调用分析函数
                x.requirement_diagram_analysis(received_data);
                res.set_content("success", "text/plain"); // 明确设置响应类型为纯文本
            }
            catch (const std::exception& e) {
                res.status = 400;
                res.set_content(e.what(), "text/plain");
            }
            });

        // 在一个单独的线程中运行服务器
        std::thread server_thread([&server]() {
            std::cout << "Server is listening on http://localhost:8080" << std::endl;
            server.listen("0.0.0.0", 8080); // 本地监听端口8080
            });

        // 主线程等待 15 秒钟
        std::this_thread::sleep_for(std::chrono::seconds(15));

        // 停止服务器并等待线程退出
        server.stop();
        server_thread.join();

        std::cout << "Server has stopped after 15 seconds." << std::endl;
    }
};

GRAPHS x;

int main() {
    SetConsoleOutputCP(CP_UTF8);
    auto start = chrono::high_resolution_clock::now();
    ofstream out1(dependency_extract_path_loc); // 打开txt文档
    out1.close(); // 关闭文件
    ofstream out2(dependency_extract_graph_loc); // 打开txt文档
    out2.close(); // 关闭文件
    ofstream out3(output_loc); // 打开txt文档
    out3.close(); // 关闭文件
    ofstream out4(path_dic_loc); // 打开txt文档
    out4.close(); // 关闭文件
    x.initialization(); // 初始化
    x.output_path_dic(); // 输出路径字典
    x.dependency_extraction(); // 依赖关系提取（结果输入各个txt文档）
    //x.checkEdgeConditionUnsatisfiable(); // 边条件永不满足检测
    //x.checkGraphEdgesForSMT(); // 分支同时满足检测
    //x.graph_miss_detect(); // 需求缺失检测（包括外部需求图缺失与内部先读后写的特例）
    //x.check_dependency_cycle(); //循环依赖检测
    //x.checkForMissingBranches(); // 分支缺失检测
    //deduplicate(output_loc); // 给dependency_extract_path_loc.txt去重
    deduplicate(dependency_extract_path_loc); // 给dependency_extract_path_loc.txt去重
    deduplicate(dependency_extract_graph_loc); // 给dependency_extract_graph_loc.txt去重
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed_seconds = end - start;
    x.output(); // 打印到控制台
    cout << "Elapsed time: " << elapsed_seconds.count() << "s\n";
    return 0;
}





